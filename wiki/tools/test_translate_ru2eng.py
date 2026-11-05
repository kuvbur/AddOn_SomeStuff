"""Офлайн-регрессии переводчика: без ключей, сети и записи страниц вики."""
import ast
import hashlib
import importlib.util
import json
import os
from pathlib import Path
import re
import sys
import tempfile
import unittest
from unittest import mock
import contextlib
import io
import subprocess

SCRIPT = Path(__file__).with_name('wiki_translation.py')


def load_module():
    text = SCRIPT.read_text(encoding='utf-8')
    if 'if not API_KEY:' in text:
        # Старый скрипт нельзя импортировать без чтения dotenv и ключа.
        import types
        module = types.ModuleType('wiki_translator_legacy')
        module.__dict__.update(hashlib=hashlib, json=json, os=os, re=re)
        tree = ast.parse(text)
        tree.body = [node for node in tree.body if isinstance(node, ast.FunctionDef)]
        exec(compile(tree, str(SCRIPT), 'exec'), module.__dict__)
        return module
    spec = importlib.util.spec_from_file_location('wiki_translator', SCRIPT)
    module = importlib.util.module_from_spec(spec)
    sys.modules[spec.name] = module
    spec.loader.exec_module(module)
    return module


m = load_module()


class MarkdownRegressionTests(unittest.TestCase):
    def test_preamble_preserved(self):
        blocks = m.split_md_by_headers('Intro\nSecond\n# Title\nBody\n')
        self.assertEqual(blocks[0]['content'], 'Intro\nSecond\n')

    def test_headingless_preserved(self):
        self.assertEqual(m.split_md_by_headers('First\nSecond\nThird\n')[0]['content'],
                         'First\nSecond\nThird\n')

    def test_backtick_fence(self):
        self.assertEqual(len(m.split_md_by_headers('# Title\n```py\n# comment\n```\n')), 1)

    def test_tilde_fence(self):
        self.assertEqual(len(m.split_md_by_headers('# Title\n~~~py\n## comment\n~~~\n')), 1)

    def test_hash_preserves_spaces(self):
        self.assertNotEqual(m.block_to_hash({'header': '# A', 'content': '`a b`'}),
                            m.block_to_hash({'header': '# A', 'content': '`ab`'}))

    def test_hash_preserves_case(self):
        self.assertNotEqual(m.block_to_hash({'header': '# A', 'content': '`MyVar`'}),
                            m.block_to_hash({'header': '# A', 'content': '`myvar`'}))

    def test_hash_preserves_heading_depth(self):
        self.assertNotEqual(m.block_to_hash({'header': '# A', 'content': 'body'}),
                            m.block_to_hash({'header': '## A', 'content': 'body'}))

    def test_image_rewrite_does_not_touch_code(self):
        m.IMAGE_DIR = 'wiki/image'
        text = '![x](https://host/wiki/image/pic-ru.png)\n`pic-ru.png`\n'
        self.assertEqual(m.find_image_links(text, {'pic-en.png': 'wiki/image/pic-en.png'}),
                         '![x](https://host/wiki/image/pic-en.png)\n`pic-ru.png`\n')

    def test_image_case_preserved(self):
        m.IMAGE_DIR = 'wiki/image'
        self.assertEqual(m.find_image_links('![x](https://host/wiki/image/pic-ru.PNG)',
                         {'pic-en.png': 'wiki/image/pic-en.PNG'}),
                         '![x](https://host/wiki/image/pic-en.PNG)')


class ProtectionTests(unittest.TestCase):
    def test_roundtrip_cases(self):
        cases = ['# Заголовок\n\nТекст  \n', '> - Русский `код`\n',
                 '```py\n# Русский комментарий\n```\n', '~~~\n## Код\n~~~\n',
                 '    # Отступленный код\n', '> ```py\n> # Вложенный код\n> ```\n',
                 '``код ` внутри`` и текст\n', '| Заголовок | Столбец |\n|---|:---:|\n| Ячейка | `значение` |\n',
                 '[Текст](https://example.org/a_(b) "Заголовок")\n',
                 '[Текст][ид]\n\n[ид]: https://example.org/path "Title"\n',
                 '<div>Русский HTML</div>\n', 'some_stuff_marker и GS::UniString\n',
                 r'Текст \CRLF \TAB \* `параметр`' + '\n',
                 'Без завершающего newline', '\n\n', 'Заголовок\n===\n',
                 '![Вложенная [подпись]](https://example.org/file.png)\n']
        for raw in cases:
            with self.subTest(raw=raw):
                p = m.protect(raw)
                self.assertEqual(p.restore(p.text), raw)
                self.assertEqual(m.translate_chunk(raw, lambda text: text, {}), raw)

    def test_missing_duplicated_reordered_token(self):
        p = m.protect('# Русский `код`\n')
        tokens = m.MARKER.findall(p.text)
        bad = [p.text.replace(tokens[0], ''), p.text + tokens[0],
               p.text.replace(tokens[0], '⟦WIKI:unknown:0⟧'), p.text + '\n', '']
        bad.append(p.text.replace(tokens[0], 'TEMP').replace(tokens[1], tokens[0]).replace('TEMP', tokens[1]))
        for value in bad:
            with self.subTest(value=value), self.assertRaises(m.TranslationError):
                p.restore(value)

    def test_added_markdown_rejected(self):
        with self.assertRaises(m.TranslationError):
            m.translate_chunk('Русский текст\n', lambda text: '**' + text + '**', {})

    def test_code_not_sent_as_plain_text(self):
        seen = []
        raw = '# Русский\n\n```py\nsecret_code(123)\n```\n'
        m.translate_chunk(raw, lambda text: seen.append(text) or text, {})
        self.assertTrue(seen)
        self.assertNotIn('secret_code', seen[0])

    def test_links_headings_code_query_fragment(self):
        url = 'https://github.com/kuvbur/AddOn_SomeStuff/wiki/Page-ru?x=1#anchor'
        raw = f'# [Страница]({url})\n`{url}`\n'
        expected = raw.replace(f']({url})', f']({url.replace("Page-ru", "Page-en")})')
        self.assertEqual(m.find_image_links(raw, {}), expected)

    def test_reference_destination_only(self):
        url = 'https://github.com/kuvbur/AddOn_SomeStuff/wiki/Page-ru'
        raw = f'[Русский][id]\n\n[id]: <{url}> "Title"\n'
        self.assertEqual(m.find_image_links(raw, {}), raw.replace(url, url.replace('-ru', '-en')))

    def test_external_wiki_unchanged(self):
        url = 'https://other.example/AddOn_SomeStuff/wiki/Page-ru'
        self.assertEqual(m.find_image_links(f'[Текст]({url})\n', {}), f'[Текст]({url})\n')

    def test_chunk_boundaries_and_limit(self):
        raw = '# Заголовок\n\n' + ('Короткий абзац.\n\n' * 5)
        parts = m.chunks(raw, 35)
        self.assertEqual(''.join(parts), raw)
        self.assertTrue(all(len(part) <= 35 for part in parts))
        with self.assertRaises(m.TranslationError):
            m.chunks('```\n' + 'x' * 50 + '\n```\n', 30)

    def test_unmatched_inline_tick_cannot_consume_next_fence(self):
        raw = 'Русский `незакрытый\n\n```py\nsecret = `value`\n```\n'
        p = m.protect(raw)
        self.assertNotIn('secret', p.text)
        self.assertNotIn('value', p.text)
        self.assertEqual(p.restore(p.text), raw)

    def test_technical_tokens_are_not_exposed(self):
        raw = 'Параметры $1_path %structype% {@gdl:имя} Sync_from{ИМЯ} SomeStuff.apx\n'
        p = m.protect(raw)
        for technical in ('$1_path', '%structype%', '{@gdl:имя}', 'Sync_from{ИМЯ}', 'SomeStuff.apx'):
            self.assertNotIn(technical, p.text)
        self.assertEqual(p.restore(p.text), raw)

    def test_literal_nonlink_is_not_localized(self):
        raw = 'Текст ](https://github.com/kuvbur/AddOn_SomeStuff/wiki/Page-ru)\n'
        self.assertEqual(m.find_image_links(raw, {}), raw)

    def test_email_and_percent_placeholder_are_immutable(self):
        raw = 'Параметр %zone% и user+tag@example.com\n'
        p = m.protect(raw)
        self.assertNotIn('%zone%', p.text)
        self.assertNotIn('user+tag', p.text)
        output = m.translate_chunk(raw, lambda text: text.replace('Параметр', 'Parameter')
                                   .replace('%zone%', '%room%')
                                   .replace('user+tag', 'other'), {})
        self.assertIn('%zone%', output)
        self.assertIn('user+tag@example.com', output)

    def test_nested_image_index_and_encoded_spaces(self):
        raw = '![x](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/sub/Report%20name-ru.PNG)\n'
        images = {'sub/report name-en.png': 'C:/custom/images/sub/Report name-en.PNG'}
        self.assertEqual(m.find_image_links(raw, images), raw.replace('name-ru.PNG', 'name-en.PNG'))


class FilePipelineTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR'))
        self.addCleanup(self.tmp.cleanup)
        self.root = Path(self.tmp.name)
        self.src, self.dst, self.cache = [self.root / name for name in ('ru.md', 'en.md', 'cache.json')]
        self.src.write_bytes('# Первый\nТекст.\n\n# Второй\nДругой.\n\n'.encode())
        self.calls = []

    def translate(self, text):
        self.calls.append(text)
        for ru, en in [('Первый', 'First'), ('Второй', 'Second'), ('Текст', 'Text'), ('Другой', 'Other')]:
            text = text.replace(ru, en)
        return text

    def run_file(self, callback=None, **kwargs):
        return m.translate_file(self.src, self.dst, self.cache, callback or self.translate, **kwargs)

    def test_first_repeat_and_deleted_tail(self):
        self.assertEqual(self.run_file()['translated'], 2)
        self.assertEqual(self.run_file()['translated'], 0)
        self.assertEqual(len(self.calls), 2)
        self.src.write_bytes('# Первый\nТекст.\n\n'.encode())
        result = self.run_file()
        self.assertEqual(result['translated'], 0)
        self.assertNotIn('Second', self.dst.read_text())

    def test_reorder_reuses_cached_translations(self):
        self.run_file()
        self.src.write_bytes('# Второй\nДругой.\n\n# Первый\nТекст.\n\n'.encode())
        self.assertEqual(self.run_file()['translated'], 0)
        self.assertTrue(self.dst.read_text().startswith('# Second'))

    def test_api_failure_does_not_advance_cache(self):
        self.run_file()
        before = self.cache.read_bytes(), self.dst.read_bytes()
        self.src.write_bytes('# Изменено\nНовый текст\n'.encode())
        with self.assertRaises(RuntimeError):
            self.run_file(lambda text: (_ for _ in ()).throw(RuntimeError('controlled failure')))
        self.assertEqual((self.cache.read_bytes(), self.dst.read_bytes()), before)
        self.assertEqual(self.run_file()['translated'], 1)

    def test_invalid_response_does_not_write(self):
        with self.assertRaises(m.TranslationError):
            self.run_file(lambda text: 'Truncated response')
        self.assertFalse(self.dst.exists())
        self.assertFalse(self.cache.exists())

    def test_destination_failure_leaves_cache_and_file(self):
        self.run_file()
        before = self.cache.read_bytes(), self.dst.read_bytes()
        self.src.write_bytes('# Изменено\nНовый текст\n'.encode())
        with mock.patch.object(m.os, 'replace', side_effect=OSError('controlled destination failure')):
            with self.assertRaises(OSError):
                self.run_file()
        self.assertEqual((self.cache.read_bytes(), self.dst.read_bytes()), before)
        self.assertFalse(list(self.root.glob('*.tmp')))

    def test_cache_write_failure_retries_safely(self):
        real_write = m.atomic_write
        def write(path, data):
            if Path(path) == self.cache:
                raise OSError('controlled cache failure')
            return real_write(path, data)
        with mock.patch.object(m, 'atomic_write', side_effect=write):
            with self.assertRaises(OSError):
                self.run_file()
        self.assertTrue(self.dst.exists())
        self.assertFalse(self.cache.exists())
        self.assertEqual(self.run_file()['translated'], 2)

    def test_model_prompt_and_force_invalidate(self):
        self.run_file()
        self.assertEqual(self.run_file(context='new-model')['translated'], 2)
        self.assertEqual(self.run_file(context='new-model')['translated'], 0)
        self.assertEqual(self.run_file(context='new-model', force=True)['translated'], 2)
        self.assertNotEqual(m.fingerprint(prompt='a'), m.fingerprint(prompt='b'))

    def test_manual_target_edit_repaired_from_cache(self):
        self.run_file()
        original = self.dst.read_bytes()
        self.dst.write_text('manual edit')
        self.assertEqual(self.run_file()['translated'], 0)
        self.assertEqual(self.dst.read_bytes(), original)

    def test_crlf_and_empty_source(self):
        data = '# Первый\r\n\r\n`Русский код`  \r\n'.encode()
        self.src.write_bytes(data)
        self.run_file(lambda text: text)
        self.assertEqual(self.dst.read_bytes(), data)
        self.src.write_bytes(b'')
        self.run_file()
        self.assertEqual(self.dst.read_bytes(), b'')

    def test_corrupt_and_legacy_cache_fail_closed(self):
        for payload in ('{broken', '{"wiki/ru/Page.md": []}', '{"version": 2, "files": []}'):
            with self.subTest(payload=payload):
                self.cache.write_text(payload)
                with self.assertRaises(m.TranslationError):
                    self.run_file()
                self.assertEqual(self.cache.read_text(), payload)
                self.assertFalse(self.dst.exists())

    def test_path_collisions_rejected(self):
        for destination, cache in [(self.src, self.cache), (self.dst, self.src), (self.dst, self.dst)]:
            with self.assertRaises(m.TranslationError):
                m.translate_file(self.src, destination, cache, self.translate)

    def test_source_changed_during_request_does_not_save(self):
        def callback(text):
            self.src.write_bytes('# Новая версия\n'.encode())
            return self.translate(text)
        with self.assertRaises(m.TranslationError):
            self.run_file(callback)
        self.assertFalse(self.dst.exists())
        self.assertFalse(self.cache.exists())

    def test_image_changes_reuse_prose_and_preserve_original_urls(self):
        self.src.write_bytes('Текст ![Отчёт](https://github.com/kuvbur/AddOn_SomeStuff/blob/master/wiki/image/report-ru.PNG)\n'.encode())
        self.run_file(images={})
        images = {'report-en.png': 'wiki/image/report-en.PNG'}
        self.assertEqual(self.run_file(images=images)['translated'], 0)
        self.assertIn('report-en.PNG', self.dst.read_text())
        self.assertEqual(self.run_file(images={})['translated'], 0)
        self.assertIn('report-ru.PNG', self.dst.read_text())

    def test_cache_tampered_code_rejected_even_with_updated_checksum(self):
        self.src.write_bytes('Текст `original_code`\n'.encode())
        self.run_file()
        cached = json.loads(self.cache.read_text())
        block = next(iter(cached['files'].values()))['blocks'][0]
        block['translation'] = block['translation'].replace('original_code', 'tampered_code')
        block['output_hash'] = m.digest(block['translation'])
        self.cache.write_text(json.dumps(cached))
        original = self.dst.read_bytes()
        with self.assertRaises(m.TranslationError):
            self.run_file()
        self.assertEqual(self.dst.read_bytes(), original)

    def test_reference_ids_across_sections_are_protected(self):
        self.src.write_bytes('# Первый\nТекст [ссылка][] и [ссылка].\n\n# Второй\n\n[ссылка]: https://example.org\n'.encode())
        def translate(text):
            return self.translate(text).replace('ссылка', 'link')
        self.run_file(translate)
        output = self.dst.read_text()
        self.assertIn('[ссылка][]', output)
        self.assertIn('[ссылка]: https://example.org', output)
        self.assertEqual(m.structure(output), m.structure(self.src.read_text()))


class HttpTests(unittest.TestCase):
    def response(self, status=200, **changes):
        import httpx
        choice = {'finish_reason': 'stop', 'message': {'content': 'English'}}
        choice.update(changes)
        return httpx.Response(status, json={'choices': [choice]})

    def test_transient_error_then_success(self):
        client = mock.Mock()
        client.post.side_effect = [self.response(429), self.response(503), self.response()]
        sleep = mock.Mock()
        with m.OpenRouterTranslator('fake-test-key', client=client, sleep=sleep) as translate:
            self.assertEqual(translate('Русский'), 'English')
        self.assertEqual(client.post.call_count, 3)
        self.assertEqual(sleep.call_count, 2)
        client.close.assert_not_called()

    def test_no_retry_auth_error_or_invalid_output(self):
        import httpx
        responses = [self.response(401), self.response(finish_reason='length'),
                     self.response(message={'content': ''}), self.response(message={'content': None}),
                     httpx.Response(200, content=b'not-json'), httpx.Response(200, json={'error': 'bad'}),
                     httpx.Response(200, json={'choices': []})]
        for response in responses:
            with self.subTest(response=response.content):
                client = mock.Mock()
                client.post.return_value = response
                with self.assertRaises(m.TranslationError):
                    m.OpenRouterTranslator('fake-test-key', client=client, sleep=lambda _: None)('Text')
                self.assertEqual(client.post.call_count, 1)

    def test_bounded_transport_retry(self):
        import httpx
        client = mock.Mock()
        client.post.side_effect = httpx.ReadTimeout('controlled timeout')
        with self.assertRaises(m.TranslationError):
            m.OpenRouterTranslator('fake-test-key', client=client, sleep=lambda _: None)('Text')
        self.assertEqual(client.post.call_count, 3)


class CliTests(unittest.TestCase):
    def test_import_and_help_without_credentials(self):
        wrapper = SCRIPT.with_name('translate_ru2eng.py')
        environment = {**os.environ, 'PYTHONDONTWRITEBYTECODE': '1'}
        environment.pop('OPENROUTER_API_KEY', None)
        environment.pop('DEEPSEEK_API_KEY', None)
        process = subprocess.run([sys.executable, str(wrapper), '--help'], env=environment,
                                 cwd=os.environ.get('TMPDIR'), capture_output=True, text=True)
        self.assertEqual(process.returncode, 0, process.stderr)
        self.assertIn('--dry-run', process.stdout)

    def test_dry_run_and_check_are_read_only(self):
        with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as directory:
            root = Path(directory)
            source = root / 'ru'
            source.mkdir()
            (source / 'Page-ru.md').write_bytes('# Русский\n'.encode())
            options = ['--source-dir', str(source), '--target-dir', str(root / 'en'),
                       '--cache', str(root / 'cache.json'), '--image-dir', str(root / 'images')]
            with mock.patch.object(m, 'OpenRouterTranslator', side_effect=AssertionError('network forbidden')):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(m.main(options + ['--dry-run']), 0)
                    self.assertEqual(m.main(options + ['--check']), 1)
                self.assertEqual(sorted(p.name for p in root.iterdir()), ['ru'])

    def test_naming(self):
        self.assertEqual(m.target_name('Page-ru.md'), 'Page-en.md')
        self.assertEqual(m.target_name('Home.md'), 'Home-en.md')
        self.assertEqual(m.target_name('_Sidebar.md'), '_Sidebar.md')

    def test_output_name_collision_is_preflight_error(self):
        with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as directory:
            root = Path(directory)
            source = root / 'ru'
            source.mkdir()
            for name in ('Page.md', 'Page-ru.md'):
                (source / name).write_bytes('# Русский\n'.encode())
            with contextlib.redirect_stderr(io.StringIO()):
                self.assertEqual(m.main(['--source-dir', str(source), '--target-dir', str(root / 'en'),
                                         '--cache', str(root / 'cache.json'), '--dry-run']), 2)
            self.assertFalse((root / 'en').exists())

    def test_cached_run_and_check_need_no_credentials(self):
        with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as directory:
            root = Path(directory)
            source, target = root / 'ru', root / 'en'
            source.mkdir()
            raw = source / 'Page-ru.md'
            raw.write_bytes('# Русский\n'.encode())
            cache = root / 'cache.json'
            m.translate_file(raw, target / 'Page-en.md', cache, lambda text: text, {})
            options = ['--source-dir', str(source), '--target-dir', str(target),
                       '--cache', str(cache), '--image-dir', str(root / 'images')]
            with mock.patch.object(m, 'OpenRouterTranslator', side_effect=AssertionError('network forbidden')):
                with contextlib.redirect_stdout(io.StringIO()):
                    self.assertEqual(m.main(options + ['--check']), 0)
                    self.assertEqual(m.main(options), 0)

    def test_check_rejects_corrupt_cache_blocks_and_checksums(self):
        with tempfile.TemporaryDirectory(dir=os.environ.get('TMPDIR')) as directory:
            root = Path(directory)
            source, target, cache = root / 'ru', root / 'en', root / 'cache.json'
            source.mkdir()
            raw = source / 'Page-ru.md'
            raw.write_bytes('# Русский\n'.encode())
            m.translate_file(raw, target / 'Page-en.md', cache, lambda text: text, {})
            original = cache.read_text()
            options = ['--source-dir', str(source), '--target-dir', str(target),
                       '--cache', str(cache), '--image-dir', str(root / 'images'), '--check']
            for failure in ('blocks', 'checksum'):
                with self.subTest(failure=failure):
                    data = json.loads(original)
                    entry = next(iter(data['files'].values()))
                    if failure == 'blocks':
                        entry['blocks'] = 'BROKEN'
                    else:
                        entry['blocks'][0]['translation'] = 'tampered'
                    cache.write_text(json.dumps(data))
                    before = cache.read_bytes()
                    with contextlib.redirect_stdout(io.StringIO()), contextlib.redirect_stderr(io.StringIO()):
                        self.assertEqual(m.main(options), 2)
                    self.assertEqual(cache.read_bytes(), before)


if __name__ == '__main__':
    unittest.main()
