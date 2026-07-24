# AGENTS.md — AddOn_SomeStuff (ArchiCAD C++ Add-On)

## What this project is

**SomeStuff** — ArchiCAD Add-On (C++, ACAPI SDK, GPL-3.0). Automates: GDL ↔ Property sync, flexible numbering, structure layer composition export, value summation, coordinates/angles, dimension tools, IFC property copy, project info, morph length, auto-classification, composite decomposition, MEP data, layout tracking, finish schedule + QR codes.

Repo: https://github.com/kuvbur/AddOn_SomeStuff  
Supported: AC 22–29, Windows / macOS. Bilingual UI (RUS/INT, auto-detected).

---

## Repository structure

```
Sources/AddOn/                # .cpp / .h sources (core + modules)  ← ALL SOURCE FILES ARE HERE
Sources/AddOnResources/       # Resources (RFIX, R<LANG>/*.grc, images)
  RFIX/Images/*.svg           # Menu icons
  RINT/AddOn.grc              # Generated from AddOn.grc.in (do NOT edit manually)
  RINT/*.grc                  # String tables (generated)
  RFIX.win/*.rc2 / RFIX.mac/*.plist
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

> **⚠️ IMPORTANT**: All C++ source files (`.cpp` / `.h`) are located in `D:\SomeStuff_addon\Sources\AddOn\`.  
> Always search/read there — NOT in the repo root or other folders.

## C++ Code Navigation & Context Rules

### Primary Navigation Flow

When looking for function definitions, classes, symbols, or architectural context in C++, you MUST follow this strict priority order:

1. **First Choice: Clangd MCP**
   Always attempt to find exact C++ symbols, definitions, and declarations using `clangd-mcp` tools first (`workspace_symbol_search`, `find_definition`, `find_references`). It provides exact AST-based locations without context noise.

2. **Second Choice (Fallback / Context Search): LightRAG MCP**
   If `clangd-mcp` fails to locate the symbol, or if the request requires higher-level architectural context, relationships, or conceptual understanding:
   - Call the **LightRAG MCP** tool **BEFORE** forming your response.
   - **query**: Formulate a concise query in English describing the function/class names, modules/files, or key concepts (e.g., threading, memory, system architecture).
   - **mode**: `"hybrid"` (unless explicitly instructed otherwise).

3. **Strict Constraints & Execution Guidelines**
   - **Base responses on retrieved context:** If retrieved context or code exists from Clangd/LightRAG, use it as the ground truth.
   - **No Hallucinations:** Do NOT invent architecture, function signatures, or APIs if they exist in the codebase/context.
   - **Acknowledge Gaps:** If neither tool yields sufficient context, explicitly state what is missing and ask for clarification.
   - **Zero Bypass Rule:** NEVER answer C++ code navigation or architectural questions without attempting symbol search via Clangd MCP first, followed by LightRAG MCP if needed. When in doubt, search first.

---

## How to build (normal build)

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

## Compiler flags & C++ versions (from CMakeCommon.cmake)

| AC version | C++ std | MSVC toolset       |
| ---------- | ------- | ------------------ |
| < 27       | 14      | v140 / v141 / v142 |
| < 29       | 17      | v142 / v143        |
| ≥ 29       | 20      | v143               |

`/W3 /WX /Zc:wchar_t- /EHsc /bigobj /wd4499 /wd5208 /wd4996 /wd4003` + many suppressed warnings.  
macOS: `-Wall -Wextra -Werror -fvisibility=hidden` + long suppression list.

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
