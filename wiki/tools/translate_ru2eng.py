import hashlib
import os
import glob
import httpx
import re
import json
from dotenv import load_dotenv

TRANSLATION_PROMPT = """
You are an expert technical translator for ArchiCAD add-ons and GDL.
Translate Russian Markdown documentation to formal, precise English technical language.
## PRESERVE EXACTLY (do not translate or modify):
- All Markdown structure: headers (#, ##), lists, tables, blockquotes (>, ```), links `[text](url)`, images `![alt](url)`
- Code blocks (```gdl, ```python, ```bash), inline code `code`
- File paths, commands, filenames, parameters, env vars (e.g. `APIX::LoadLibrary`, `$1_path`, `gdl_object.gsm`)

## TRANSLATE ONLY:
- Plain text in paragraphs, list items, table cells
- Use consistent terminology: "wall" → "Wall", "элемент" → "Element", "параметр" → "Parameter"

## OUTPUT RULES:
- Return ONLY the translated Markdown
- Same structure, same line count where possible
- No explanations, comments, new sections, or additional text
- Use American English spelling
""".strip()
# MODEL = "openai/gpt-3.5-turbo"
MODEL="deepseek/deepseek-v3.2"
load_dotenv()

SOURCE_DIR = "wiki/ru"
TARGET_DIR = "wiki/en"
IMAGE_DIR = "wiki/image"
HASH_FILE = "wiki/tools/translation_hashes.json"
BASE_URL = "https://openrouter.ai/api/v1/chat/completions"
API_KEY = os.getenv("DEEPSEEK_API_KEY")

if not API_KEY:
    raise RuntimeError("DEEPSEEK_API_KEY must be set (via env)")
def get_image_files_dict(root_dir):
    image_dict = {}
    if not os.path.exists(root_dir):
        return image_dict
    for dirpath, dirnames, filenames in os.walk(root_dir):
        for fname in filenames:
            if fname.lower().endswith((".png")):
                rel_path = os.path.relpath(os.path.join(dirpath, fname), start=root_dir)
                image_dict[rel_path.replace("\\", "/").lower()] = os.path.join(dirpath, fname).replace("\\", "/")
    return image_dict

def find_image_links(text: str, image_dict: dict) -> str:
    # Паттерн для Markdown‑изображений: ![alt](url)
    pattern_image = r"!\[([^\]]*)\]\((https?://[^\s)]+)\)"
    for m in re.finditer(pattern_image, text):           # текст в квадратных скобках
        url = m.group(2)
        if IMAGE_DIR in url and '-ru' in url:
            name=url.split('/')[-1]
            newname=name.replace('-ru', '-en').lower()
            if newname in image_dict:
                text = text.replace(name, newname)
    pattern = r"\[([^\]]+)\]\((https?://[^\s)]*AddOn_SomeStuff/wiki/[^\s)]*)\)"
    for m in re.finditer(pattern, text):
        url = m.group(2)
        if '-ru' in url:
            new_url = url.replace("-ru", "-en")
            text = text.replace(url, new_url)
    return text

def load_hashes():
    if not os.path.exists(HASH_FILE):
        return {}
    with open(HASH_FILE, "r", encoding="utf-8") as f:
        return json.load(f)

def save_hashes(hashes):
    with open(HASH_FILE, "w", encoding="utf-8") as f:
        json.dump(hashes, f, ensure_ascii=False, indent=2, sort_keys=True)

def block_to_hash(block: dict) -> str:
    s = ""
    if block["header"]:
        s += block["header"]
    if block["content"]:
        s += block["content"]
    s = s.replace("\n", '')
    s = s.replace(" ", '')
    s = s.replace("\t", '')
    s = s.replace("#", '').lower()
    return hashlib.sha256(s.encode("utf-8")).hexdigest()

def get_changed_blocks(file_path: str, current_blocks: list, is_file_exists: bool) -> list:
    hashes = load_hashes()
    file_key = file_path.replace("\\", "/")  # нормализуем путь
    saved = hashes.get(file_key, [])
    changed = []
    for i, block in enumerate(current_blocks):
        block_hash = block_to_hash(block)
        if i >= len(saved) or saved[i].get("hash") != block_hash or not is_file_exists:
            changed.append((i, block, block_hash))
    return changed


def update_hashes(file_path: str, blocks: list):
    hashes = load_hashes()
    file_key = file_path.replace("\\", "/")
    hash_list = [ {"index": idx, "hash": block_to_hash(block)} for idx, block in enumerate(blocks) ]
    hashes[file_key] = hash_list
    save_hashes(hashes)

def translate_text(text: str) -> str:
    headers = {
        "Authorization": f"Bearer {API_KEY}",
        "Content-Type": "application/json",
    }

    payload = {
        "model": MODEL,
        "messages": [
            {"role": "system", "content": TRANSLATION_PROMPT},
            {"role": "user", "content": text},
        ],
        "temperature": 0.1,
        "max_tokens": 8192,
        "top_p": 0.9
    }

    with httpx.Client(timeout=30.0) as client:
        resp = client.post(BASE_URL, headers=headers, json=payload)

        if resp.status_code != 200:
            raise RuntimeError(f"LLM API error: {resp.status_code}\n{resp.text}")

        data = resp.json()
        return data["choices"][0]["message"]["content"].strip()

def split_md_by_headers(text: str):
    """
    Разбивает текст по ## и ### заголовкам, возвращая список блоков.
    Блок = [тип_заголовка, заголовок, содержимое_до_следующего].
    """
    # Найдём все заголовки, сохраняя их строку и тип (##/###)
    blocks = []
    current_block = {"type": None, "header": None, "content": ""}

    lines = text.splitlines(keepends=True)

    for line in lines:
        # Проверяем, является ли строка заголовком
        m = re.match(r"^(#{1,4})\s+(.+)", line)
        if m:
            if current_block["type"] is not None:
                blocks.append(current_block)
            current_block = {
                "type": m.group(1),
                "header": m.group(0).rstrip(),  # вся строка заголовка
                "content": "",
            }
        else:
            if current_block["type"] is None:
                # часть до первого заголовка
                current_block = {
                    "type": None,
                    "header": None,
                    "content": "",
                }
            current_block["content"] += line

    if current_block["type"] is not None or current_block["content"]:
        blocks.append(current_block)

    return blocks

def translate_md_block(block) -> dict:
    """
    Переводит один блок: либо только текст, либо заголовок + текст.
    """
    if block["header"]:
        # ### Заголовок
        header_line = block["header"] + "\n"
        content = block["content"]
        full_text = header_line + content
        translated = translate_text(full_text).strip()
        first_line = translated.splitlines()[0] if translated.strip() else ""
        rest = "\n".join(translated.splitlines()[1:])
        return {
            "type": block["type"],
            "header": first_line,
            "content": rest,
        }
    else:
        # начало файла до первого заголовка
        if block["content"].strip():
            translated = translate_text(block["content"])
            return {
                "type": None,
                "header": None,
                "content": translated,
            }
        else:
            return block

def translate_md_file_split(src_path: str, rel_path: str, image_dict):
    dst_path = os.path.join(TARGET_DIR, rel_path)
    if '-ru.md' in dst_path:
        dst_path = dst_path.replace("-ru.md", "-en.md")
    elif '.md' in dst_path and not '_Sidebar.md' in dst_path:
        dst_path = dst_path.replace(".md", "-en.md")
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)

    with open(src_path, "r", encoding="utf-8") as f:
        src_text = f.read()

    blocks = split_md_by_headers(src_text)
    changed = get_changed_blocks(src_path, blocks, os.path.exists(dst_path))
    update_hashes(src_path, blocks)
    translated_blocks = [""] * len(blocks)
    if os.path.exists(dst_path):
        if not changed:
            print(f"No changes in {src_path}, skipping translation.")
            return
        with open(dst_path, "r", encoding="utf-8") as f:
            existing_text = f.read()
        existing_blocks = split_md_by_headers(existing_text)
        for i, block in enumerate(existing_blocks):
            if i < len(translated_blocks):
                translated_blocks[i] = block
    # Переводим только changed-блоки
    for idx, block, block_hash in changed:
        print(f"Translating changed block {idx}")
        trans_block = translate_md_block(block)
        translated_blocks[idx] = trans_block

    # Собираем обратно
    translated_lines = []
    for block in translated_blocks:
        if isinstance(block, dict):
            if block["header"]:
                translated_lines.append("\n"+block["header"].strip() + "\n")
            if block["content"]:
                block["content"] = find_image_links(block["content"], image_dict)
                translated_lines.append("\n"+block["content"].strip() + "\n")
        else:
            translated_lines.append(block)

    translated = "".join(translated_lines).strip("\n")
    with open(dst_path, "w", encoding="utf-8") as f:
        f.write(translated)
    print(f"Saved: {dst_path}\n")

if __name__ == "__main__":
    n_processed = 0
    if 'tools' in os.getcwd():
        os.chdir('..')
    if 'wiki' in os.getcwd():
        os.chdir('..')
    file_list = glob.glob(f"{SOURCE_DIR}/*.md")
    image_dict =get_image_files_dict(IMAGE_DIR)
    for src_path in file_list:
        rel_path = os.path.relpath(src_path, SOURCE_DIR)
        try:
            translate_md_file_split(src_path, rel_path, image_dict)
            n_processed += 1
        except Exception as e:
            print(f"ERROR on {src_path}:\n{e}\n")
            raise
    print(f"Done. Translated {n_processed} files (split by headers).")