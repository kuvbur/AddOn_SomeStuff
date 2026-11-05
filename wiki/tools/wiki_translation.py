"""Безопасный конвейер перевода Markdown; сеть используется только явно из CLI."""
from __future__ import annotations

import argparse
from contextlib import ExitStack
from dataclasses import dataclass
import hashlib
import json
import os
from pathlib import Path
import re
import tempfile
import time
from urllib.parse import quote, unquote, urlsplit, urlunsplit

ROOT = Path(__file__).resolve().parents[2]
MODEL = 'deepseek/deepseek-v3.2'
BASE_URL = 'https://openrouter.ai/api/v1/chat/completions'
CACHE_VERSION = 2
PROTECTION_VERSION = 1
MAX_CHARS = 16000
TRANSLATION_PROMPT = """Translate Russian documentation for the SomeStuff Archicad add-on to precise American English.
Return ONLY the translation, without explanations or a Markdown wrapper.
Tokens of the form ⟦WIKI:...⟧ are immutable: copy each exactly once, in the original order.
They contain all protected Markdown, newlines, code and URLs. Do not add Markdown or physical newlines.
Translate prose, headings and link labels, but never invent technical facts or expand the source.
Terminology: свойство = property; параметр = parameter; зона = zone; избранное = favorite;
ведомость отделки = finishing schedule; строительный материал = building material;
многослойная конструкция = composite structure; сложный профиль = complex profile.
Use normal English capitalization, not Title Case for every technical noun."""
MARKER = re.compile(r'⟦WIKI:[^⟧]*⟧')
CYRILLIC = re.compile(r'[\u0400-\u04ff]')
EMAIL = re.compile(r"[A-Za-z0-9.!#$%&'*+/=?^_`{|}~-]+@[A-Za-z0-9](?:[A-Za-z0-9.-]*[A-Za-z0-9])?")


class TranslationError(RuntimeError):
    pass


def markdown_parser():
    try:
        from markdown_it import MarkdownIt
    except ImportError as exc:
        raise TranslationError('Install wiki/tools/requirements.txt first.') from exc
    return MarkdownIt('commonmark').enable('table')


def digest(text):
    return hashlib.sha256(text.encode('utf-8')).hexdigest()


def block_to_hash(block):
    return digest(block.get('text', (block.get('header') or '') + block.get('content', '')))


def split_md_by_headers(text):
    """Сохраняет исходные срезы, включая преамбулу, отступы и последний newline."""
    lines = text.splitlines(keepends=True)
    headings = {token.map[0]: token for token in markdown_parser().parse(text)
                if token.type == 'heading_open' and token.level == 0}
    starts = sorted({0, *headings}) if text else []
    blocks = []
    for start, end in zip(starts, starts[1:] + [len(lines)]):
        raw = ''.join(lines[start:end])
        token = headings.get(start)
        blocks.append({'type': token.tag if token else None,
                       'header': lines[start].rstrip('\r\n') if token else None,
                       'content': ''.join(lines[start + 1:end]) if token else raw,
                       'text': raw})
    return blocks


def chunks(text, maximum=MAX_CHARS):
    """Не разрывает таблицы, списки и код ради лимита запроса."""
    if maximum < 1:
        raise TranslationError('max-chars must be positive')
    result = []
    for section in split_md_by_headers(text):
        raw = section['text']
        lines = raw.splitlines(keepends=True)
        ends = sorted({token.map[1] for token in markdown_parser().parse(raw)
                       if token.map and token.level == 0} | {len(lines)})
        start, pending = 0, ''
        for end in ends:
            part = ''.join(lines[start:end])
            start = end
            if pending and len(pending) + len(part) > maximum:
                result.append(pending)
                pending = ''
            if len(part) > maximum:
                raise TranslationError(f'Markdown block exceeds --max-chars ({len(part)} > {maximum})')
            pending += part
        if pending:
            result.append(pending)
    return result


def get_image_files_dict(root_dir):
    root = Path(root_dir)
    return {path.relative_to(root).as_posix().casefold(): path.as_posix()
            for path in sorted(root.rglob('*'))
            if path.is_file() and path.suffix.lower() in {'.png', '.jpg', '.jpeg', '.gif', '.svg', '.webp'}}


def localize_url(url, images):
    try:
        parsed = urlsplit(url)
    except ValueError as exc:
        raise TranslationError('Malformed link URL') from exc
    path = parsed.path
    if parsed.netloc.lower() == 'github.com' and path.startswith('/kuvbur/AddOn_SomeStuff/wiki/'):
        path = re.sub(r'-ru(?=(?:\.md)?$)', '-en', path)
    elif '/wiki/image/' in path:
        prefix, relative = path.split('/wiki/image/', 1)
        candidate = re.sub(r'-ru(?=\.[^/]+$)', '-en', unquote(relative))
        actual = images.get(candidate.casefold())
        if actual and candidate != unquote(relative):
            # Ключ нечувствителен к регистру, URL берётся из настоящего пути.
            actual = actual.replace('\\', '/')
            actual_relative = '/'.join(actual.split('/')[-len(candidate.split('/')):])
            path = prefix + '/wiki/image/' + quote(actual_relative, safe='/')
    return urlunsplit((parsed.scheme, parsed.netloc, path, parsed.query, parsed.fragment))


def code_ranges(text):
    lines = text.splitlines(keepends=True)
    offsets = [0]
    for line in lines:
        offsets.append(offsets[-1] + len(line))
    return {offsets[token.map[0]]: offsets[token.map[1]]
            for token in markdown_parser().parse(text)
            if token.type in {'fence', 'code_block', 'html_block', 'hr'} and token.map}


@dataclass
class ProtectedText:
    text: str
    values: list[str]
    prefix: str

    def restore(self, translated):
        expected = [f'{self.prefix}{index}⟧' for index in range(len(self.values))]
        if not isinstance(translated, str) or MARKER.findall(translated) != expected:
            raise TranslationError('Missing, duplicated, reordered or unknown protected token')
        if '\n' in translated or '\r' in translated:
            raise TranslationError('Translation added unprotected newlines')
        if not translated.strip():
            raise TranslationError('Empty translation')
        return MARKER.sub(lambda match: self.values[int(match.group()[len(self.prefix):-1])], translated)


def reference_keys(text):
    env = {}
    markdown_parser().parse(text, env)
    return set(env.get('references', {}))


def document_context(context, text):
    return digest(context + json.dumps(sorted(reference_keys(text)), ensure_ascii=False))


def protect(text, images=None, references=None):
    """Модель видит текст, а не код, адреса, разметку или технические идентификаторы."""
    from markdown_it.helpers import parseLinkDestination
    from markdown_it.common.utils import normalizeReference

    if MARKER.search(text):
        raise TranslationError('Source contains reserved translation tokens')
    images = images or {}
    references = reference_keys(text) if references is None else references
    ranges = code_ranges(text)
    prefix = f'⟦WIKI:{digest(text)[:12]}:'
    values, output = [], []

    def keep(value):
        output.append(f'{prefix}{len(values)}⟧')
        values.append(value)

    pos, brackets = 0, 0
    while pos < len(text):
        if pos in ranges:
            end = ranges[pos]
            keep(text[pos:end])
            pos = end
            continue
        tail = text[pos:]
        if pos == 0 or text[pos - 1] == '\n':
            reference = re.match(r' {0,3}\[[^\]\n]+\]:[^\n]*(?:\n|$)', tail)
            if reference:
                raw = reference.group()
                raw = re.sub(r'(\]:[ \t]*)(<?)(\S+?)(>?)(?=[ \t\r\n]|$)',
                             lambda m: m[1] + m[2] + localize_url(m[3], images) + m[4], raw, count=1)
                keep(raw)
                pos += reference.end()
                continue
            table_rule = re.match(r'[ \t]*\|?[ \t]*:?-+:?[ \t]*(?:\|[ \t]*:?-+:?[ \t]*)+\|?[ \t]*(?:\n|$)', tail)
            setext = re.match(r' {0,3}(?:=+|-+)[ \t]*(?:\n|$)', tail)
            line_prefix = re.match(r'[ \t]*(?:>[ \t]*)*(?:(?:#{1,6}[ \t]+)|(?:(?:[-+*]|\d+[.)])[ \t]+))?(?:\[[ xX]\][ \t]+)?', tail)
            match = table_rule or setext or line_prefix
            if match and match.end():
                keep(match.group())
                pos += match.end()
                continue
        match = re.match(r'[ \t]*(?:\r?\n|$)', tail)
        if match and match.end():
            keep(match.group())
            pos += match.end()
            continue
        if text[pos] == '`':
            run = re.match(r'`+', tail).group()
            boundary = min((start for start in ranges if start > pos), default=len(text))
            blank = re.search(r'\n[ \t]*\n', text[pos:boundary])
            if blank:
                boundary = pos + blank.start()
            closing = re.search(r'(?<!`)' + re.escape(run) + r'(?!`)', text[pos + len(run):boundary])
            if closing:
                end = pos + len(run) + closing.end()
                keep(text[pos:end])
                pos = end
                continue
        if text[pos] == '[':
            shortcut = re.match(r'\[([^\]\n]+)\](?:\[\])?(?![([])', tail)
            if shortcut and normalizeReference(shortcut[1]) in references:
                keep(shortcut.group())
                pos += shortcut.end()
                continue
            brackets += 1
            keep('[')
            pos += 1
            continue
        if brackets and tail.startswith(']('):
            start = pos + 2
            while start < len(text) and text[start] in ' \t':
                start += 1
            destination = parseLinkDestination(text, start, len(text))
            if destination.ok:
                # Заголовок ссылки и её URL не переводятся; подпись перед ] остаётся текстом.
                suffix = re.match(r'[ \t]*(?:(?:"[^"\n]*"|\'[^\'\n]*\'|\([^\n]*?\))[ \t]*)?\)', text[destination.pos:])
                if suffix:
                    end = destination.pos + suffix.end()
                    raw_url = text[start:destination.pos]
                    angle = raw_url.startswith('<') and raw_url.endswith('>')
                    new_url = localize_url(raw_url[1:-1] if angle else raw_url, images)
                    keep(text[pos:start] + ('<' + new_url + '>' if angle else new_url) + text[destination.pos:end])
                    pos = end
                    brackets -= 1
                    continue
        match = re.match(r'\]\[[^\]\n]*\]', tail) if brackets else None
        if match:
            keep(match.group())
            pos += match.end()
            brackets -= 1
            continue
        if text[pos] == ']':
            brackets = max(0, brackets - 1)
            keep(']')
            pos += 1
            continue
        email = EMAIL.match(tail)
        if email:
            keep(email.group())
            pos += email.end()
            continue
        match = re.match(r'\\(?:CRLF|TAB|[A-Za-z_][\w:]*|[^\n])|<[^>\n]*>|https?://[^\s<>]+|(?:[A-Za-z]:[/\\]|\.{1,2}/)[^\s<>]+|\$[A-Za-z0-9_]+|%[^%\n]+%|\{@[^}\n]+\}|\b(?:Sync_\w+|Spec_rule\w*)\{[^}\n]*\}|\b[A-Za-z][\w]*(?:::[\w]+)+\b|\b[A-Za-z][\w]*_[\w]+\b|\b[\w.-]+\.[A-Za-z][A-Za-z0-9]{0,7}\b|[*_~|<>#`]+', tail)
        if match:
            keep(match.group())
            pos += match.end()
            continue
        output.append(text[pos])
        pos += 1
    return ProtectedText(''.join(output), values, prefix)


def find_image_links(text, image_dict):
    protected = protect(text, image_dict)
    return protected.restore(protected.text) if text else text


def structure(text):
    def signature(tokens):
        return [(token.type, token.tag, token.nesting, token.markup,
                 signature(token.children) if token.children else [])
                for token in tokens if token.type not in {'text', 'text_special'}]
    return signature(markdown_parser().parse(text))


def translate_chunk(raw, translate, images, references=None):
    protected = protect(raw, references=references)
    prose = MARKER.sub('', protected.text)
    if CYRILLIC.search(prose):
        translated = translate(protected.text)
    else:
        translated = protected.text
    restored = protected.restore(translated)
    validate_translation(raw, restored, references)
    return restored


def validate_translation(raw, translated, references=None):
    if (structure(translated) != structure(raw)
            or protect(translated, references=references).values != protect(raw, references=references).values):
        raise TranslationError('Translation changed Markdown structure or protected content')


def image_fingerprint(images):
    return digest(json.dumps(images or {}, sort_keys=True, ensure_ascii=False))


def atomic_write(path, data):
    path = Path(path)
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=path.parent, prefix='.' + path.name + '.', suffix='.tmp', delete=False) as stream:
            temporary = Path(stream.name)
            stream.write(data)
            stream.flush()
            os.fsync(stream.fileno())
        os.replace(temporary, path)
    finally:
        if temporary and temporary.exists():
            temporary.unlink()


def validate_cache_entry(entry):
    if not isinstance(entry, dict) or not all(isinstance(entry.get(key), str)
            for key in ('context', 'source_hash', 'output_hash', 'images_hash')):
        raise TranslationError('Malformed cache file entry')
    if not isinstance(entry.get('blocks'), list):
        raise TranslationError('Malformed cache blocks')
    for block in entry['blocks']:
        if not isinstance(block, dict) or not all(isinstance(block.get(key), str)
                for key in ('source', 'translation', 'output_hash')):
            raise TranslationError('Malformed cached translation')
        if digest(block['translation']) != block['output_hash']:
            raise TranslationError('Cached translation checksum mismatch')


def load_cache(path):
    path = Path(path)
    if not path.exists():
        return {'version': CACHE_VERSION, 'files': {}}
    try:
        data = json.loads(path.read_text(encoding='utf-8'))
    except (ValueError, OSError) as exc:
        raise TranslationError(f'Cannot read cache {path}; left untouched') from exc
    if not isinstance(data, dict) or data.get('version') != CACHE_VERSION or not isinstance(data.get('files'), dict):
        raise TranslationError('Unsupported cache; use a new --cache path (legacy hashes cannot verify translations)')
    for entry in data['files'].values():
        validate_cache_entry(entry)
    return data


def fingerprint(model=MODEL, prompt=TRANSLATION_PROMPT, base_url=BASE_URL):
    return digest(json.dumps([CACHE_VERSION, PROTECTION_VERSION, model, prompt, base_url], ensure_ascii=False))


def translate_file(src_path, dst_path, cache_path, translate, images=None, *, force=False,
                   maximum=MAX_CHARS, context=None):
    src_path, dst_path, cache_path = map(Path, (src_path, dst_path, cache_path))
    if len({p.resolve() for p in (src_path, dst_path, cache_path)}) != 3:
        raise TranslationError('Source, destination and cache paths must be different')
    source_bytes = src_path.read_bytes()
    source = source_bytes.decode('utf-8-sig').replace('\r\n', '\n')
    newline = '\r\n' if b'\r\n' in source_bytes else '\n'
    parts = chunks(source, maximum)
    cache = load_cache(cache_path)
    key = src_path.resolve().as_posix()
    entry = cache['files'].get(key, {})
    references = reference_keys(source)
    current_context = document_context(context or fingerprint(), source)
    old_parts = entry.get('blocks', []) if entry.get('context') == current_context else []
    reusable = {block['source']: block['translation'] for block in old_parts}
    translations, records = [], []
    translated_count = cached_count = 0
    for raw in parts:
        hashed = digest(raw)
        if not force and hashed in reusable:
            output = reusable[hashed]
            # Даже прочитанный кэш не обходит проверку защищённых фрагментов.
            validate_translation(raw, output, references)
            cached_count += 1
        else:
            output = translate_chunk(raw, translate, images or {}, references)
            translated_count += 1
        translations.append(output)
        records.append({'source': hashed, 'translation': output, 'output_hash': digest(output)})
    joined = ''.join(translations)
    if source:
        validate_translation(source, joined, references)
    output_bytes = find_image_links(joined, images or {}).replace('\n', newline).encode('utf-8')
    if src_path.read_bytes() != source_bytes:
        raise TranslationError('Source changed during translation; destination and cache left untouched')
    changed = not dst_path.exists() or dst_path.read_bytes() != output_bytes
    if changed:
        atomic_write(dst_path, output_bytes)
    # При сбое кэша готовый EN остаётся корректным; повтор лишь повторит работу.
    cache['files'][key] = {'context': current_context, 'blocks': records,
                           'images_hash': image_fingerprint(images),
                           'source_hash': hashlib.sha256(source_bytes).hexdigest(),
                           'output_hash': hashlib.sha256(output_bytes).hexdigest()}
    serialized = (json.dumps(cache, ensure_ascii=False, indent=2, sort_keys=True) + '\n').encode('utf-8')
    if not cache_path.exists() or cache_path.read_bytes() != serialized:
        atomic_write(cache_path, serialized)
    return {'translated': translated_count, 'cached': cached_count, 'changed': changed}


class OpenRouterTranslator:
    def __init__(self, api_key, *, model=MODEL, base_url=BASE_URL, timeout=120,
                 retries=2, client=None, sleep=time.sleep):
        import httpx
        self.client = client or httpx.Client(timeout=timeout)
        self.owned = client is None
        self.api_key, self.model, self.base_url = api_key, model, base_url
        self.retries, self.sleep = retries, sleep

    def __enter__(self):
        return self

    def __exit__(self, *args):
        if self.owned:
            self.client.close()

    def __call__(self, text):
        import httpx
        payload = {'model': self.model, 'messages': [
            {'role': 'system', 'content': TRANSLATION_PROMPT}, {'role': 'user', 'content': text}],
            'temperature': 0.1, 'max_tokens': 16384}
        for attempt in range(self.retries + 1):
            try:
                response = self.client.post(self.base_url, headers={
                    'Authorization': f'Bearer {self.api_key}', 'Content-Type': 'application/json'}, json=payload)
            except httpx.TransportError as exc:
                if attempt == self.retries:
                    raise TranslationError('API transport failure after bounded retries') from exc
            else:
                if response.status_code == 200:
                    break
                if response.status_code not in {408, 429, 500, 502, 503, 504} or attempt == self.retries:
                    # Ответ сервера может содержать исходный текст или служебные данные.
                    raise TranslationError(f'LLM API returned HTTP {response.status_code}')
            self.sleep(min(2 ** attempt, 8))
        try:
            data = response.json()
            choice = data['choices'][0]
            content = choice['message']['content']
            if choice.get('finish_reason') != 'stop' or not isinstance(content, str) or not content.strip():
                raise ValueError('Incomplete or empty response')
            return content.strip()
        except (ValueError, KeyError, IndexError, TypeError) as exc:
            raise TranslationError('Invalid, incomplete or empty API response; destination not saved') from exc


def target_name(name):
    if name == '_Sidebar.md':
        return name
    return name[:-6] + '-en.md' if name.endswith('-ru.md') else Path(name).stem + '-en.md'


def parser():
    cli = argparse.ArgumentParser(description='Translate SomeStuff RU wiki safely; dry-run/check never call the API.')
    cli.add_argument('--source-dir', type=Path, default=ROOT / 'wiki/ru')
    cli.add_argument('--target-dir', type=Path, default=ROOT / 'wiki/en')
    cli.add_argument('--image-dir', type=Path, default=ROOT / 'wiki/image')
    cli.add_argument('--cache', type=Path, default=ROOT / 'wiki/tools/translation_cache.v2.json')
    cli.add_argument('--file', action='append', help='Source filename; repeat for several pages')
    mode = cli.add_mutually_exclusive_group()
    mode.add_argument('--dry-run', action='store_true', help='Show work without API or writes')
    mode.add_argument('--check', action='store_true', help='Validate inputs/cache and report stale outputs; exit 1 if stale')
    cli.add_argument('--force', action='store_true', help='Ignore cached translations')
    cli.add_argument('--model', default=MODEL)
    cli.add_argument('--base-url', default=BASE_URL)
    cli.add_argument('--timeout', type=float, default=120)
    cli.add_argument('--retries', type=int, default=2)
    cli.add_argument('--max-chars', type=int, default=MAX_CHARS)
    return cli


def main(argv=None):
    cli = parser()
    args = cli.parse_args(argv)
    try:
        if args.timeout <= 0 or args.retries < 0 or args.retries > 5 or args.max_chars < 1:
            raise TranslationError('Require timeout > 0, retries 0..5, max-chars > 0')
        if not args.source_dir.is_dir():
            raise TranslationError(f'Source directory does not exist: {args.source_dir}')
        if args.source_dir.resolve() == args.target_dir.resolve():
            raise TranslationError('Source and target directories must differ')
        names = args.file or [p.name for p in sorted(args.source_dir.glob('*.md'))]
        if not names:
            raise TranslationError('No Markdown files selected')
        if any(Path(name).name != name or not name.endswith('.md') for name in names):
            raise TranslationError('--file accepts a Markdown filename, not a path')
        files = [args.source_dir / name for name in dict.fromkeys(names)]
        if any(not path.is_file() for path in files):
            raise TranslationError('A selected source file does not exist')
        cache = load_cache(args.cache)
        images = get_image_files_dict(args.image_dir)
        context = fingerprint(args.model, base_url=args.base_url)
        destinations = [args.target_dir / target_name(source.name) for source in files]
        source_paths = {source.resolve() for source in files}
        destination_paths = [destination.resolve() for destination in destinations]
        if (len(set(destination_paths)) != len(destination_paths)
                or source_paths.intersection(destination_paths)
                or args.cache.resolve() in source_paths.union(destination_paths)):
            raise TranslationError('Selected sources, destinations and cache must not overlap')
        stale = 0
        # Все входы проверяются до первого запроса или записи.
        for source in files:
            raw = source.read_bytes()
            text = raw.decode('utf-8-sig').replace('\r\n', '\n')
            for part in chunks(text, args.max_chars):
                protected = protect(part, images)
                if protected.restore(protected.text) != find_image_links(part, images):
                    raise TranslationError('Protected Markdown round-trip failed')
            destination = args.target_dir / target_name(source.name)
            if len({source.resolve(), destination.resolve(), args.cache.resolve()}) != 3:
                raise TranslationError('Source, destination and cache paths must be different')
            entry = cache['files'].get(source.resolve().as_posix(), {})
            current = (not args.force and isinstance(entry, dict) and entry.get('context') == document_context(context, text)
                       and entry.get('images_hash') == image_fingerprint(images)
                       and entry.get('source_hash') == hashlib.sha256(raw).hexdigest()
                       and destination.exists() and entry.get('output_hash') == hashlib.sha256(destination.read_bytes()).hexdigest())
            stale += not current
            if args.dry_run or args.check:
                print(f'{"CURRENT" if current else "STALE"}: {source.name} -> {destination.name}')
        if args.dry_run or args.check:
            print(f'Files: {len(files)}; stale: {stale}; API calls: 0; writes: 0')
            return 1 if args.check and stale else 0
        if urlsplit(args.base_url).scheme != 'https':
            raise TranslationError('API endpoint must use HTTPS')
        with ExitStack() as stack:
            clients = []

            def translator(text):
                # При полном попадании в кэш не нужны ни ключ, ни HTTP-клиент.
                if not clients:
                    from dotenv import load_dotenv
                    load_dotenv(ROOT / '.env')
                    key = os.getenv('OPENROUTER_API_KEY') or os.getenv('DEEPSEEK_API_KEY')
                    if not key:
                        raise TranslationError('Set OPENROUTER_API_KEY (legacy DEEPSEEK_API_KEY is also supported)')
                    clients.append(stack.enter_context(OpenRouterTranslator(
                        key, model=args.model, base_url=args.base_url,
                        timeout=args.timeout, retries=args.retries)))
                return clients[0](text)

            for source in files:
                result = translate_file(source, args.target_dir / target_name(source.name), args.cache,
                                        translator, images, force=args.force, maximum=args.max_chars, context=context)
                print(f'{source.name}: translated={result["translated"]}, cached={result["cached"]}, changed={result["changed"]}')
        return 0
    except (TranslationError, OSError, UnicodeError, ImportError) as exc:
        print(f'ERROR: {exc}', file=__import__('sys').stderr)
        return 2
