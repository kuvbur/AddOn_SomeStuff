# AGENTS.md — AddOn_SomeStuff

Repo-specific rules only. Global Codex behavior lives outside this file.

## 1. Project

- ArchiCAD Add-On, C++, versions **22–30** (config `devKitLinks` 22–30;
  `AC_30` guard in `CommonFunction.hpp`). No default version — decide from
  task → build config → existing `#if` blocks → test files → ask the user.
- Platforms: **Windows + macOS**.
- C++ source: `Sources/AddOn/` only — never search C++ outside it, no
  recursive disk/repo search. Don't edit `Sources/AddOnResources/` unless the
  task needs it.

## 2. Structure

Current structure: `Docs/REPOMAP.md` (generated, authoritative; §13, §17).
Multi-session task state: `IDEA.md` (§12).

## 3. Workflow

```
IDEA.md → knowledge/landmines → Docs/modules/<area>.md (if it exists) →
Sources/AddOn/ only → determine AC version(s) → verify SDK/API →
root cause → minimal change → clang-format → LSP → Build →
Runtime (as needed) → git diff → checkpoint → answer
```

Code changes need a GitHub issue first (§11.1).

`Docs/` (module cards, `_generated/`, `_progress.md`, `ARCHITECTURE.md`,
`REPOMAP.md`) is **read-only during work**. Updating it is a separate step
before the commit, on the owner's explicit command (§17). Until then the
`IDEA.md` Plan keeps the item `[ ] Docs/ update before checkpoint`.

## 4. Project Knowledge

OpenViking is unavailable in this Codex runtime — don't search for it,
request it, or treat its absence as a blocker.

Before investigating errors, non-trivial bugs, unfamiliar symbols, `ACAPI_*`
calls, or architecture decisions:

1. Read the active state in `IDEA.md`.
2. Check landmines/constraints in this file (index: §6).
3. If `Docs/modules/<module>.md` exists for the area, read it (contract, side
   effects, callers/callees; §17). Check `Docs/DISCREPANCIES.md` before
   trusting a comment. Docs are a starting point, not a substitute for
   verifying the detail you touch: they carry a commit hash — if the file
   changed materially since, trust live inspection and treat the doc as stale.
4. Inspect the actual source and Git history when useful.
5. Use global Codex engineering memory per the global instructions.

Durable findings that outlive the task go in this file's landmine sections.
Current task state goes in `IDEA.md`.

## 5. C++ Navigation

- Call-site questions (who calls X / what X calls): check
  `Docs/_generated/callgraph.json` first. It has a full `pk/` slice plus a
  stale Roombook/Spec slice (generated 2026-10-02); elsewhere
  `[не проверено]` (`Docs/_progress.md`). A `pk/` hit saves a live query;
  a Roombook/Spec hit may be stale — check `meta.generated` and fall through
  to clangd/grep on doubt; a miss is not evidence of absence.
- Definitions/references/locations: Clangd MCP, then targeted inspection
  inside `Sources/AddOn/` only.
- SDK/API context: skill `lightrag` (§6).

## 6. SDK Verification & Landmines

Before touching any `ACAPI_*` call, use skill `lightrag`. It covers the Hermes
LightRAG lookup, the pre-edit checklist (signature, ownership, lifetime,
transaction/undo, version differences, etc.), and the fallback chain
headers → docs → verified call sites.

**Landmine index** (full text in the named section):
JS bridge (below) · Archicad profiles (§9) · shared `Helpers.cpp` (§11) ·
resource string IDs and `.apx` encoding (§15.1) · docs location (§17) ·
known do-not-fix items (§16).

**Landmine — JS bridge.** The bridge is inline functions registered via
`RegisterACAPIJavaScriptObject` (§14). Parse `JSFunction` arguments via
`DynamicCast<JSValue>` — `DynamicCast<JSArray>` crashes ArchiCAD (latent
R1–R2 crash). JSON commands were removed (b7a996b) — do not reintroduce
them. The bridge's reply is a JSON string: nested `DG::JSObject`/`JSArray`
passed to JS are silently lost — flatten the reply.

**Landmine — HashTable iteration (AC28+).** The `CurrentPair` field type
changed from pointers to references in AC28+, but access is always
`operator->`: `cIt->key` is correct for AC22–29, `cIt.key` does not compile
in any version (the dot form is valid only in a range-for).

## 7. C++ Editing

Follow the global Codex coding instructions — no additional repo rules.

## 8. Formatting

After every `.cpp`/`.hpp`/`.h` edit, format **only the changed lines**:
`git clang-format` (tracked files) or `clang-format -i --lines=<from>:<to>
file`; a brand-new file may be formatted whole. Keep the output — don't
hand-fix indentation/includes, and never reformat a whole existing file just
to format it. If `git clang-format` pulls in unrelated regions (observed in
`BrowserPalette.cpp`), revert those hunks — keep only the lines you edited.

## 9. Validation

**LSP** — inspect via Clangd; regenerate the compile DB only if needed:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version> --lsp`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version> --lsp`

**Build** — only when no Visual Studio MCP debug session is active. Claim
`compiled` only after an actual successful build on the target platform:

- Win: `python Tools\BuildAddOn.py -c config.json -v <version>`
- Mac: `python3 Tools/BuildAddOn.py -c config.json -v <version>`

Debug-loop builds via VS MCP `build_solution` do **not** count: `compiled`
can be claimed only after a real `BuildAddOn.py` build or
`restart_archicad_for_test.ps1` — i.e. after the debug session ends.

**Runtime** — only when no VS MCP debug session is active.
- Win: run `Tools\restart_archicad_for_test.ps1` from the repo root
  (`D:\SomeStuff_addon`) — final-check build + Archicad launch; it also runs
  the JSON command tests (`Tools/test_json_commands.py`, exit 70 on failure),
  but does not collect C++ test results.
- Mac: no equivalent in the repo → say so, ask, don't invent one.
- Claim `tested` only if the behavior was actually executed and observed.
- C++ test/debug output (`DBprnt`/`DBtest`) is read from the Visual Studio
  Debug («Отладка») output pane via VS MCP `output_read`, not from a results
  file.

**Landmine — don't clean Archicad profiles.** Never delete
`%LOCALAPPDATA%\GRAPHISOFT\ARCHICAD__*` yourself: temp directories may hold
needed projects. If Archicad never reaches its window (empty
`MainWindowTitle`, port 19723 silent), report it and ask the user to clean up
(owner's decision, 2026-10-01).

**Debug (Visual Studio MCP)** — for live investigation (breakpoints, locals,
call stack) use skill `visualstudio-cpp-debugger`. The repo's loading path,
the debugger/runner conflict, and how to confirm a build relinked are in that
skill's `archicad-somestuff.md` reference — don't repeat them here. If
observed runtime behavior confirms or contradicts a `Docs/modules/<module>.md`
card, add one line to `IDEA.md` Observations; the card itself is edited later,
before commit (§17).

**HTML interface** — after every `Interface_ru.html` edit run
`powershell -File Tools/test_html.ps1` before claiming `tested`.

## 10. Tests

`Sources/AddOn/tests/TestFunc.cpp/.hpp`, active under `TESTING`.

- Task = write/fix tests → **production code is read-only** (inspect, don't
  edit).
- A test exposing a prod bug → stop, report file/function/why; the fix is a
  separate task.
- Never modify `Test_file/*.pln` in a normal test task.

## 11. Git Safety

`git status` before touching files. Never discard uncommitted user changes or
touch unrelated ones. Never: `clean -fdx`, `commit --amend`, `rebase -i`,
`push --force`, `add .`/`add -A` without checking status/diff first. Never
stage `Build/` (incl. `Build/LspCompileCommands/`, `Build/DevKit/`) or
`compile_commands.json`.

**Branching.** `master` is the trunk.
- **Never create branches** — including per-module ones (owner's ban,
  2026-11-06; the earlier feature-branch rule is withdrawn). Work happens on
  `master`; doc-only changes and small fixes go straight to it. Existing
  local branches (`roombook_refactor`, `spec_refactor`, `gh-pages`) are
  pre-ban leftovers, not a template.
- Never: `push --force` on shared branches (ask the owner if a pushed branch
  needs rewriting), history rewriting.
- `Helpers.cpp` is shared by ~29 files (its header included across the
  addon): take it under lock — one task at a time, keep such changes out of
  parallel work.

**Checkpoint** = `git add <intended files only>` + `git commit`, only after
the step is completed and validated — never a broken/unverified state.
Message:

```text
[step-ref] short description
Refs: IDEA.md step <n>
Refs: #N            (code changes only; §11.1)
```

### 11.0 Attribution

Never add `Co-Authored-By`, `Generated with`, or any tool/model attribution
to commit messages, issue bodies, or PR descriptions unless the owner asks.
The owner never has; a few commits carried a false
`Co-Authored-By: Claude Opus 4.8` (2026-10-03). If attribution seems
warranted, ask.

Corollary: when asked why something happened, check what actually produced it
(tool call in the session log, git metadata, config, hooks) before answering.
"I don't know yet" beats a plausible story; searching only for evidence of an
existing guess is not verification.

### 11.1 Issues (GitHub)

- Code changes only: every user wish and every bug (found or fixed) → a
  GitHub issue in `kuvbur/AddOn_SomeStuff` FIRST (`gh` CLI; skill
  `addon-somestuff-issues`). Backlink it in `IDEA.md`. Dedup first — if an
  issue exists, comment there. The closing commit carries `Refs: #N`.
- Documentation/instruction-only changes (incl. `AGENTS.md`/`IDEA.md`) need no
  issue unless they accompany a code change that does.
- Before closing an issue run `Tools\restart_archicad_for_test.ps1` (§9), even
  after a thorough debug session: a debug session confirms behavior at a
  breakpoint, not the build+load path. The runner proves build+load only, not
  that tests passed — read `DBprnt`/`DBtest` output via VS MCP `output_read`.
- Native Windows Codex: `gh.exe` expected on `PATH` (known install:
  `C:\Program Files\GitHub CLI\gh.exe`). If `gh` works but a GitHub API call
  fails, check `HTTP_PROXY`/`HTTPS_PROXY`/`NO_PROXY` before assuming auth is
  invalid; don't re-login or replace credentials until network access is
  verified.
- `Reviews/` is gitignored — a local-only review tracker (BOM+LF; edit via
  Python `utf-8-sig`, not a plain text write).

## 12. IDEA.md

Read `IDEA.md` before non-trivial work and resume the active task from its
state, updating the Plan first if stale.

- Plan markers: `[ ]` not started · `[/]` in progress · `[x]` completed **and
  validated**.
- The Plan must contain `Docs/ update before checkpoint` — `[ ]` until the
  owner commands the commit; mark `[x]` only after the docs were actually
  updated (§17). It replaces intermediate card/`_generated/` updates.
- `## Scope` bounds the active task; out-of-scope work is a separate task. If
  AC version(s) aren't established, resolve that before SDK-sensitive edits.
- Before ending a session update: current state, last completed step, next
  action, last checkpoint. Move completed tasks to `## Archive` (or
  `IDEA_ARCHIVE.md`); keep the active section short. Not a transcript of tool
  calls.
- Durable landmines (architecture constraints, crash patterns, version
  differences) go in this file (§6), not in `IDEA.md` Pitfalls, which is
  task-specific and archived with the task.

```markdown
# Current Task
## Task
## Scope
## Status          <!-- IN_PROGRESS / WAITING_FOR_TEST / BLOCKED -->
## Last Completed
## Next Step
## Last Checkpoint
## Plan            <!-- - [x] done  - [/] in progress  - [ ] todo -->
## Observations    <!-- one-liners from runtime/debug (§9) -->
## Pitfalls        <!-- task-specific only -->
## Decisions
## Archive
```

## 13. Project Areas

File responsibilities: `Docs/REPOMAP.md` and `Docs/modules/<module>.md`
(generated, authoritative). Per-file landmines: grep this file for the
filename (§6 index, §16).

## 14. BrowserPalette / HTML

Loads HTML from disk: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`,
implemented in `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp`.

- Bridge = inline functions via `RegisterACAPIJavaScriptObject` — see the JS
  bridge landmine in §6.
- HTML changes follow `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` only,
  not ad hoc. Run `Tools/test_html.ps1` after every edit (§9).
- Don't use `html_to_hpp.py`/`HTML_Pages.hpp` — not present in the repo; any
  HTML-in-HPP approach is out of scope here.

## 15. Pre-PR Checklist

- Only intended files changed; no user changes overwritten; no generated
  files staged; no unrelated refactor.
- AC version(s) known.
- Every touched `ACAPI_*` verified (§6) or marked `not verified` with reason.
- clang-format run (§8); LSP checked.
- Build done for version(s)+platform (VS MCP debug-loop builds don't count,
  §9).
- Runtime test where applicable: HTML → `test_html.ps1` ran; issue-closing
  changes → `restart_archicad_for_test.ps1` ran, not just a debug session
  (§11.1).
- `verified`/`compiled`/`tested` reported accurately.
- Code changes: GitHub issue exists and the commit has `Refs: #N` (§11.1).
- Memory updated; `IDEA.md` re-read before the final write.
- `Docs/` updated **only if the owner commanded the commit** (§17): then the
  affected module card, `_generated/`, and `_progress.md` go into the same
  checkpoint. Otherwise `IDEA.md` keeps the `[ ]` item and the answer states
  explicitly that the card is stale — never leave it silently looking
  current.

## 15.1 Resource String IDs (`AddOn.grc.in`)

**A resource string's ID is its ORDINAL POSITION in the `'STR#'` block, not
the number in the `/* [ N ] */` comment** (the preprocessor, `cl /E`, strips
comments).

**Landmine — block `32501` row gap is historical.** There used to be a gap
at rows 88–89 shifting everything after by two (comment `[ 93]` was ID 91).
As of 2026-11-19 the block is complete (104 rows, comments match ordinals),
but the rule stands: **never trust the comment — recount rows** after any
`.grc` edit.

Verify ID → text after any `.grc` edit (build first, or reuse the last
build):

```bash
python - <<'EOF'
import re
s = open(r'Build/SomeStuff/25/ResourceObjects/AddOn.grc.i', encoding='utf-8').read()
i = s.find(' 32501 '); j = s.find('{', i); k = s.find('\n}', j)
rows = re.findall(r'"((?:[^"\\]|\\.)*)"', s[j:k])
for cid in (90, 91, 92): print(cid, repr(rows[cid-1][:40]))   # 1-based!
EOF
```

Row index N-1 in `.grc.i` = resource ID N. `32000/32500/32501` are RU
blocks, `33000/33500/33501` ENG.

**Landmine — text is stored in `.apx` as UTF-8**, not UTF-16:
`"string".encode('utf-16-le') in apx` gives a false negative.

## 16. Known — do not fix without explicit request

Decided out of scope by the author:

- `Dimensions.cpp` — `pen_original`.
- Roombook — recreation of finish elements.
- `Sync.cpp` — cumulative `epm`.

If root-cause analysis leads here, stop and flag it — this is a deliberate
choice, not an oversight.

## 17. Documentation

Contents (`_generated/` is collected via clangd MCP,
`textDocument/documentSymbol` per file — see
`Docs/tools/UPDATE_PROCEDURE.md` §2):

- `Docs/ARCHITECTURE.md` — module overview, layers, Mermaid dependency graph.
- `Docs/REPOMAP.md` — repo map and stats.
- `Docs/modules/<module>.md` — one per module (`pk` has the full template with
  "Вызывает"/"Вызывается из").
- `Docs/DISCREPANCIES.md` — comment/code mismatches (fix nothing without
  confirmation).
- `Docs/_generated/symbols.json`, `callgraph.json` — deterministic clangd
  output; `source_hashes.json` — sha256 of sources for freshness checks.
- `Docs/_progress.md` — documentation work state (sole resume source).
- `Docs/tools/UPDATE_PROCEDURE.md` — step-by-step doc-update procedure; read it
  before any doc-update task.
- `Tools/check_docs.py` — checks path existence, no line numbers in cards,
  source freshness by hash, lag of `Состояние кода: <hash>`; `--update`
  rewrites hashes, `--strict` treats STALE as an error.

**Update rule — only before a commit, on command.** `Docs/` is not touched in
intermediate steps (not after a code edit, a successful run, or a review). It
is a separate stage started by the owner's **explicit commit command**. The
rule saves time and tokens; the side effect is lagging docs, which is
acceptable while `IDEA.md` holds the open `Docs/ update before checkpoint`
item.

Order before commit: code already built and verified → on command, collect
`symbols.json`/`callgraph.json`, update affected `Docs/modules/*.md`,
`DISCREPANCIES.md`, and `ARCHITECTURE.md`/`REPOMAP.md` if structure changed,
write `_progress.md` → `python Tools/check_docs.py --update` → clean
`python Tools/check_docs.py` report → `git diff` → one checkpoint with code
and docs.

**No line numbers in cards.** A card references symbol names; the only source
of coordinates is `Docs/_generated/symbols.json`. Enforced by
`Tools/check_docs.py` (category `COORD`); don't add coordinates to
`Docs/modules/`.

**Exceptions — document during the work:**
- A comment/code mismatch that would be lost without a record →
  `DISCREPANCIES.md` (a line in `IDEA.md` is acceptable instead of the file).
- A change that already misleads the next card reader into a wrong contract
  conclusion → fixing the card is part of the step (say so in the answer).

**Landmine — docs location.** There is no separate doc branch, and don't name
one when looking for missing docs. `Docs/` ships in the code's checkpoint on
`master` (work happens on `master`, §11). If
a `Docs/` change seems missing, verify the _content_
(`git log --oneline -1 master -- Docs/`, or whether a card mentions recent
work) instead of hunting for another branch.

**Role in the workflow (§3–5).** Read the module card and `DISCREPANCIES.md`
BEFORE raw-source digging and live clangd/LightRAG queries in a familiar
area: it is a ready contract, with side effects and (where collected) a
callgraph — not a license to skip §6 SDK verification for the specific
change. The full "Вызывает"/"Вызывается из" template exists for `pk/` only;
elsewhere those fields are `[не проверено]` — don't trust them.
