# AGENTS.md — AddOn_SomeStuff

Repo rules for AddOn_SomeStuff. `SOUL.md` = global behavior, read first — this file doesn't repeat it.

## 1. Project

- ArchiCAD Add-On, C++, versions **22–29** (no default — decide from task →
  build config → existing `#if` blocks → test files → ask user).
- Platforms: **Windows + macOS**.
- Source: `Sources/AddOn/` — never search C++ source outside it, no recursive
  disk/repo search. Never edit `Sources/AddOnResources/` unless the task needs it.

## 2. Structure

```text
Sources/AddOn/                  C++ source
Sources/AddOn/api_headers/      ArchiCAD API headers
Sources/AddOn/json_commands/    JSON command handlers
Sources/AddOn/third_party/      Embedded third-party libs
Sources/AddOnResources/         Resources
Sources/MacDarkModeIcon/        macOS assets
Tools/BuildAddOn.py             Build/config script
Tools/CMakeCommon.cmake         CMake config
Tools/CompileResources.py       Resource compiler
Test_file/                      AC test PLN files
CMakeLists.txt / config.json    Build entry
IDEA.md                         Multi-session task state
```

## 3. Workflow

```
OpenViking → Sources/AddOn/ only → determine AC version(s) →
ACAPI_* → LightRAG → root cause → minimal change → clang-format →
LSP → Build → Runtime (as needed) → git diff → checkpoint → save memory → answer
```

## 4. OpenViking

Mandatory, no fallback tool — if unavailable, follow SOUL.md's "tool
unavailable" rule (say so, fall to source priority, `not verified`).
Search before: errors, non-trivial bugs, unfamiliar symbols, `ACAPI_*`
calls, architecture decisions. Save conclusions after non-trivial fixes.

## 5. C++ Navigation

OpenViking → Clangd MCP (locations/defs/refs) → LightRAG (SDK/arch
context). Only inside `Sources/AddOn/`. Can't establish it → `not verified`, don't guess.

## 6. LightRAG (SDK — authoritative)

Every `ACAPI_*` call: `OpenViking → LightRAG → source/headers/call sites`.
Down or unhelpful → say so, fall to installed headers/docs, else `not
verified`. Never borrow behavior from another AC version, Revit, AutoCAD, another IFC lib.

Verify before touching an SDK API: signature, params, return, ownership,
lifetime, pointer/iterator validity, transaction/undo, redraw/notification,
version differences. Watch closely: `API_Element`, `API_ElementMemo`,
`ACAPI_Element_GetMemo`, `GetPtr`, `GS::*` containers/iterators, SDK-managed memory.

**Landmine:** before `ACAPI_Element_GetMemo(...)`, always
`BNZeroMemory(&memo, sizeof(memo));` — don't remove without verified evidence.

## 7. C++ Editing

Follow SOUL.md Coding rules as-is. No repo-specific additions.

## 8. Formatting

After every `.cpp`/`.hpp`/`.h` edit: `clang-format -i path/to/file.cpp`.
Keep its output — don't hand-fix indentation/includes, don't rewrite a whole file just to format it.

## 9. Validation

**LSP** — inspect via Clangd. Regenerate compile DB only if needed:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version> --lsp`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version> --lsp`

Never commit `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/`.

**Build** — only claim `compiled` after an actual successful build, on the target platform:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version>`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version>`

**Runtime** — Win launcher: `"D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1"`.
No mac equivalent in repo → say so, ask; don't invent one.
Only claim `tested` if behavior was actually executed and observed.

## 10. Tests

`Sources/AddOn/TestFunc.cpp/.hpp`, active under `TESTING`.
Task = write/fix tests → **production code is read-only** (inspect, don't
edit). Test exposes a prod bug → stop, report file/function/why, treat fix
as a separate task. Never modify `Test_file/*.pln` in a normal test task.

## 11. Git Safety

`git status` before touching files. Never discard uncommitted user changes
or touch unrelated ones. Never: `clean -fdx`, `commit --amend`, `rebase -i`,
`push --force`, `add .`/`add -A` without checking status/diff first.
Never stage: `Build/`, `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/`.
Checkpoint mechanics = SOUL.md definition, no repo exceptions.

## 12. IDEA.md

Format/archival rules = SOUL.md.

```markdown
# Current Task

## Task

...

## Status

IN_PROGRESS / WAITING_FOR_TEST / BLOCKED

## Last Completed

...

## Next Step

...

## Plan

- [x] ...
- [ ] ...

## Decisions

- ...
```

Not a transcript of every tool call.

## 13. Project Areas (navigation hints only — inspect actual code)

| File                         | Responsibility                                  |
| ---------------------------- | ----------------------------------------------- |
| `SomeStuff_Main.cpp/hpp`     | Entry point, interface, menu, observers         |
| `Helpers.cpp/hpp`            | Core helpers, properties, selection, params     |
| `Propertycache.cpp/hpp`      | Property/classification/attribute/project cache |
| `Sync.cpp/hpp`               | Property sync & monitoring                      |
| `Roombook.cpp`               | Finish schedule                                 |
| `Spec.cpp`                   | Spec rules                                      |
| `Summ.cpp`                   | Property summation                              |
| `ReNum.cpp`                  | Renumbering                                     |
| `Revision.cpp`               | Revision markers                                |
| `Dimensions.cpp`             | Dimensions                                      |
| `ClassificationFunction.cpp` | Auto-classification                             |
| `ResetProperty.cpp`          | Property reset                                  |
| `AutomateFunction.cpp`       | Automation/alignment                            |
| `MEPv1.cpp`                  | MEP                                             |
| `CommonFunction.cpp`         | Common utils                                    |

## 14. BrowserPalette / HTML

Loads HTML from disk: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`

- `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp`. Don't use
  `html_to_hpp.py`/`HTML_Pages.hpp` — unused here. Don't edit
  `Sources/AddOnResources/` unless required.

## 15. Pre-PR Checklist

Only intended files changed · no user changes overwritten · AC version(s)
known · every `ACAPI_*` checked via LightRAG or marked `not verified` with
reason · clang-format run · LSP checked · build done for
version(s)+platform · runtime test done where applicable ·
verified/compiled/tested reported correctly · no generated files staged ·
no unrelated refactor · memory updated · IDEA.md re-read before final write.
