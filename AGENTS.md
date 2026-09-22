# AGENTS.md — AddOn_SomeStuff

Repo-specific rules only. Global Codex behavior lives outside this file and is not repeated here.

## 1. Project

- ArchiCAD Add-On, C++, versions **22–29** (no default — decide from task →
  build config → existing `#if` blocks → test files → ask user).
- Platforms: **Windows + macOS**.
- Source: `Sources/AddOn/` only — never search C++ outside it, no recursive
  disk/repo search. Don't edit `Sources/AddOnResources/` unless the task needs it.

## 2. Structure

Current structure: `Docs/REPOMAP.md` (generated, authoritative — §13/§17).
`IDEA.md` — multi-session task state, see §12.

## 3. Workflow

```
IDEA.md → repo knowledge/landmines → Docs/modules/<area>.md (if it exists) →
Sources/AddOn/ only → determine AC version(s) → verify SDK/API →
root cause → minimal change → clang-format → LSP → Build →
Runtime (as needed) → update durable knowledge (incl. Docs/modules/<area>.md
if it drifted) → git diff → checkpoint → answer
```

## 4. Codex Environment / Project Knowledge

OpenViking is unavailable in this Codex runtime — don't search for it,
request it, or treat its absence as a blocker.

Before investigating errors, non-trivial bugs, unfamiliar symbols,
`ACAPI_*` calls, or architecture decisions:

1. Read `IDEA.md`'s active state.
2. Check landmines/constraints in this file.
3. If `Docs/modules/<module>.md` exists for the area, read it — contract,
   side effects, and known callers/callees may already be there (§17).
   Check `Docs/DISCREPANCIES.md` for known comment/code mismatches before
   trusting a comment. Docs are a starting point, not a substitute for
   verifying the specific detail touched — they carry a commit hash; if
   the file changed materially since, prefer live inspection and treat
   the doc as stale.
4. Inspect the actual source and Git history when useful.
5. Use global Codex engineering memory per the global instructions.

Durable findings that outlive the task belong in this file's
landmine/architecture sections. Current task state belongs in `IDEA.md`.

## 5. C++ Navigation

For call-site questions (who calls X / what X calls), check
`Docs/_generated/callgraph.json` first — populated for `pk/` only,
`[не проверено]` elsewhere (`Docs/_progress.md`). A hit saves a live
query; a miss isn't evidence of absence — fall through to clangd/grep.

Use Clangd MCP for definitions/references/locations, then targeted
inspection inside `Sources/AddOn/` only — no recursive repo/disk search.

For SDK/API context, **LightRAG is a Hermes skill/workflow, not a native
Codex tool**. When Hermes skills are available, inspect them
(`skills_list`/`skill_view`) and follow the relevant LightRAG skill. Don't
expect a standalone `lightrag_*` tool unless the environment provides
one. If the skill is unavailable, use installed SDK headers, official
docs, and verified call sites; if still unresolved, mark `not verified` —
don't guess.

## 6. SDK Verification / Hermes LightRAG Skill

Verify every `ACAPI_*` call's relevant behavior before changing it.
LightRAG here is a Hermes skill, not a native Codex tool. Under
Hermes + Codex:

1. `skills_list`.
2. If a LightRAG/Archicad SDK skill exists, `skill_view` it.
3. Follow its SDK/API lookup procedure.
4. Confirm important findings against installed headers, official docs,
   or verified call sites when appropriate.

Local REST calls:

- Endpoint: `http://127.0.0.1:9621/query`.
- Health: `curl.exe --noproxy 127.0.0.1,localhost -s http://127.0.0.1:9621/health`.
- Query (PowerShell, `--%` so JSON isn't mangled):

```powershell
curl.exe --% --noproxy 127.0.0.1,localhost -s --max-time 60 -X POST http://127.0.0.1:9621/query -H "Content-Type: application/json" -d "{""query"":""ACAPI_Element_GetElemList"",""mode"":""local"",""only_need_context"":true}"
```

`only_need_context: true` for API lookup. Exact name → `local` →
`naive` → `hybrid` → `global`. "How to" without an exact name → start
`hybrid`; broad concept → start `global`. `curl` failing through
`http_proxy`/`https_proxy` → keep `--noproxy`, don't treat LightRAG as
down. `No relevant context found` → retry with a more exact identifier
or the next mode before falling back to headers/docs.

Don't hunt for a nonexistent standalone LightRAG tool. If the Hermes
skill is unavailable: installed headers → official docs → existing call
sites/source → still unresolved → `not verified`.

Never borrow behavior from another AC version, Revit, AutoCAD, or another
IFC lib.

Before touching an SDK API verify: signature, params, return, ownership,
lifetime, pointer/iterator validity, transaction/undo,
redraw/notification, version differences. Watch: `API_Element`,
`API_ElementMemo`, `ACAPI_Element_GetMemo`, `GetPtr`, `GS::*`
containers/iterators, SDK-managed memory.

**Landmine — memo init:** always `BNZeroMemory(&memo, sizeof(memo));`
before `ACAPI_Element_GetMemo(...)` — don't remove without verified
evidence.

**Landmine — preferences vs. Teamwork:** `ACAPI_SetPreferences`
(DevKit-25: `Preferences_Save`) writes to every project file and breaks
Teamwork. Settings go only in local
`…/GRAPHISOFT/SomeStuff/SyncSettings.dat`. `ReadSyncSettingsFromFile`
rejects a mismatched `PreferencesVersion` — bump it on any new settings
array.

**Landmine — undo regions:** one per user action, never per element
(else hundreds of undo steps) — DevKit docs require this.

**Landmine — Propertycache keys:** cache keys are always lowercase
(`ToLowerCase` + `BRACEEND`) — an unnormalized name silently never
matches.

**Landmine — JS bridge:** bridge = inline functions via
`RegisterACAPIJavaScriptObject` (JSON commands removed, b7a996b — don't
reintroduce). Parse `JSFunction` args via `DynamicCast<JSValue>` —
`DynamicCast<JSArray>` crashes ArchiCAD (latent R1–R2 crash).

**Landmine — element highlight:** `APIIo_HighlightElementsID` +
`APIDo_ZoomToElementsID` trigger `SelectionChangeHandler`, which can
reset palette selection — guard with `static suppressSelectionRefresh`.
`SetElementHighlight` is AC26+/27+ only; on AC25 use `ACAPI_Interface`
directly (Clear before Set; a call with no `par1` clears).

**Landmine — palette window resize:** a docked palette's width is owned
by ArchiCAD's dock manager — `SetClientWidth` is ignored while docked
(observed: 450→49px docked, 450→35px undocked). Recipe: `UnDock()` →
`SetClientWidth()` → `Dock()` (`DG::Palette::IsDocked`/`UnDock`/`Dock`;
`DGIsPaletteDocked(guid)` also exists). A growing DG dialog reports
`GetMinClientWidth() == original width`, so relax `SetMinClientWidth`
BEFORE shrinking or the resize is silently clamped.

**Landmine — palette default width / HTML width alignment:**
expanded-palette size comes from the RINT template `Tools/AddOn.grc.in`
(`'GDLG' 32580 Palette … 0 0 <w> <h>` **and** `Browser 0 0 <w> <h>` —
change both); same width is what a growing DG dialog reports as
`GetMinClientWidth()` (the narrowest a user can drag to). HTML is
embedded in the same grc (`'DATA' ID_ADDON_HTML` → `Interface_ru.html`)
— an HTML edit needs a rebuild. HTML `min-width` on `<body>` must stay ≤
that width (ТЗ §2 forbids clipping/overflow); a row that stops fitting
must wrap, not lose ТЗ-mandated labels.

**Landmine — "Монитор" data source:** property values come only from
`PROPERTYCACHE()`, never `ACAPI_Property_GetPropertyValue` per element —
the cache's `property` entry holds definitions only, values are looked
up per element separately.

## 7. C++ Editing

Follow the global Codex coding instructions — no additional repo rules.

## 8. Formatting

After every `.cpp`/`.hpp`/`.h` edit: `clang-format -i path/to/file.cpp`.
Keep its output — don't hand-fix indentation/includes, don't reformat a
whole file just to format it.

## 9. Validation

**LSP** — inspect via Clangd; regenerate the compile DB only if needed:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version> --lsp`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version> --lsp`

Never commit `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/`.

**Build** — claim `compiled` only after an actual successful build on the
target platform:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version>`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version>`

**Runtime** — Win launcher:
`"D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1"`. No mac
equivalent in repo → say so, ask, don't invent one. Claim `tested` only
if behavior was actually executed and observed.

**HTML interface** — after every `Interface_ru.html` edit, run
`powershell -File Tools/test_html.ps1` before claiming `tested`.

## 10. Tests

`Sources/AddOn/TestFunc.cpp/.hpp`, active under `TESTING`. Task =
write/fix tests → **production code is read-only** (inspect, don't
edit). A test exposing a prod bug → stop, report file/function/why,
treat the fix as a separate task. Never modify `Test_file/*.pln` in a
normal test task.

## 11. Git Safety

`git status` before touching files. Never discard uncommitted user
changes or touch unrelated ones. Never: `clean -fdx`, `commit --amend`,
`rebase -i`, `push --force`, `add .`/`add -A` without checking
status/diff first. Never stage: `Build/`, `compile_commands.json`,
`Build/LspCompileCommands/`, `Build/DevKit/`.

Checkpoint = `git add <intended files only>` + `git commit`. Message:

```text
[step-ref] short description
Refs: IDEA.md step <n>
```

Checkpoint only after the step is completed and validated — never a
broken/unverified state.

### 11.1 Issues (GitHub)

Code changes only: every user wish and every bug (found or fixed) →
GitHub issue in `kuvbur/AddOn_SomeStuff` FIRST (`gh` CLI; workflow —
skill `addon-somestuff-issues`). Backlink in `IDEA.md` and
`Reviews/*.tracker.csv` (column `issue`). Dedup check before creating —
an existing issue may already cover it (comment there instead). Commit
that closes it: `Refs: #N`.

Documentation/instruction-only changes (incl. `AGENTS.md`/`IDEA.md`
maintenance) don't need an issue unless they accompany a code change
that does.

Native Windows Codex: `gh.exe` expected on `PATH` (known install:
`C:\Program Files\GitHub CLI\gh.exe`). If `gh` works but the GitHub API
call fails, check `HTTP_PROXY`/`HTTPS_PROXY`/`NO_PROXY` before assuming
auth is invalid — don't re-login or replace credentials until network
access is verified.

`Reviews/` is gitignored — tracker is local-only (BOM+LF; edit via
Python `utf-8-sig`, not a plain text write).

## 12. IDEA.md

Read `IDEA.md` before non-trivial work; resume the active task from its
state, updating the Plan first if stale.

Plan markers: `[ ]` not started · `[/]` in progress · `[x]` completed
**and validated**.

Before ending a session: update current state, last completed step,
next action, last checkpoint. Completed tasks move to `## Archive` (or
`IDEA_ARCHIVE.md`) — keep the active section short.

`## Scope` bounds the active task; work outside it needs a separate
task. If AC version(s) aren't established, resolve that before
SDK-sensitive edits.

Landmines that outlive the task (architecture constraints, crash
patterns, version differences) go in this file's §6/§14 — not
`IDEA.md`'s "Грабли", which is task-specific and archives with the task.

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

## 13. Project Areas

File responsibilities: `Docs/REPOMAP.md` / `Docs/modules/<module>.md`
(generated, authoritative). Per-file landmines: grep this file for the
filename — headings in §6 and the list in §16 name their file directly.

## 14. BrowserPalette / HTML

Loads HTML from disk: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`,
implemented in `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp`.

- Bridge is inline functions via `RegisterACAPIJavaScriptObject` — see §6
  "Landmine — JS bridge" for the `DynamicCast<JSValue>` crash.
- HTML changes follow `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md`
  only, not ad hoc. Run `Tools/test_html.ps1` after every edit (§9).
- Don't use `html_to_hpp.py`/`HTML_Pages.hpp` — unused here.
- Don't edit `Sources/AddOnResources/` unless required.

## 15. Pre-PR Checklist

Only intended files changed · no user changes overwritten · AC
version(s) known · every touched `ACAPI_*` verified (Hermes LightRAG
skill when available, else installed headers/docs/verified call sites)
or marked `not verified` with reason · clang-format run · LSP checked ·
build done for version(s)+platform · runtime test done where applicable
(HTML: `test_html.ps1` ran) · verified/compiled/tested reported
accurately · no generated files staged · no unrelated refactor · memory
updated · IDEA.md re-read before final write · code changes: GitHub
issue exists and commit references it (`Refs: #N`, §11.1) · if the
change altered a contract/side effect/signature already documented in
`Docs/modules/<module>.md`, that card updated in the same checkpoint
(§17); if out of scope for this task, said so explicitly rather than
leaving it silently stale.

## 16. Known — do not fix without explicit request

Author already decided these are out of scope:

- `Dimensions.cpp:158` — `pen_original`.
- Roombook — recreation of finish elements.
- `Sync.cpp:419-428` — cumulative `epm`.

If root-cause analysis leads here, stop and flag it — this is a
deliberate choice, not an oversight.

## 17. Documentation

Maintained on branch `docs/codebase-map`, built by scripts
(`Docs/tools/generate_symbols.py` + clangd MCP):

- `Docs/ARCHITECTURE.md` — module overview, layers, Mermaid dependency graph.
- `Docs/REPOMAP.md` — repo map and stats.
- `Docs/modules/<module>.md` — one per module (`pk` has the full template
  with "Вызывает"/"Вызывается из").
- `Docs/DISCREPANCIES.md` — comment/code mismatches (fix nothing without
  confirmation).
- `Docs/_generated/symbols.json` / `callgraph.json` — deterministic
  clangd output.
- `Docs/_progress.md` — documentation work state (sole resume source).
- `Docs/tools/generate_symbols.py` — symbols.json generator (callHierarchy
  is collected via clangd MCP — `subprocess.PIPE` doesn't work on
  Windows).
- `Docs/tools/UPDATE_PROCEDURE.md` — step-by-step doc-update procedure
  (regenerate symbols/callgraph, update module cards, DISCREPANCIES.md,
  overview files, commit). Read it before any doc-update task; it also
  settles the commit-scenario split noted below.

Rule: when code changes, update the matching `Docs/modules/<module>.md`
and `_generated/`.

Workflow role (§3-5): read the module card and `DISCREPANCIES.md` BEFORE
raw-source digging and live clangd/LightRAG queries in a familiar area —
it's a ready contract, side effects, and (where collected) callgraph,
not a license to skip §6 SDK verification for the specific change. Full
"Вызывает"/"Вызывается из" template — `pk/` only; elsewhere those fields
are `[не проверено]` — don't trust them.
