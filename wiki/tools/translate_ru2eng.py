import os
import glob
import httpx
import json

# --- Промт: перевод md‑документации без изменения форматирования ---
TRANSLATION_PROMPT = """
You translate Russian technical documentation (ArchiCAD, BIM) from Markdown to English.
Keep the Markdown structure exactly as in the input:
  - headers (#, ##, etc.), lists, tables, blockquotes, horizontal rules, etc.
NEVER change:
  - code blocks (```...``` and inline `code`),
  - file paths, links [text](...), commands, filenames, options, environment variables.
Translate only plain text in paragraphs and lists.
Use neutral, formal technical English.
Return the SAME Markdown structure; do not add new sections, explanations or comments.
Output only the translated Markdown; nothing else.
""".strip()


SOURCE_DIR = "wiki/ru"
TARGET_DIR = "wiki/en"

BASE_URL = "https://api.deepseek.com/v1"
API_KEY = os.getenv("DEEPSEEK_API_KEY")
MODEL = "deepseek-chat"

if not API_KEY:
    raise RuntimeError("DEEPSEEK_API_KEY must be set (via env)")
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
    }

    with httpx.Client(timeout=30.0) as client:
        resp = client.post(
            f"{BASE_URL}/chat/completions",
            headers=headers,
            json=payload,
        )

        if resp.status_code != 200:
            raise RuntimeError(f"LLM API error: {resp.status_code}\n{resp.text}")

        data = resp.json()
        return data["choices"][0]["message"]["content"].strip()


def translate_md_file(src_path: str, rel_path: str):
    dst_path = os.path.join(TARGET_DIR, rel_path)
    os.makedirs(os.path.dirname(dst_path), exist_ok=True)

    print(f"Translating: {src_path}")
    with open(src_path, "r", encoding="utf-8") as f:
        src_text = f.read()

    translated = translate_text(src_text)

    with open(dst_path, "w", encoding="utf-8") as f:
        f.write(translated)

    print(f"Saved: {dst_path}\n")


if __name__ == "__main__":
    n_processed = 0
    for src_path in glob.glob(f"{SOURCE_DIR}/*.md", recursive=False):
        rel_path = os.path.relpath(src_path, SOURCE_DIR)
        try:
            translate_md_file(src_path, rel_path)
            n_processed += 1
        except Exception as e:
            print(f"ERROR on {src_path}:\n{e}\n")
            raise  # чтобы CI/CD видел, что что‑то пошло не так
    print(f"Done. Translated {n_processed} files.")