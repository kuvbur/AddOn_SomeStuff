#!/usr/bin/env python3
"""Проверка актуальности Docs: пути, отсутствие номеров строк в карточках, свежесть исходников.

Проверки (без clangd, только файлы и git):
  1. Каждый упомянутый в Docs путь к файлу существует на диске.
  2. В Docs/modules/** и Docs/lifecycle/** нет ссылок вида `File.cpp:123` —
     координаты символов живут только в Docs/_generated/symbols.json.
  3. Docs/_generated/source_hashes.json сверяется с диском: изменившийся
     исходник делает STALE каждую карточку, которая его упоминает.
  4. Строка заголовка карточки "Состояние кода: `<hash>`" сверяется с последним
     коммитом, трогавшим упомянутые карточкой исходники.

Коды возврата: 0 — ошибок нет, 1 — есть ошибки или (с --strict) STALE.

Использование:
  python Tools/check_docs.py            # проверка
  python Tools/check_docs.py --update   # перезаписать source_hashes.json
  python Tools/check_docs.py --strict   # STALE считается ошибкой
"""

import hashlib
import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DOCS = ROOT / "Docs"
GENERATED = DOCS / "_generated"
HASHES_FILE = GENERATED / "source_hashes.json"
SOURCES = ROOT / "Sources" / "AddOn"

# Координаты вида `roombook/Roombook.cpp:123` или `Roombook.hpp:88-95`.
COORD_RE = re.compile(r"`[A-Za-z0-9_./\\-]+\.(?:cpp|hpp|h)[:](\d+)")
# Пути к файлам, которые карточка обязана упоминать существующими.
PATH_RE = re.compile(
    r"`((?:Sources|Tools|Docs|Test_file)/[A-Za-z0-9_./ -]+\.(?:cpp|hpp|h|py|ps1|js|md|in|html|pln|json))`"
)
# Строка состояния карточки: > Состояние кода: `36feb07` ... (+ опционально "незакоммиченные")
STATE_RE = re.compile(r"Состояние кода:\s*`([0-9a-f]{7,40})`")
DIRTY_MARK = "незакоммиченные"
CARD_DIRS = ("modules", "lifecycle")


def sha256(path):
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def source_files():
    out = []
    for p in SOURCES.rglob("*"):
        if p.is_file() and p.suffix.lower() in (".cpp", ".hpp", ".h"):
            out.append(p)
    return sorted(out)


def rel(path):
    return path.relative_to(ROOT).as_posix()


def md_files():
    for p in sorted(DOCS.rglob("*.md")):
        if GENERATED in p.parents:
            continue
        yield p


def git_last_commit(paths):
    if not paths:
        return None, None
    try:
        res = subprocess.run(
            ["git", "log", "-1", "--format=%h%x09%cs", "--", *paths],
            cwd=str(ROOT),
            capture_output=True,
            text=True,
        )
    except OSError:
        return None, None
    line = res.stdout.strip().split("\n")[0]
    if not line:
        return None, None
    parts = line.split("\t")
    return (parts[0], parts[1] if len(parts) > 1 else None)


def update_hashes():
    data = {rel(p): sha256(p) for p in source_files()}
    GENERATED.mkdir(parents=True, exist_ok=True)
    GENERATED.joinpath("source_hashes.json").write_text(
        json.dumps(data, indent=2, ensure_ascii=False, sort_keys=True) + "\n",
        encoding="utf-8",
        newline="\n",
    )
    print(f"source_hashes.json: {len(data)} файлов записано")
    return 0


def main():
    strict = "--strict" in sys.argv
    if "--update" in sys.argv:
        return update_hashes()

    errors = []
    stale = []
    dirty = []
    coords = []

    try:
        recorded = json.loads(HASHES_FILE.read_text(encoding="utf-8"))
    except FileNotFoundError:
        recorded = {}
        errors.append(
            f"{rel(HASHES_FILE)} отсутствует — выполнить python Tools/check_docs.py --update"
        )

    changed_sources = []
    for relpath, digest in recorded.items():
        p = ROOT / relpath
        if not p.exists():
            changed_sources.append((relpath, "файл удалён"))
        elif sha256(p) != digest:
            changed_sources.append((relpath, "изменён"))

    for md in md_files():
        relmd = rel(md)
        text = md.read_text(encoding="utf-8", errors="replace")
        in_card = md.parent.name in CARD_DIRS

        for m in PATH_RE.finditer(text):
            target = m.group(1)
            # Сокращение вида `X.cpp/.hpp` — не путь, а перечисление расширений.
            if re.search(r"\.(?:cpp|hpp|h|py|js)/", target):
                continue
            if not (ROOT / target).exists():
                line = text[: m.start()].count("\n") + 1
                errors.append(f"{relmd}:{line}: путь не существует: {target}")

        if in_card:
            for m in COORD_RE.finditer(text):
                line = text[: m.start()].count("\n") + 1
                coords.append(
                    f"{relmd}:{line}: номер строки в карточке ({m.group(0).strip('`')}); "
                    "ссылаться на имя символа, координаты — в symbols.json"
                )

        # исходники, упомянутые карточкой
        mentioned = [p for p in recorded if p in text]
        for relpath, reason in changed_sources:
            if relpath in mentioned:
                stale.append(f"{relmd}: STALE — {relpath} {reason}")

        # состояние по коммиту
        state = STATE_RE.search(text)
        if state:
            mentioned_dirs = sorted({str(Path(p).parent) for p in mentioned})
            head, date = git_last_commit(mentioned_dirs)
            if head and head != state.group(1) and DIRTY_MARK not in text:
                stale.append(
                    f"{relmd}: STALE — карточка описывает {state.group(1)}, "
                    f"последний коммит по модулю {head} ({date})"
                )
            if DIRTY_MARK in text:
                dirty.append(f"{relmd}: рабочее дерево помечено как незакоммиченное")

    for item in dirty:
        print(f"NOTE  {item}")
    for item in stale:
        print(f"STALE {item}")
    for item in coords:
        print(f"COORD {item}")
    for item in errors:
        print(f"ERROR {item}")

    print(
        f"\nитог: ошибок {len(errors)}, устаревших {len(stale)}, "
        f"карточек с номерами строк {len(coords)}, изменённых исходников {len(changed_sources)}"
    )
    if errors:
        return 1
    if strict and (stale or coords):
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
