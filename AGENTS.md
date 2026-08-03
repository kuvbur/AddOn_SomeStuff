# AGENTS.md — AddOn_SomeStuff (ArchiCAD C++ Add-On)

This file defines mandatory rules for any AI agent working on this repository.
Follow every rule literally. Do not skip, reorder, or reinterpret steps marked as mandatory.

If any instruction in this file conflicts with your general defaults, this file wins.
If any instruction in this file is unclear or two rules seem to conflict, **stop and ask the user** — do not guess.

**Relationship to SOUL.md:** this repo is also governed by a separate `SOUL.md`, which sets global agent behavior — output language, verification discipline, autonomy/rollback rules, confidence markers, anti-sycophancy, OpenViking memory. Where the two overlap, `SOUL.md` sets the general rule (e.g. "verify before answering") and this file supplies the repo-specific mechanics (which tool to verify with, what a checkpoint means here). This file is in English for token efficiency — that does not override SOUL.md's rule that user-facing answers and code comments are in Russian.

---

## 0. Quick Reference — Golden Rules

Read this list first. It summarizes the most critical rules in this file. Every item is explained in full further below.

1. All C++ source code lives in `Sources/AddOn/`. Never look for or edit source files elsewhere.
2. Never edit files under `Sources/AddOnResources/`.
3. For C++ symbol lookup: try **Clangd MCP first**, then **LightRAG** if that fails (Section 5).
4. For any `ACAPI_*` call: check **LightRAG first**, always, no exceptions (Section 10).
5. After editing any `.cpp`/`.hpp`/`.h` file, immediately run `clang-format -i` on it (Section 6).
6. Before any `WIP:` commit, run the full 3-step Verification Cascade (Section 7). No step may be skipped.
7. Never assume ArchiCAD version 25 as the task's target — this project supports AC 22–29 (Section 9).
8. Never run `git add .`, `git commit --amend`, `git rebase -i`, or `git push --force` (Section 11).
9. For any task spanning more than one session, keep `IDEA.md` at the repo root up to date (Section 12).
10. If you are missing information needed to proceed safely (AC version, tool unavailable, ambiguous scope) — **ask the user**. Do not guess, do not proceed on an assumption.

---

## Table of Contents

1. Project Overview
2. Glossary
3. Repository Structure
4. HTML UI (BrowserPalette)
5. C++ Code Navigation Rules
6. Code Editing & Indentation Policy
7. Mandatory Verification Cascade
8. LSP (clangd) Configuration
9. ArchiCAD Version Handling
10. ArchiCAD API (ACAPI) Usage — Mandatory RAG Verification
11. Git & Checkpoint Discipline
12. Session State File — `IDEA.md`
13. Session Bootstrap Checklist
14. Key Source Files & Responsibilities
15. Prohibited Actions
16. Debug Workflow
17. Pre-PR Checklist
18. Tool Unavailability Fallback
19. Useful Links

---

## 1. Project Overview

- **Name:** SomeStuff
- **Type:** ArchiCAD Add-On (C++, ACAPI SDK, GPL-3.0)
- **Repository:** https://github.com/kuvbur/AddOn_SomeStuff
- **Supported ArchiCAD versions:** 22–29
- **Supported OS:** Windows, macOS
- **UI languages:** Russian, International (auto-detected)

**Features:** GDL ↔ Property sync, flexible numbering, structure layer composition export, value summation, coordinates/angles, dimension tools, IFC property copy, project info, morph length, auto-classification, composite decomposition, MEP data, layout tracking, finish schedule with QR codes.

---

## 2. Glossary

Read this before the rest of the file if any of these terms are unfamiliar.

| Term           | Meaning                                                                                                                                                                                 |
| -------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **ACAPI**      | ArchiCAD's C++ API — the SDK this add-on is built against. Functions are prefixed `ACAPI_*`.                                                                                            |
| **Add-On**     | A compiled plugin (`.apx` on Windows, `.bundle` on macOS) that ArchiCAD loads at startup.                                                                                               |
| **DevKit**     | ArchiCAD's official SDK package for a specific version, containing headers/libs needed to build an add-on. Downloaded automatically by `BuildAddOn.py` unless a local copy is supplied. |
| **GDL**        | Geometric Description Language — ArchiCAD's scripting language for parametric library parts (objects, doors, windows).                                                                  |
| **LSP**        | Language Server Protocol — used here by `clangd` to give code intelligence (go-to-definition, diagnostics) in editors/agents.                                                           |
| **MCP**        | Model Context Protocol — the mechanism exposing `clangd-mcp` tools to the agent.                                                                                                        |
| **OpenViking** | The agent's cross-session memory store, defined in `SOUL.md`. Must be checked for prior relevant entries before any other verification step (see Sections 5 and 10 of this file).       |
| **PLN**        | ArchiCAD project file format (`.pln`). Test files live in `Test_file/`.                                                                                                                 |
| **RAG**        | Retrieval-Augmented Generation — here, the LightRAG knowledge base of the ArchiCAD SDK (docs, examples, past decisions).                                                                |
| **WIP commit** | "Work In Progress" — a mandatory checkpoint commit made after each verified sub-task (see Section 11).                                                                                  |
| **MEP**        | Mechanical, Electrical, Plumbing — a category of building elements/data (AC28+ feature area in this add-on).                                                                            |
| **IFC**        | Industry Foundation Classes — an open BIM data exchange format; this add-on has IFC property copy functionality.                                                                        |

---

## 3. Repository Structure

```
Sources/AddOn/                 # .cpp / .h source files (core + modules) ← ALL SOURCE CODE IS HERE
  api_headers/                 # Per-version APICommon headers (AC22–AC29)
  json_commands/                # JSON API command handlers (CommandBase, GetPropertyDefinitions, Health, etc.)
  third_party/                  # Embedded third-party libraries (exprtk, alphanum, qrcodegen)
Sources/AddOnResources/         # Resources (RFIX, RINT, platform-specific)
  RFIX/AddOnFix.grc             # Fixed resource definitions
  RFIX/HTML/                    # HTML UI for BrowserPalette (loaded directly, see Section 4)
  RFIX/Images/*.svg             # Menu icons (18×18)
  RINT/AddOn.grc                # Generated from AddOn.grc.in — do NOT edit manually
  RFIX.win/*.rc2                # Windows resource scripts
  RFIX.mac/*.plist              # macOS property lists
Sources/MacDarkModeIcon/        # macOS dark mode icon assets
Tools/
  CMakeCommon.cmake             # Shared CMake config: AC version detection, compiler flags, libraries
  BuildAddOn.py                 # Python wrapper: downloads DevKit, configures CMake, builds, packages
  CompileResources.py           # Resource compiler wrapper
Test_file/                      # Test .pln files per version (test_25.pln … test_29.pln)
CMakeLists.txt                  # Entry point: version, name, language; includes CMakeCommon
config.json                     # BuildAddOn.py config: DevKit URLs per version/platform, languages
.github/workflows/               # CI: build_25+.yml (AC 25–29), build_23-24.yml (AC 23–24)
wiki/                             # Docs, images, example files
```

### RULE: Source file location

All C++ source files (`.cpp` / `.h`) live in:

```
D:\SomeStuff_addon\Sources\AddOn\
```

- Always search and read files there. Do NOT look in the repo root or any other folder for source code.
- ALWAYS construct relative file paths starting from the repository root.
- NEVER query C++ source or header files by filename alone (e.g., NEVER use `TestFunc.cpp`).
- ALWAYS prepend `Sources/AddOn/` when accessing any `.cpp`, `.hpp`, or `.h` file (e.g., `Sources/AddOn/Test

---

## 4. HTML UI (BrowserPalette)

### 4.1 Architecture

- `BrowserPalette` (C++) combines `DG::Palette` and `DG::Browser`.
- The HTML file is loaded **directly from disk** via `DG::Browser::LoadURL()`:
  `Sources/AddOnResources/RFIX/HTML/Interface_ru.html`
- The add-on registers a JavaScript object `DG::JSObject("ACAPI")` that exposes C++ functions to the HTML/JS front end.

### 4.2 RULE: Do NOT use these files

- `html_to_hpp.py` — an HTML→C++ raw-string-literal converter. Not used in this project.
- `HTML_Pages.hpp` — a generated header file. Not used in this project.

### 4.3 Key files

| File                                                 | Purpose                                                 |
| ---------------------------------------------------- | ------------------------------------------------------- |
| `Sources/AddOnResources/RFIX/HTML/Interface_ru.html` | Main HTML interface (vanilla JS, no separate CSS files) |
| `Sources/AddOn/dialogs/BrowserPalette.cpp`           | Loads the HTML file; registers the `ACAPI` JS object    |
| `Sources/AddOn/dialogs/BrowserPalette.hpp`           | Declarations for the above                              |

### 4.4 JS functions exposed on the `ACAPI` object

| Function                                   | Purpose                                                       |
| ------------------------------------------ | ------------------------------------------------------------- |
| `ACAPI.GetPropertyDefinitions()`           | Returns an array of properties from `PROPERTYCACHE()`         |
| `ACAPI.GetPropertyDescription(name)`       | Returns a property's description from the cache               |
| `ACAPI.GetPropertyValue(name)`             | Returns a property's value for the currently selected element |
| `ACAPI.ParsePropertyDescription(desc)`     | Parses a description string (Renum/Sync/Spec)                 |
| `ACAPI.SetPropertyDescription(name, desc)` | Writes a property description                                 |

---

## 5. C++ Code Navigation Rules (MANDATORY ORDER)

When you need to find a function definition, class, symbol, or architectural context in C++, follow this exact priority order. Do not skip a step.

### Step 0 — OpenViking (per SOUL.md, always first)

Check OpenViking for prior entries about this symbol, file, or area of the codebase before doing anything else. If a matching entry exists, apply it and state that you did. This does not replace Steps 1–2 below — it only tells you whether the answer, or a shortcut to it, is already known.

### Step 1 — Clangd MCP (always try first among the live-lookup tools)

Use `clangd-mcp` tools (`workspace_symbol_search`, `find_definition`, `find_references`) to find exact C++ symbols. This gives exact AST-based locations with no noise.

### Step 2 — LightRAG (fallback, only if Step 1 fails or more context is needed)

If Clangd cannot locate the symbol, OR the request needs higher-level architectural context, relationships, or conceptual understanding:

- Call the **LightRAG** skill BEFORE writing your response.
- **query:** a concise English description of the function/class name, module/file, or concept (e.g. threading, memory, system architecture).
- **mode:** `"hybrid"` unless explicitly told otherwise.

### Step 3 — Constraints (apply at all times)

1. If Clangd or LightRAG returned real context, treat it as ground truth.
2. Never invent architecture, function signatures, or APIs that are not confirmed by retrieved context.
3. If neither tool provides enough context, explicitly state what is missing and ask the user for clarification. Do not guess.
4. Never answer a C++ navigation or architecture question without first attempting Clangd, then LightRAG if Clangd was insufficient. This rule has no exceptions.

---

## 6. Code Editing & Indentation Policy

1. Never rewrite an entire file just to fix whitespace or indentation mismatches. Make the minimal edit instead.
2. After editing any `.cpp`, `.hpp`, or `.h` file, immediately run:
   ```bash
   clang-format -i path/to/edited_file.cpp
   ```
   Keep every change `clang-format` produces. Do not manually revert any part of it.
3. If a patch fails to apply due to whitespace mismatches, run `clang-format -i` on the target file first, then re-apply the patch.
4. If `clang-format` reorders `#include` directives, do not revert the order. Placing the paired header (e.g. `#include "TestFunc.hpp"` inside `TestFunc.cpp`) first is standard LLVM/Google C++ style and verifies header self-sufficiency.
5. Never treat a `clang-format` change as a style violation or an error. Its output is the project's ground truth for formatting.

---

## 7. Mandatory Verification Cascade

Before creating a `WIP:` checkpoint commit, or declaring any C++ change complete, you **MUST** run the following 3 steps in this exact order. No step may be skipped, reordered, or bypassed.

### Step 1 — LSP Static Analysis (clangd)

- Inspect every modified `.cpp` and `.hpp` file for diagnostic errors, syntax issues, missing symbols, and type mismatches via clangd.
- **Pass condition:** zero errors and zero unhandled diagnostics.
- If you added new `.cpp`/`.hpp` files, changed `#include` directives, or changed CMake targets, regenerate the compilation database immediately:
  ```bash
  python Tools\BuildAddOn.py -c config.json -v 25 --lsp
  ```

### Step 2 — Full Add-On Compilation

- Compile for the target ArchiCAD version (default `25`, or the version specified by the task):
  ```bash
  python Tools\BuildAddOn.py -c config.json -v 25
  ```
- **Pass condition:** process exit code is `0`.
- `/WX` (warnings-as-errors) is enabled: zero warnings and zero errors are allowed.
- If the build fails, **stop immediately**. Do not attempt to run the add-on on a broken build.

### Step 3 — Runtime / End-to-End Test

- For any change that touches runtime execution logic, run:
  ```powershell
  "D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1"
  ```
  This script builds the add-on and launches ArchiCAD with the test project file.
- **Pass conditions:**
  1. ArchiCAD boots and loads the add-on with no crashes and no memory faults.
  2. No `DBASSERT` failures and no unhandled exceptions appear in the ArchiCAD Report Window (`DBprnt` output).
- Never mark a feature or fix complete without verifying its actual runtime behavior in ArchiCAD.

### Failure & Rollback Protocol

If any step in the cascade fails:

1. **Halt immediately.** Do not proceed to the next step.
2. Identify and fix the root cause locally.
3. Restart the cascade from Step 1.
4. **Never** run `git commit`, stage files, or declare a sub-task complete while any verification step is failing or has been skipped.

### Build script reference (`BuildAddOn.py`)

| Flag           | Effect                                                                                        |
| -------------- | --------------------------------------------------------------------------------------------- |
| `-v <version>` | Target AC version, e.g. `25`, `27`; multiple versions space-separated                         |
| (no flag)      | Debug build, single version, no language selection                                            |
| `--release`    | RelWithDebInfo build for all languages in `config.json` (or use `-l <LANG>` for one language) |
| `--package`    | Packages the build into `.apx`/`.bundle` under `Build/Package`                                |
| `-d <path>`    | Use a local DevKit instead of auto-downloading (requires `-v` to be a single version)         |

DevKit is auto-downloaded to `Build/DevKit/APIDevKit-<version>` unless `-d` is given.

### AC version → C++ standard / toolset (auto-detected from `ACAPinc.h` via `DetectACVersion` in `CMakeCommon.cmake`)

| AC version | C++ standard | MSVC toolset       |
| ---------- | ------------ | ------------------ |
| < 27       | C++14        | v140 / v141 / v142 |
| < 29       | C++17        | v142               |
| ≥ 29       | C++20        | v143               |

- `/WX` (warnings-as-errors) is ON. Many `/wd####` warnings are intentionally suppressed — do not re-enable any without a documented reason.
- Precompiled header: `AddOn.hpp` via `target_precompile_headers`.

### Compiler flags

- **Windows:** `/W3 /WX /Zc:wchar_t- /EHsc /bigobj /wd4499 /wd5208 /wd4996 /wd4003` (plus additional suppressed warnings)
- **macOS:** `-Wall -Wextra -Werror -fvisibility=hidden` (plus a long suppression list)

---

## 8. LSP (clangd) Configuration — Different From the Normal Build

The normal build uses the Visual Studio generator (multi-config), which does **not** produce a `compile_commands.json`.

For LSP, a separate Ninja + `clang-cl` configuration is used instead, because MSVC's `cl.exe` cannot extract system includes for clangd.

```bash
python Tools\BuildAddOn.py -c config.json -v <version> --lsp
```

This must be run from a **Developer Command Prompt**. It generates `Build/LspCompileCommands/<version>/compile_commands.json` and copies it to the repo root.

**Do NOT commit:** `compile_commands.json`, `Build/LspCompileCommands/`, `Build/DevKit/` — these are machine-specific artefacts.

**Regenerate this after:**

- Changing the target AC version
- Editing `CMakeCommon.cmake`
- Adding new `.cpp` files to `Sources/AddOn`

### Related script

`D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1` — builds the add-on and launches ArchiCAD with the test project file. (Also referenced in Section 7, Step 3.)

---

## 9. ArchiCAD Version Handling

- This project has **no single default AC version**. It supports AC 22–29, with different C++ standards per version (Section 7) and two separate CI matrices (23–24 and 25–29).
- Do not assume AC 25 is the target for a task just because it appears as the example version in build commands throughout this file.
- Before writing any version-sensitive code (anything using a C++17/20 feature, or an ACAPI symbol that differs across versions), determine which AC version(s) the current task targets. Sources, in order of preference:
  1. The task/issue description
  2. The relevant `Test_file/test_<version>.pln`
  3. Ask the user directly
     Do not guess silently.
- If behavior must differ across supported versions, guard it explicitly (e.g. with `#if` on the detected version). Do not write code that happens to work only on the version you tested against.

---

## 10. ArchiCAD API (ACAPI) Usage — Mandatory RAG Verification

- **First, check OpenViking** for a prior entry on this exact `ACAPI_*` call. If one exists, apply it and say so — but still do the LightRAG check below if the entry is incomplete or the AC version differs.
- Before writing or modifying any `ACAPI_*` call, search the ArchiCAD SDK knowledge base (functions, types, examples, past decisions) via the LightRAG skill. This is **mandatory**, not a fallback — unlike Section 5's Clangd-first order (which is for navigating this project's _own_ symbols), calls into the Archicad SDK always go through LightRAG first, regardless of whether Clangd can already resolve the symbol:
  - Clangd confirms the function exists and its declared signature.
  - Only LightRAG confirms the call is correct and valid for the target AC version (Section 9 — the ACAPI surface is versioned).
- Query convention: same as Section 5 — concise English query naming the exact function/struct/pattern, `mode: "hybrid"` unless told otherwise.
- No relevant LightRAG coverage → say so and ask the user. Do not guess from memory.
- **Pre-verified landmine:** `ACAPI_Element_GetMemo` requires `BNZeroMemory(&memo, sizeof(memo))` first.

---

## 11. Git & Checkpoint Discipline

SOUL.md's "Autonomy & Checkpoint Safety Net" defines _when_ to commit, roll back, and escalate, and lists the hard stops that apply everywhere. The rules below are this repo's specific mechanics for satisfying that: what a checkpoint means here, and repo-specific constraints on top of the general ones.

1. **Check before editing:** run `git status` before making any change. If there are uncommitted changes unrelated to the current task, stop and flag them. Do not build on top of them, commit them, or discard them silently.
2. **Checkpoint commits are mandatory:** after every verified sub-task (build succeeds, no new warnings/errors for the AC version(s) in scope), commit with a `WIP:` prefix. This makes autonomous work safe — a mistake only rolls back to the last checkpoint, not to the start of the task. A `WIP:` commit is a valid rollback point even mid-task; it does not need to be the task's final commit.
3. **The branch tip must always build:** once the task is done, squash the `WIP:` commits into one clean commit. Never leave a non-building commit as the tip of the branch.
4. **Never destroy the rollback trail:** do not run `git clean -fdx`, do not delete untracked files, do not delete files outside the current task's scope.
5. **Never rewrite shared history:** do not run `git commit --amend`, `git rebase -i`, or `git push --force` on any branch that has already been reviewed or shared.
6. **Never stage blindly:** do not run `git add .` or `git add -A` without first reading `git status` and `git diff`. Build/LSP artefacts (`compile_commands.json`, `Build/`) must never be staged.
7. Commit messages must reference the relevant task/issue. (This project has no phase structure, unlike other repos.)

---

## 12. Session State File — `IDEA.md`

This repo has no fixed implementation phases. Work arrives as discrete tasks/bugs/features across many separate agent sessions, with no shared memory between sessions.

For any task expected to span more than one session, maintain `IDEA.md` at the repo root (create it if missing — same file name/role used in other projects, but task-keyed here instead of phase-keyed):

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

**Rule:** Update this file before and after every significant step. If a step is not recorded here, treat it as not done when resuming a session. This file is the checkpoint unit referenced by the Git checkpoint rule in Section 11.

---

## 13. Session Bootstrap Checklist

Run through this checklist at the **start of every new session**, before writing or editing any code. This consolidates checks that are individually mandated elsewhere in this file (Sections 9, 11, 12) into one ordered sequence.

1. **Check OpenViking** (per SOUL.md) for prior entries relevant to the task at hand, in addition to `IDEA.md` below — they serve different purposes: `IDEA.md` tracks _this specific task's_ progress, OpenViking holds lessons from _past_ tasks that may apply here.
2. **Read `IDEA.md`** at the repo root, if it exists. If it shows `Status: IN_PROGRESS` or `WAITING_FOR_TEST`, resume from its "Immediate Next Step" — do not restart the task from scratch.
3. **Run `git status`.** If there are uncommitted changes unrelated to the current task, stop and flag them to the user (Section 11, Rule 1).
4. **Determine the target ArchiCAD version(s)** for the task, per Section 9. Check the task/issue description first, then `IDEA.md`, then ask the user. Never default to AC 25 silently.
5. **Confirm required tools are reachable**: `clangd-mcp` and the LightRAG skill. If either is unavailable, follow Section 18 before proceeding.
6. Only after steps 1–5 are complete, begin the task.

---

## 14. Key Source Files & Responsibilities (`Sources/AddOn/`)

| File                         | LOC   | Role                                                                                                                                                                                                                                                                                                                                                                          |
| ---------------------------- | ----- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `SomeStuff_Main.cpp/hpp`     | 528   | Entry point: `CheckEnvironment`, `RegisterInterface`, `Initialize`, `FreeData`; menu dispatcher (`MenuCommandHandler`); observers (`ElementEventHandlerProc`, `ProjectEventHandlerProc`, `ReservationChangeHandler`, `SelectionChangeHandlerProc`); `Do_ElementMonitor`; menu state sync                                                                                      |
| `Helpers.cpp/hpp`            | 9,877 | **Core engine.** `ParamHelpers` namespace: property read/write (`ElementsRead`/`ElementsWrite`/`WriteProperty`), `ParamValue`/`ParamDictElement`/`ParamDictValue` (internal property currency), format-string parsing (`FormatStringFunc`), element selection/filtering, coordinate/angle helpers, QR-code generation, classification, attribute cache, GDL parameter parsing |
| `Propertycache.cpp/hpp`      | 1,102 | Singleton `PropertyCache`: caches property definitions, classifications, attributes, project info, geo-location, MEP (AC29+), format strings, composite layer data; `Update()` refreshes all                                                                                                                                                                                  |
| `Sync.cpp/hpp`               | 2,685 | `SyncAndMonAll`, `SyncSelected`, `MonAll`/`MonByType` (reactive observer attach), `SyncByType` per element type; throttling via a 500 ms dedup cache                                                                                                                                                                                                                          |
| `Roombook.cpp`               | 5,656 | Finish schedule: zone-based collection of walls/columns/slabs/doors/windows → composite layers → material lookup → favourite matching → element create/update                                                                                                                                                                                                                 |
| `Spec.cpp`                   | 2,316 | Specification rules from property descriptions (`Spec_rule{…}`); classification filtering; writes list data to properties                                                                                                                                                                                                                                                     |
| `Summ.cpp`                   | 504   | Sums property values across elements into a target property or project info field                                                                                                                                                                                                                                                                                             |
| `ReNum.cpp`                  | 933   | Renumbering by property criteria (alternative to the ID Manager)                                                                                                                                                                                                                                                                                                              |
| `Revision.cpp`               | 1,044 | Revision markers / change clouds                                                                                                                                                                                                                                                                                                                                              |
| `Dimensions.cpp`             | 368   | Dimension rounding, formula writing (e.g. `6×100=600`)                                                                                                                                                                                                                                                                                                                        |
| `ClassificationFunction.cpp` | 254   | Auto-classification by property values                                                                                                                                                                                                                                                                                                                                        |
| `ResetProperty.cpp`          | 390   | Resets properties to default / clears them                                                                                                                                                                                                                                                                                                                                    |
| `AutomateFunction.cpp`       | 987   | Profile-by-line, drawing alignment                                                                                                                                                                                                                                                                                                                                            |
| `MEPv1.cpp`                  | 1,202 | MEP system/group/description output (AC28+)                                                                                                                                                                                                                                                                                                                                   |
| `DG4rule.cpp`                | 163   | DG rules for dialogs                                                                                                                                                                                                                                                                                                                                                          |
| `TestFunc.cpp`               | 1,495 | Debug helpers, active only under `TESTING`                                                                                                                                                                                                                                                                                                                                    |
| `qrcodegen.cpp`              | 829   | QR code generation (used by the finish schedule)                                                                                                                                                                                                                                                                                                                              |
| `Spec_libpart.cpp`           | 553   | Spec string parsing                                                                                                                                                                                                                                                                                                                                                           |
| `SyncSettings.cpp/hpp`       | 101   | Settings serialization to Add-On Preferences (`ACAPI_Get/SetPreferences`)                                                                                                                                                                                                                                                                                                     |
| `CommonFunction.cpp`         | 2,543 | Element selection helpers, story handling, QR utilities, debug print (`DBprnt`)                                                                                                                                                                                                                                                                                               |

---

## 15. Prohibited Actions

- ❌ Do not use C++17/20 syntax in code compiled for AC < 27 / AC < 29 — this is a hard compiler error.
- ❌ Do not edit files under `Sources/AddOnResources/`.
- ❌ Do not commit `compile_commands.json`, `Build/LspCompileCommands/`, or `Build/DevKit/`.
- ❌ Do not re-enable a suppressed `/wd####` warning without a documented reason.
- ❌ Do not call `ACAPI_Element_GetMemo` without first calling `BNZeroMemory(&memo, sizeof(memo))`.
- ❌ Do not query LightRAG any way other than `bash_tool` with `curl`. Read the LightRAG skill for details.
- ❌ Do not assume AC 25 is this repo's target version — see Section 9, "ArchiCAD Version Handling."

---

## 16. Debug Workflow

- A Debug build copies `Test_file/test_<version>.pln` into the build directory.
- The Visual Studio debugger launches ArchiCAD with that `.pln` file (`VS_DEBUGGER_COMMAND` is set in `CMakeCommon.cmake`).
- `DBprnt` / `msg_rep` output appears in the ArchiCAD Report Window, only in Debug builds or when `TESTING` is defined.

---

## 17. Pre-PR Checklist

Before opening a pull request, confirm all of the following:

- [ ] Build passes locally for the target AC version(s), e.g. `-v 25 27 29`
- [ ] No C++17/20 syntax exists in code paths compiled for AC < 27 / AC < 29
- [ ] Any new `.cpp` files are picked up by CMake's `file(GLOB …)` automatically, or added manually if not
- [ ] `--lsp` was run once after adding new files, so `clangd` can see them
- [ ] Both CI workflows (23–24 and 25–29) run on push
