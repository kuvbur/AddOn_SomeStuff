# AGENTS.md — AddOn_SomeStuff (ArchiCAD C++ Add-On)

## What this project is

**SomeStuff** — ArchiCAD Add-On (C++, ACAPI SDK, GPL-3.0). Automates: GDL ↔ Property sync, flexible numbering, structure layer composition export, value summation, coordinates/angles, dimension tools, IFC property copy, project info, morph length, auto-classification, composite decomposition, MEP data, layout tracking, finish schedule + QR codes.

Repo: https://github.com/kuvbur/AddOn_SomeStuff  
Supported: AC 22–29, Windows / macOS. Bilingual UI (RUS/INT, auto-detected).

---

## Repository structure

```
Sources/AddOn/                # .cpp / .h sources (core + modules)  ← ALL SOURCE FILES ARE HERE
  api_headers/                # Per-version APICommon headers (AC22–AC29)
  json_commands/              # JSON API command handlers (CommandBase, GetPropertyDefinitions, Health, etc.)
  third_party/                # Embedded third-party libs (exprtk, alphanum, qrcodegen)
Sources/AddOnResources/       # Resources (RFIX, RINT, platform-specific)
  RFIX/AddOnFix.grc           # Fixed resource definitions
  RFIX/HTML/                  # **HTML UI для BrowserPalette (подгружается напрямую)**
  RFIX/Images/*.svg           # Menu icons (18×18)
  RINT/AddOn.grc              # Generated from AddOn.grc.in (do NOT edit manually)
  RFIX.win/*.rc2              # Windows resource scripts
  RFIX.mac/*.plist            # macOS property lists
Sources/MacDarkModeIcon/      # macOS dark mode icon assets
Tools/
  CMakeCommon.cmake           # Shared CMake: AC version detect, compiler flags, libs
  BuildAddOn.py               # Python wrapper: downloads DevKit, configures CMake, builds, packages
  CompileResources.py         # Resource compiler wrapper
Test_file/                    # Test .pln files per version (test_25.pln … test_29.pln)
CMakeLists.txt                # Entry point: version, name, language, includes CMakeCommon
config.json                   # BuildAddOn.py config: DevKit URLs per version/platform, languages
.github/workflows/            # CI: build_25+.yml (AC 25-29), build_23-24.yml (AC 23-24)
wiki/                         # Docs, images, example files
```

> **⚠️ IMPORTANT**: All C++ source files (`.cpp` / `.h`) are located in `D:\\SomeStuff_addon\\Sources\\AddOn\\`.  
> Always search/read there — NOT in the repo root or other folders.

---

## HTML UI Interface (BrowserPalette)

**Architecture:**

- `BrowserPalette` (C++) — `DG::Palette` + `DG::Browser`
- HTML загружается **напрямую из файла**: `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` через `DG::Browser::LoadURL()`
- Регистрирует `DG::JSObject("ACAPI")` с функциями для вызова из JS

**Важно:** НЕ используются:

- `html_to_hpp.py` — конвертер HTML → C++ raw string literal
- `HTML_Pages.hpp` — сгенерированный заголовочный файл

**Ключевые файлы:**
| Файл | Назначение |
|------|------------|
| `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` | Основной HTML интерфейс (vanilla JS, без CSS файлов) |
| `Sources/AddOn/dialogs/BrowserPalette.cpp` | Загрузка HTML, регистрация `ACAPI` JS объекта |
| `Sources/AddOn/dialogs/BrowserPalette.hpp` | Объявления методов |

**JS функции в `ACAPI` объект:**

- `ACAPI.GetPropertyDefinitions()` — возвращает массив свойств из `PROPERTYCACHE()`
- `ACAPI.GetPropertyDescription(name)` — описание свойства из кэша
- `ACAPI.GetPropertyValue(name)` — значение свойства для выделенного элемента
- `ACAPI.ParsePropertyDescription(desc)` — парсинг описания (Renum/Sync/Spec)
- `ACAPI.SetPropertyDescription(name, desc)` — запись описания свойства

---

## C++ Code Navigation & Context Rules

When looking for function definitions, classes, symbols, or architectural context in C++, you MUST follow this strict priority order:

1. **First Choice: Clangd MCP**
   Always attempt to find exact C++ symbols, definitions, and declarations using `clangd-mcp` tools first (`workspace_symbol_search`, `find_definition`, `find_references`). It provides exact AST-based locations without context noise.

2. **Second Choice (Fallback / Context Search): LightRAG**
   If `clangd-mcp` fails to locate the symbol, or if the request requires higher-level architectural context, relationships, or conceptual understanding:
   - Call the **LightRAG** skill **BEFORE** forming your response.
   - **query**: Formulate a concise query in English describing the function/class names, modules/files, or key concepts (e.g., threading, memory, system architecture).
   - **mode**: `"hybrid"` (unless explicitly instructed otherwise).

3. **Strict Constraints & Execution Guidelines**
   - **Base responses on retrieved context:** If retrieved context or code exists from Clangd/LightRAG, use it as the ground truth.
   - **No Hallucinations:** Do NOT invent architecture, function signatures, or APIs if they exist in the codebase/context.
   - **Acknowledge Gaps:** If neither tool yields sufficient context, explicitly state what is missing and ask for clarification.
   - **Zero Bypass Rule:** NEVER answer C++ code navigation or architectural questions without attempting symbol search via Clangd MCP first, followed by LightRAG skill if needed. When in doubt, search first.

---

### Code Editing & Indentation Policy

To prevent indentation errors and issues with patching tools:

1. **Never rewrite large files entirely** just because of whitespace/indentation mismatches during patching.
2. **Auto-format edited files:** Right after making any edits to a C++ file (`.cpp`, `.hpp`, `.h`), run `clang-format` via terminal to ensure perfect adherence to project style:
   `clang-format -i path/to/edited_file.cpp`. Keep ALL changes produced by `clang-format` without manual rollback.
3. **Pre-formatting before patching (Optional):** If a patch fails due to whitespace mismatches, run `clang-format -i` on the target file first, then re-apply the patch.
4. **Trust `clang-format` on include ordering:** Do NOT manually revert or rearrange `#include` directives if `clang-format` modifies their order.
   - Putting the paired header (e.g., `#include "TestFunc.hpp"` inside `TestFunc.cpp`) as the FIRST include is standard LLVM/Google/C++ behavior to verify header self-sufficiency.
5. **Do NOT fight `clang-format`:** If `clang-format` reorganizes lines or includes, accept its output as the project truth. Do NOT treat `clang-format` changes as style violations or errors.

---

## How to build (normal build)

**Example / default version for manual local builds:** Archicad 25 (AC25) — see "Archicad Version Handling" below for how to pick the right version for an actual task; this project supports AC 22–29, this is not a fixed target.

Requires **Developer Command Prompt for VS** (provides `INCLUDE`/`LIB`/MSVC toolset).

```bash
python Tools\BuildAddOn.py -c config.json -v <version> [--release] [--package]
```

- `-v <version>` — e.g. `25`, `27`; multiple versions space-separated
- Without `--release` → Debug build, single version, no language selection
- `--release` → RelWithDebInfo for all languages from config.json (or `-l <LANG>`)
- `--package` → pack `.apx`/`.bundle` into `Build/Package`
- DevKit auto-downloaded to `Build/DevKit/APIDevKit-<version>`; or supply local `-d <path>` (then `-v` must be single)

AC version auto-detected from `ACAPinc.h` in DevKit (`DetectACVersion` in `CMakeCommon.cmake`). C++ standard / toolset derive from it:

- AC < 27 → C++14, toolset v140 / v141 / v142
- AC < 29 → C++17
- AC ≥ 29 → C++20
- `/WX` (warnings-as-errors) ON; many `/wd####` intentionally suppressed — don't re-enable without reason
- PCH: `AddOn.hpp` via `target_precompile_headers`

---

## Archicad Version Handling

- This project has **no single default AC version** — it supports AC 22–29 with per-version C++ standard differences (C++14 <27, C++17 <29, C++20 ≥29) and two separate CI matrices (23-24 vs 25-29). Do not assume AC25 for a task just because it appears as the example in "How to build" above.
- Before writing version-sensitive code (anything touching a C++17/20 feature, or an ACAPI symbol that differs across versions), establish which AC version(s) the current task targets: from the task/issue description, the relevant `Test_file/test_<version>.pln`, or by asking — do not guess silently.
- If a change must behave differently across the supported range, guard it explicitly (e.g. `#if` on the detected version) rather than writing code that happens to work only on the version you tested against.

---

## Archicad API (ACAPI) Usage — Mandatory RAG Verification

- Before writing or modifying any `ACAPI_*` call, Search the ArchiCAD C++ SDK knowledge base: functions, types, code examples, past decisions via lightrag skill.
- This is mandatory verification, not a fallback. Unlike the Clangd-first / LightRAG-fallback ordering above — which is for navigating _this project's own_ symbols — an actual call into the Archicad SDK always goes through LightRAG first, regardless of whether Clangd can already resolve the symbol. Clangd confirms a function exists and its declared signature; only the documentation/example corpus confirms it's being called correctly, and for the right AC version (see "Archicad Version Handling" above — the ACAPI surface is versioned).
- Query convention: same as the navigation rule above — concise English query naming the exact ACAPI function/struct/pattern, `mode: "hybrid"` unless told otherwise.
- If RAG has no relevant coverage for a specific call, say so explicitly and ask rather than guessing from memory.
- Known landmine already documented in this file (treat as pre-verified — no need to re-check): `ACAPI_Element_GetMemo` requires `BNZeroMemory(&memo, sizeof(memo))` first.

---

## Git & Checkpoint Discipline

- **Clean tree before editing**: run `git status` before making any change. Uncommitted changes unrelated to the current task → stop and flag, don't build on top of them or silently commit/discard them.
- **Checkpoint commits are mandatory**: commit, marked `WIP:`, after every verified sub-task — build succeeds, no new warnings/errors for the AC version(s) in scope. This is what makes autonomous execution safe: a mistake rolls back to the last checkpoint, not to the start of the task.
- **Tip must build**: squash WIP commits into one clean commit once the task is done; never leave a non-building commit as the branch tip.
- **Never destroy the rollback trail**: no `git clean -fdx`, no deleting untracked files, no deleting files outside the current task's scope.
- **Never rewrite shared history**: no `git commit --amend`, `git rebase -i`, `git push --force` on any branch already reviewed or shared.
- **Never stage blindly**: no `git add .` / `git add -A` without reading `git status`/`git diff` first — this repo's build/LSP artefacts (`compile_commands.json`, `Build/`) must never be staged.
- Commit messages reference the task/issue (this project has no phase structure, unlike other repos this agent works in).

---

## Session State — `IDEA.md`

This repo has no fixed implementation phases — work arrives as discrete tasks/bugs/features across many separate agent sessions with no shared memory between them. For any task expected to span more than one sitting, keep `IDEA.md` at the repo root (create if missing, same file name/role this agent uses in other projects — just task-keyed here instead of phase-keyed):

```markdown
# Current Task

## Task

[Short description / issue reference]

## Current State & Resume Marker

- **Status:** [IN_PROGRESS / WAITING_FOR_TEST / BLOCKED]
- **Last Action Completed:** [...]
- **Immediate Next Step:** [...]

## Tactical Step-by-Step Plan

- [x] Completed step
- [/] Active step
- [ ] Upcoming step

## Execution Log & Decisions

- [Step]: action taken, files touched, outcome.
```

Sync it before and after any significant step. If it isn't written down here, treat it as not done when resuming — this is the checkpoint unit the git checkpoint-commit rule above (and this agent's general checkpoint/rollback rules) refers to for this repo.

---

## Compiler flags & C++ versions (from CMakeCommon.cmake)

| AC version | C++ std | MSVC toolset       |
| ---------- | ------- | ------------------ |
| < 27       | 14      | v140 / v141 / v142 |
| < 29       | 17      | v142 / v143        |
| ≥ 29       | 20      | v143               |

`/W3 /WX /Zc:wchar_t- /EHsc /bigobj /wd4499 /wd5208 /wd4996 /wd4003` + many suppressed warnings.  
macOS: `-Wall -Wextra -Werror -fvisibility=hidden` + long suppression list.

---

## LSP (clangd) — NOT the same as normal build

Normal build uses VS generator (multi-config) → **no `compile_commands.json`**.  
For LSP a separate Ninja + `clang-cl` config is used (MSVC `cl.exe` fails to extract system includes).

```bash
python Tools\BuildAddOn.py -c config.json -v <version> --lsp
```

Also from **Developer Command Prompt**. Generates `Build/LspCompileCommands/<version>/compile_commands.json` and copies to repo root.

**Do NOT commit:** `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/` — machine-specific artefacts.

Regenerate after: AC version change, `CMakeCommon.cmake` edits, new `.cpp` files in `Sources/AddOn`.

---

## Key source files & responsibilities (from `Sources/AddOn/`)

| File                           | LOC   | Role                                                                                                                                                                                                                                                                                                                                                                |
| ------------------------------ | ----- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **SomeStuff_Main.cpp/hpp**     | 528   | Entry point: `CheckEnvironment`, `RegisterInterface`, `Initialize`, `FreeData`; menu dispatcher (`MenuCommandHandler`), observers (`ElementEventHandlerProc`, `ProjectEventHandlerProc`, `ReservationChangeHandler`, `SelectionChangeHandlerProc`), `Do_ElementMonitor`, menu state sync                                                                            |
| **Helpers.cpp/hpp**            | 9 877 | **Core engine**: `ParamHelpers` namespace — property read/write (`ElementsRead`/`ElementsWrite`/`WriteProperty`), `ParamValue`/`ParamDictElement`/`ParamDictValue` (internal property currency), format-string parsing (`FormatStringFunc`), element selection/filtering, coordinate/angle helpers, QR-code, classification, attribute cache, GDL parameter parsing |
| **Propertycache.cpp/hpp**      | 1 102 | Singleton `PropertyCache`: caches property definitions, classifications, attributes, project info, geo-location, MEP (AC29+), format strings, composite layer data; `Update()` refreshes all                                                                                                                                                                        |
| **Sync.cpp/hpp**               | 2 685 | `SyncAndMonAll`, `SyncSelected`, `MonAll`/`MonByType` (reactive observer attach), `SyncByType` per element type; throttling (500 ms dedup cache)                                                                                                                                                                                                                    |
| **Roombook.cpp**               | 5 656 | Finish schedule: zone-based collection of walls/columns/slabs/doors/windows → composite layers → material lookup → favourite matching → element create/update                                                                                                                                                                                                       |
| **Spec.cpp**                   | 2 316 | Specification rules from property descriptions (`Spec_rule{…}`); classification filtering; writes list data to properties                                                                                                                                                                                                                                           |
| **Summ.cpp**                   | 504   | Summation of property values across elements → target property / project info                                                                                                                                                                                                                                                                                       |
| **ReNum.cpp**                  | 933   | Renumbering by property criteria (ID Manager alternative)                                                                                                                                                                                                                                                                                                           |
| **Revision.cpp**               | 1 044 | Revision markers / change clouds                                                                                                                                                                                                                                                                                                                                    |
| **Dimensions.cpp**             | 368   | Dimension rounding, formula writing (`6×100=600`)                                                                                                                                                                                                                                                                                                                   |
| **ClassificationFunction.cpp** | 254   | Auto-classification by property values                                                                                                                                                                                                                                                                                                                              |
| **ResetProperty.cpp**          | 390   | Reset properties to default / clear                                                                                                                                                                                                                                                                                                                                 |
| **AutomateFunction.cpp**       | 987   | Profile-by-line, drawing alignment                                                                                                                                                                                                                                                                                                                                  |
| **MEPv1.cpp**                  | 1 202 | MEP system/group/description output (AC28+)                                                                                                                                                                                                                                                                                                                         |
| **DG4rule.cpp**                | 163   | DG rules for dialogs                                                                                                                                                                                                                                                                                                                                                |
| **TestFunc.cpp**               | 1 495 | Debug helpers under `TESTING`                                                                                                                                                                                                                                                                                                                                       |
| **qrcodegen.cpp**              | 829   | QR code generation (finish schedule)                                                                                                                                                                                                                                                                                                                                |
| **Spec_libpart.cpp**           | 553   | Spec string parsing                                                                                                                                                                                                                                                                                                                                                 |
| **SyncSettings.cpp/hpp**       | 101   | Settings serialisation to Add-On Preferences (`ACAPI_Get/SetPreferences`)                                                                                                                                                                                                                                                                                           |
| **CommonFunction.cpp**         | 2 543 | Element selection helpers, story handling, QR utils, debug print (`DBprnt`)                                                                                                                                                                                                                                                                                         |

---

## What NOT to do

- ❌ Don't use C++17/20 syntax in code compiled for AC < 27 / < 29 — hard compiler error
- ❌ Don't edit `Sources/AddOnResources/`
- ❌ Don't commit `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/`
- ❌ Don't re-enable suppressed `/wd####` warnings without reason
- ❌ Don't call `ACAPI_Element_GetMemo` without `BNZeroMemory(&memo, sizeof(memo))` first
- ❌ The only way to query LightRAG is bash_tool with curl. Read lightrag skill for details.
- ❌ Don't assume AC25 as this repo's target version — see "Archicad Version Handling" above

---

## Debug workflow

- Debug build copies `Test_file/test_<version>.pln` → build dir
- VS debugger launches ArchiCAD with that `.pln` (`VS_DEBUGGER_COMMAND` set in `CMakeCommon.cmake`)
- `DBprnt` / `msg_rep` output → ArchiCAD Report Window (only in Debug / `TESTING`)

---

## CI / Pre-PR checklist

- [ ] Build passes locally for target AC version(s) (`-v 25 27 29`)
- [ ] No C++17/20 syntax in code paths compiled for AC < 27 / < 29
- [ ] New `.cpp` files appear in CMake `file(GLOB …)` (auto) or add manually
- [ ] Run `--lsp` once after adding files so `clangd` sees them
- [ ] CI runs both workflows (23-24 + 25-29) on push

---

## Useful links

- Wiki (EN): D:\SomeStuff_addon\wiki\en
