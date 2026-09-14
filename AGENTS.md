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
Sources/AddOn/third_party/      Embedded third-party libs
Sources/AddOnResources/         Resources
Sources/MacDarkModeIcon/        macOS assets
Tools/BuildAddOn.py             Build/config script
Tools/CMakeCommon.cmake         CMake config
Tools/CompileResources.py       Resource compiler
Tools/test_html.ps1             HTML interface test (run after every Interface_ru.html edit)
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

**Landmine — memo init:** before `ACAPI_Element_GetMemo(...)`, always
`BNZeroMemory(&memo, sizeof(memo));` — don't remove without verified evidence.

**Landmine — preferences vs. Teamwork:** `ACAPI_SetPreferences` writes to
every project file (DevKit-25: `Preferences_Save`) and breaks Teamwork.
Settings go only in the local `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`.
`ReadSyncSettingsFromFile` rejects a file with a mismatched
`PreferencesVersion` — bump the version on any new settings array.

**Landmine — undo regions:** one undo region per user action, never per
element (hundreds of undo steps otherwise) — DevKit docs require this.

**Landmine — Propertycache keys:** cache keys are always lowercase (prefix

- `ToLowerCase` + `BRACEEND`). Returning a name without normalizing case
  means a rule silently never matches.

**Landmine — JS bridge:** bridge = inline functions via
`RegisterACAPIJavaScriptObject` (JSON commands were removed, b7a996b — do
not reintroduce them). Parse `JSFunction` args via `DynamicCast<JSValue>` —
`DynamicCast<JSArray>` crashes ArchiCAD (was a latent R1–R2 crash).

**Landmine — element highlight:** `APIIo_HighlightElementsID` +
`APIDo_ZoomToElementsID` trigger `SelectionChangeHandler`, which can reset
palette selection — guard with `static suppressSelectionRefresh`.
`SetElementHighlight` is AC26+/27+ only; on AC25 use `ACAPI_Interface`
directly (Clear before Set; a call with no `par1` clears).

**Landmine — "Монитор" data source:** property values come only from
`PROPERTYCACHE()`, never `ACAPI_Property_GetPropertyValue` per element. The
cache's `property` entry holds definitions only — values are looked up per
element separately.

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

**HTML interface** — after every `Interface_ru.html` edit, run
`powershell -File Tools/test_html.ps1` before claiming `tested`.

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

### 11.1 Issues (GitHub)

Every user wish (feature request) and every bug (found or fixed) → GitHub
issue in `kuvbur/AddOn_SomeStuff` FIRST (`gh` CLI; workflow — skill
`addon-somestuff-issues`). Then backlink in `IDEA.md` and
`Reviews/*.tracker.csv` (column `issue`). Dedup check before create —
existing issue may already cover the wish (comment there instead).
Commit that closes the issue: `Refs: #N` in message.

`gh` is not on PATH by default in this environment:
`export PATH="$PATH:/c/Program Files/GitHub CLI"`.
`Reviews/` is gitignored — the tracker is local-only (BOM+LF; edit via
Python `utf-8-sig`, not a plain text write).

## 12. IDEA.md

Format/archival rules = SOUL.md. Landmines that outlive the current task
(architecture constraints, crash patterns, version differences) belong in
this file's §6/§14, not in `IDEA.md`'s "Грабли" — that section is for
issues specific to the active task and gets archived with it.

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

| File                         | Responsibility                                                                                           |
| ---------------------------- | -------------------------------------------------------------------------------------------------------- |
| `SomeStuff_Main.cpp/hpp`     | Entry point, interface, menu, observers                                                                  |
| `Helpers.cpp/hpp`            | Core helpers, properties, selection, params                                                              |
| `Propertycache.cpp/hpp`      | Property/classification/attribute/project cache — see §6 landmines (cache keys, PROPERTYCACHE-only data) |
| `Sync.cpp/hpp`               | Property sync & monitoring                                                                               |
| `Roombook.cpp`               | Finish schedule                                                                                          |
| `Spec.cpp`                   | Spec rules                                                                                               |
| `Summ.cpp`                   | Property summation                                                                                       |
| `ReNum.cpp`                  | Renumbering                                                                                              |
| `Revision.cpp`               | Revision markers                                                                                         |
| `Dimensions.cpp`             | Dimensions — see §16 (line 158, do not fix)                                                              |
| `ClassificationFunction.cpp` | Auto-classification                                                                                      |
| `ResetProperty.cpp`          | Property reset — see §6 landmine (undo regions)                                                          |
| `AutomateFunction.cpp`       | Automation/alignment                                                                                     |
| `MEPv1.cpp`                  | MEP                                                                                                      |
| `CommonFunction.cpp`         | Common utils                                                                                             |

The repository may change. Do not blindly trust this table.

## 14. BrowserPalette / HTML

Loads HTML from disk: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`,
implemented in `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp`.

- Bridge is inline functions via `RegisterACAPIJavaScriptObject` — see §6
  "Landmine — JS bridge" for the `DynamicCast<JSValue>` crash pattern.
- HTML changes follow `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md`
  only — not ad hoc. After every edit, run `Tools/test_html.ps1` (see §9).
- Don't use `html_to_hpp.py`/`HTML_Pages.hpp` — unused here.
- Don't edit `Sources/AddOnResources/` unless required.

## 15. Pre-PR Checklist

Only intended files changed · no user changes overwritten · AC version(s)
known · every `ACAPI_*` checked via LightRAG or marked `not verified` with
reason · clang-format run · LSP checked · build done for
version(s)+platform · runtime test done where applicable (HTML changes: ran
`test_html.ps1`) · verified/compiled/tested reported correctly · no
generated files staged · no unrelated refactor · memory updated · IDEA.md
re-read before final write · GitHub issue exists for the change and commit
references it (`Refs: #N`, see §11.1).

## 16. Known — do not fix without explicit request

Author already decided these are out of scope:

- `Dimensions.cpp:158` — `pen_original`.
- Roombook — recreation of finish elements.
- `Sync.cpp:419-428` — cumulative `epm`.

If a task's root-cause analysis leads here, stop and flag it rather than
"fixing" it — this is a deliberate choice, not an oversight.
