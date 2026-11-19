# Documentation update procedure (`Docs/`)

> Trigger: the owner's explicit commit command. Outside that command `Docs/` is read-only.

## 1. Trigger and scope

- Run this procedure only when the owner commands a commit/checkpoint.
- Until then `IDEA.md` keeps an open `[ ] Docs/ обновить перед checkpoint` item and `Docs/` is only read.
- In scope: `Docs/modules/<module>.md`, `Docs/_generated/symbols.json`, `Docs/_generated/callgraph.json`, `Docs/DISCREPANCIES.md`, `Docs/ARCHITECTURE.md`, `Docs/REPOMAP.md`, `Docs/_progress.md`.
- Exception: a comment/code mismatch found mid-task may be recorded at once in `DISCREPANCIES.md` (or as one line in `IDEA.md`).
- Card description markers stay Russian literals: `[из комментария]` / `[по коду]` / `[не проверено]` / `[назначение не установлено]`.

## 2. `symbols.json` — clangd MCP only

No script produces `symbols.json`: the generator script (`generate_symbols.py`) was deleted and removed from the repo (its clangd client never handshook, so it always fell back to a regex scan that records `if`/`for`/`return` as function names and confuses call sites with definitions). Collect through clangd MCP only.

Collection:

1. Take the `.cpp` list from `compile_commands.json` (31 files).
2. Call `get_document_symbols` once per file with an absolute path.
3. Walk the tree; do not record `Namespace`/`Class`/`Struct`, but recurse into their `children`; prefix child names (`ParamHelpers::WriteProperty`); class fields as `Class.field`.
4. clangd lines are 0-based → store `line + 1` (1-based, as in cards).
5. `module` = file name without extension; for `Sources/AddOn/<subdir>/<file>.cpp` use the subdirectory (`pk`, `spec`, `table`, `dialogs`, `json_commands`, `third_party`).
6. Merge batches, dedupe on `(name, file, line)`, write `Docs/_generated/symbols.json`: UTF-8, `indent=2`, `ensure_ascii=False`, LF, no BOM, no trailing newline.

Verify before commit:

```bash
python - <<'EOF'
import json
d = json.load(open('Docs/_generated/symbols.json', encoding='utf-8'))
print(len(d), 'symbols,', len({s['file'] for s in d}), 'files')
bad = [s for s in d if s['name'].split('::')[-1] in {'if','for','while','switch','return'}]
print('keyword names:', len(bad))          # must be 0
EOF
```

Expected order of magnitude: ~840 symbols over 31 `.cpp` files. A large deviation means skipped files or merged garbage.

## 3. `callgraph.json` — clangd MCP only

Scope policy: per hot module, never all modules at once. Only `pk/` is complete (29 edges); elsewhere the card fields «Вызывает»/«Вызывается из» stay `[не проверено]`.

No scripted transport exists; collect edges through MCP:

1. `mcp__clangd__get_call_hierarchy` at the function NAME position in its `.cpp` definition. MCP coordinates are 0-based; grep is 1-based (offset −1).
2. Keep only edges whose paths are under `Sources/AddOn/`; SDK calls (Build/DevKit, MSVC, Windows Kits) go into `sdk_edges_summary` as `outgoing_count`.
3. Write `Docs/_generated/callgraph.json` in the existing shape: `meta`, `edges`, `not_resolved`, `stats`.
4. Remove `[не проверено]` from the card fields you actually filled.

Fallback when clangd does not resolve the position ("No call hierarchy", or `find_references` returns a foreign symbol; seen at `pk/ResetProperty.cpp:14`, `pk/Revision.cpp:14`):

- `grep -rn "FuncName\s*(" --include=*.cpp --include=*.hpp Sources/AddOn`, then drop the definition itself.
- Confirm each call site by reading context (`sed -n '<line>±5p'`) before writing it into a card.
- After two failed clangd attempts, stop and write `не проверено` with the reason.

## 4. Module cards

For each changed module `Docs/modules/<module>.md`:

1. Check the public API table against real definitions (`get_document_symbols` or grep).
2. Update/add records for non-trivial functions; trivial ones stay one table row.
3. Mark every description with its source — no marker, no description.
4. **No line numbers in cards.** Name the symbol; the single source of coordinates is `Docs/_generated/symbols.json`. A card that needs a location points at the name, and `check_docs.py` flags any remaining `File.cpp:123`.
5. Update the header state line — `Состояние кода: <hash>` — and the date.

Record template:

```markdown
### `Namespace::Func(params) -> ret`
- Расположение: `path/file.cpp:123`
- Назначение: 1-2 строки. [из комментария | по коду | назначение не установлено]
- Контракт: предусловия; гарантии; как сообщает об ошибках
- Побочные эффекты: запись в проект, глобальное состояние, ввод-вывод
- Вызывает: (из callgraph.json или grep)
- Вызывается из: (из callgraph.json или grep)
```

## 5. `DISCREPANCIES.md`

One row per mismatch: `file:line`, what the comment claims, what the code does, status. Do not change code here.

## 6. Overview files

- `Docs/ARCHITECTURE.md` — dependency graph when cross-module `#include`s changed; every Mermaid arrow must be backed by an include or a callgraph edge.
- `Docs/REPOMAP.md` — when directories or statistics changed.
- `Docs/_progress.md` — one short dated entry per pass: what was updated and at which commit (sole resume source).

## 7. Commit

Both scenarios start from the owner's commit command, with the code already built and validated (`AGENTS.md` §9).

Order: build/validation → `symbols.json`/`callgraph.json` → affected cards → `DISCREPANCIES.md`/`_progress.md` → `python Tools/check_docs.py --update` and a clean report → `git diff` (own files and hunks only) → commit.

**A. Docs catch up with accumulated code changes and ride the same checkpoint** (typical «коммить» case; no separate `[docs]` commit):

```bash
git add Sources/AddOn/<changed files> Docs/modules/<module>.md Docs/_generated/ Docs/_progress.md
git commit -m "[step-ref] short description
Refs: IDEA.md step <n>"
```

**B. Standalone documentation pass** (regeneration or catching up several modules, not tied to one code change):

```bash
git add Docs/   # Docs/ only, plus AGENTS.md §17 if it changed
git commit -m "[docs] Актуализация: <кратко>
Refs: Docs/_progress.md"
```

Forbidden: `commit --amend`, `rebase -i`, `push --force`, deleting files outside `Docs/`, committing `Build/`, the root `compile_commands.json`, `.cache/`.

## 8. Validation gate

Run before the commit, after the docs pass:

```bash
python Tools/check_docs.py --update   # refresh Docs/_generated/source_hashes.json
python Tools/check_docs.py            # report
```

- `ERROR` (exit 1) — a path referenced by a card does not exist, or `source_hashes.json` is missing. Fix before committing.
- `COORD` — the card still carries `File.cpp:123`. It does not block the commit; migrate that card to symbol names when you next touch it.
- `STALE` — a source mentioned by the card changed after the recorded hash, or the card's `Состояние кода` commit is behind the last commit touching the module.
- `--strict` turns `STALE` and `COORD` into a failure.

## 9. Pitfalls

- clangd MCP reports `-Wunused-*` as errors — a mismatch with MSVC, not a blocker; the arbiter is `BuildAddOn.py`.
- clangd may resolve a foreign symbol at a definition — check that the returned file/name matches.
- The directory is `Docs/` with a capital D (Windows is case-insensitive, git is not).
- `Docs/ARCHITECTURE.md` was once gitignored — when re-creating it, use `git add -f`.
- `module` field for new records: 3 path segments → file name without extension; 4+ → subdirectory name (`roombook`, `tests`, `dialogs`, `spec`, `pk`, `json_commands`, `table`, `third_party`).
- Generated JSON must keep `symbols.json` collection order; never re-sort the file.
