# AGENTS.md — AddOn_SomeStuff

Repo rules for AddOn_SomeStuff. Global Codex behavior is defined outside the repository; this file contains only project-specific rules and does not repeat global instructions.

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
IDEA.md → repo knowledge/landmines → Sources/AddOn/ only →
determine AC version(s) → verify SDK/API → root cause → minimal change →
clang-format → LSP → Build → Runtime (as needed) → git diff →
checkpoint → update durable knowledge if needed → answer
```

## 4. Codex Environment / Project Knowledge

OpenViking is not available in the Codex runtime used for this repository.
Do not search for it, request it, or treat its absence as a blocker.

Before investigating errors, non-trivial bugs, unfamiliar symbols,
`ACAPI_*` calls, or architecture decisions:

1. Read the active state in `IDEA.md`.
2. Check relevant landmines/constraints in this `AGENTS.md`.
3. Inspect the actual source and relevant Git history when useful.
4. Use global Codex engineering memory according to the global instructions.

Project-specific durable findings that outlive the active task belong in
the appropriate landmine/architecture section of this file. Current task
state belongs in `IDEA.md`.

## 5. C++ Navigation

Use Clangd MCP for definitions/references/locations, then targeted source
inspection inside `Sources/AddOn/` only. Do not recursively search the
whole repository or disk for C++ source.

For SDK/API context, **LightRAG is a Hermes skill/workflow, not a native
Codex tool**. When Hermes skills are available to Codex, first inspect the
available skills (`skills_list` / `skill_view`) and use the relevant
LightRAG skill according to its instructions.

Do not expect a standalone `lightrag_*` Codex tool unless the current
environment explicitly provides one.

If the Hermes LightRAG skill is unavailable in the current session, use
installed SDK headers, official documentation, and existing verified call
sites. If the required behavior still cannot be established, mark it
`not verified`; do not guess.

## 6. SDK Verification / Hermes LightRAG Skill

For every `ACAPI_*` call, verify the relevant behavior before changing it.

**LightRAG in this project is provided through Hermes as a skill/workflow.**
It is not assumed to exist as a native Codex tool.

When running under Hermes + Codex:

1. Inspect available Hermes skills with `skills_list`.
2. If a LightRAG/Archicad SDK skill is available, read it with `skill_view`.
3. Follow that skill's procedure for SDK/API lookup.
4. Confirm important findings against installed SDK headers, official docs,
   or verified project call sites when appropriate.

Correct local REST calls in this environment:

- LightRAG SDK endpoint: `http://127.0.0.1:9621/query`.
- Health check: `curl.exe --noproxy 127.0.0.1,localhost -s http://127.0.0.1:9621/health`.
- Query through PowerShell with `--%` so JSON is not mangled:

```powershell
curl.exe --% --noproxy 127.0.0.1,localhost -s --max-time 60 -X POST http://127.0.0.1:9621/query -H "Content-Type: application/json" -d "{""query"":""ACAPI_Element_GetElemList"",""mode"":""local"",""only_need_context"":true}"
```

Use `only_need_context: true` for API lookup. For exact function/type names,
try `local` first, then `naive`, then `hybrid`, then `global`. For "how to"
questions without an exact name, start with `hybrid`; for broad concepts,
start with `global`. If `curl` fails through `http_proxy` / `https_proxy`,
keep `--noproxy 127.0.0.1,localhost` rather than treating LightRAG as down.
If one query returns `No relevant context found`, retry with a more exact
identifier or the next mode before falling back to headers/docs.

Do not spend time searching for a nonexistent standalone LightRAG tool merely
because the project mentions LightRAG. The authoritative integration path is
the Hermes skill when that skill is exposed to Codex.

If the Hermes LightRAG skill is unavailable, fall back to:

installed SDK headers → official docs → existing project call sites/source.

If the issue remains unresolved, mark it `not verified`.

Never borrow behavior from another AC version, Revit, AutoCAD, another IFC lib.

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

**Landmine — palette window resize:** a docked palette's width is owned by
ArchiCAD's dock manager — `SetClientWidth` is ignored while it stays docked
(observed: 450 → 49 px with the dock, 450 → 35 px without). The working recipe
is `UnDock()` → `SetClientWidth()` → `Dock()` again, so the palette keeps its
dock state but adopts the new width (`DG::Palette::IsDocked` / `UnDock` /
`Dock`; `DGIsPaletteDocked(guid)` also exists). A growing DG dialog also
reports `GetMinClientWidth() == original width`, so `SetMinClientWidth` must be
relaxed BEFORE shrinking or the resize is silently clamped.

**Landmine — palette default width / HTML width alignment:** the expanded
palette's size comes from the RINT dialog template `Tools/AddOn.grc.in`
(`'GDLG' 32580 Palette … 0 0 <w> <h>` **and** the `Browser 0 0 <w> <h>` control —
change both), and that same width is what a growing DG dialog reports as
`GetMinClientWidth()`, i.e. the narrowest the user can drag to. The HTML is
embedded in the same grc (`'DATA' ID_ADDON_HTML` → `Interface_ru.html`), so an
HTML edit needs a rebuild too. The HTML `min-width` on `<body>` must stay ≤ that
width (ТЗ §2 forbids clipping/overflow), and a row that stops fitting at the
narrower width must be made to wrap, not trimmed of ТЗ-mandated labels.

**Landmine — "Монитор" data source:** property values come only from
`PROPERTYCACHE()`, never `ACAPI_Property_GetPropertyValue` per element. The
cache's `property` entry holds definitions only — values are looked up per
element separately.

## 7. C++ Editing

Follow the global Codex coding instructions. No additional repo-specific
editing rules beyond this file.

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

Checkpoint = `git add <intended files only>` + `git commit`.

Commit message:

```text
[step-ref] short description
Refs: IDEA.md step <n>
```

Create a checkpoint only after the corresponding step is completed and
validated. Never checkpoint a broken/unverified state.

### 11.1 Issues (GitHub)

For code changes only: every user wish (feature request) and every bug
(found or fixed) → GitHub issue in `kuvbur/AddOn_SomeStuff` FIRST (`gh`
CLI; workflow — skill `addon-somestuff-issues`). Then backlink in
`IDEA.md` and `Reviews/*.tracker.csv` (column `issue`). Dedup check before
create — existing issue may already cover the wish (comment there instead).
Commit that closes the issue: `Refs: #N` in message.

Documentation/instruction-only changes, including `AGENTS.md` and `IDEA.md`
maintenance, do not require a GitHub issue unless they accompany a code
change that already requires one.

On native Windows Codex, `gh.exe` is expected on `PATH`; known installation:
`C:\Program Files\GitHub CLI\gh.exe`.

If GitHub API access fails while `gh` itself is available, check the
current `HTTP_PROXY` / `HTTPS_PROXY` / `NO_PROXY` environment before
treating authentication as invalid. Do not re-login or replace credentials
until network access to GitHub API is verified.

`Reviews/` is gitignored — the tracker is local-only (BOM+LF; edit via
Python `utf-8-sig`, not a plain text write).

## 12. IDEA.md

Read `IDEA.md` before starting non-trivial work. Resume the active task
from its current state and update the Plan before changing code when the
plan is stale.

Plan markers:

- `[ ]` — not started
- `[/]` — in progress
- `[x]` — completed **and validated**

Before ending a work session, update the current state, last completed
step, next action, and last checkpoint. Completed tasks move to
`## Archive` (or `IDEA_ARCHIVE.md` if used); keep the active section short.

`## Scope` defines the active task boundary. Work outside it requires a
separate task. If the repository/task does not establish the target
ArchiCAD version(s), resolve that before SDK-sensitive edits.

Landmines that outlive the current task (architecture constraints, crash
patterns, version differences) belong in this file's §6/§14, not in
`IDEA.md`'s "Грабли" — that section is for issues specific to the active
task and gets archived with it.

```markdown
# Current Task

## Task

...

## Scope

...

## Status

IN_PROGRESS / WAITING_FOR_TEST / BLOCKED

## Last Completed

...

## Next Step

...

## Last Checkpoint

...

## Plan

- [x] ...
- [/] ...
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
known · every touched `ACAPI_*` verified with available SDK evidence
(Hermes LightRAG skill when available; otherwise installed
headers/docs/verified call sites) or marked `not verified` with reason ·
clang-format run · LSP checked · build done for
version(s)+platform · runtime test done where applicable (HTML changes: ran
`test_html.ps1`) · verified/compiled/tested reported correctly · no
generated files staged · no unrelated refactor · memory updated · IDEA.md
re-read before final write · for code changes, GitHub issue exists for the
change and commit references it (`Refs: #N`, see §11.1).

## 16. Known — do not fix without explicit request

Author already decided these are out of scope:

- `Dimensions.cpp:158` — `pen_original`.
- Roombook — recreation of finish elements.
- `Sync.cpp:419-428` — cumulative `epm`.

If a task's root-cause analysis leads here, stop and flag it rather than
"fixing" it — this is a deliberate choice, not an oversight.
