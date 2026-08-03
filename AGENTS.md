# AGENTS.md — AddOn_SomeStuff

Mandatory repository rules for AI agents working on this ArchiCAD C++ Add-On.

**SOUL.md defines global agent behavior. This file defines repository-specific rules.**

---

## 1. Project

- Project: `SomeStuff` — ArchiCAD C++ Add-On.
- ArchiCAD versions: **22–29**.
- Platforms: Windows, macOS.
- SDK: ArchiCAD DevKit.
- Main C++ source directory:

```text
Sources/AddOn/
```

### Critical source-location rule

**For C++ source files, never search outside `Sources/AddOn/`.**

Do not use recursive filesystem searches from the repository root or disk root to find C++ source files.

Do not search outside `Sources/AddOn/` unless the task explicitly requires:

- build configuration;
- resources;
- tests;
- documentation;
- scripts;
- or another specifically named file.

Never edit:

```text
Sources/AddOnResources/
```

unless the task explicitly requires a resource change.

---

## 2. Repository Structure

```text
Sources/AddOn/                  C++ source code
Sources/AddOn/api_headers/      ArchiCAD API headers
Sources/AddOn/json_commands/    JSON command handlers
Sources/AddOn/third_party/      Embedded third-party libraries

Sources/AddOnResources/         Resources
Sources/MacDarkModeIcon/        macOS assets

Tools/BuildAddOn.py             Build/configuration script
Tools/CMakeCommon.cmake         CMake configuration
Tools/CompileResources.py       Resource compiler

Test_file/                      ArchiCAD test PLN files
CMakeLists.txt                  Main CMake entry
config.json                     Build configuration
IDEA.md                         Multi-session task state
```

---

## 3. Mandatory Workflow

For a non-trivial task use this order:

```text
OpenViking
→ Inspect relevant files
→ Determine AC version(s)
→ Verify APIs / architecture
→ Diagnose root cause
→ Make minimal change
→ clang-format
→ Validate
→ Review diff
→ Checkpoint
→ OpenViking memory
→ Answer
```

Do not skip verification because a solution looks obvious.

Do not fix symptoms before understanding the root cause.

Prefer:

- minimal changes;
- existing project patterns;
- deterministic behavior;
- simple solutions;
- small diffs.

Do not perform unrelated refactoring.

---

## 4. OpenViking

**OpenViking is mandatory and has no fallback.**

Before investigating:

- an error;
- a non-trivial bug;
- an unfamiliar symbol;
- an `ACAPI_*` call;
- an architectural decision;

search OpenViking first.

If OpenViking is unavailable, follow the mandatory OpenViking rules from `SOUL.md`. Do not silently treat the search as completed.

After resolving a non-trivial problem, save the useful conclusion to OpenViking according to `SOUL.md`.

---

## 5. C++ Navigation

For C++ symbol navigation:

1. Search OpenViking first.
2. Use **Clangd MCP** for exact source locations, definitions and references.
3. Use LightRAG when additional SDK or architectural context is needed.

For C++ source files, search only inside:

```text
Sources/AddOn/
```

Do not search the entire disk or repository for source copies.

If symbol information cannot be established, do not guess.

---

## 6. ArchiCAD API — Mandatory LightRAG

**LightRAG is the authoritative SDK knowledge source for this project.**

It contains the project's indexed:

- ArchiCAD SDK;
- SDK documentation;
- SDK examples;
- relevant API information.

### Every `ACAPI_*` call

Always follow:

```text
OpenViking
→ LightRAG
→ project source / headers / call sites when needed
```

LightRAG is **mandatory and is not a fallback**.

Do not rely on:

- model memory;
- generic C++ knowledge;
- another ArchiCAD version;
- Revit;
- AutoCAD;
- another IFC library.

If LightRAG does not provide enough information, write:

```text
not verified
```

Do not invent missing API behavior.

### Verify when relevant

Before using or modifying an SDK API, verify:

- signature;
- parameters;
- return value;
- ownership;
- lifetime;
- pointer/reference validity;
- memo memory;
- iterator validity;
- transaction/undo requirements;
- redraw/notification requirements;
- version-specific behavior.

Pay particular attention to:

```text
API_Element
API_ElementMemo
ACAPI_Element_GetMemo
GetPtr
GS::* containers
GS::* iterators
SDK-managed memory
```

### Critical landmine

Before calling:

```cpp
ACAPI_Element_GetMemo(...)
```

zero-initialize the memo:

```cpp
BNZeroMemory(&memo, sizeof(memo));
```

Do not remove this initialization without verified SDK/project evidence.

---

## 7. ArchiCAD Versions

This project supports:

```text
AC 22–29
```

There is **no default ArchiCAD version**.

Before version-sensitive changes, determine the target version(s) from:

1. task requirements;
2. build configuration;
3. existing source conditionals;
4. relevant test files;
5. user clarification if still necessary.

Never silently assume AC25.

For version-specific behavior:

- inspect existing `#if` branches;
- follow existing project patterns;
- verify the actual build configuration.

Do not introduce language features unsupported by the target compiler.

---

## 8. C++ Editing

Generate compilable C++ only. Never pseudocode.

Preserve:

- architecture;
- naming;
- formatting;
- APIs;
- project conventions.

Do not introduce libraries unless required.

Do not change public APIs unless necessary.

### No unrequested refactoring

Do not refactor code merely because it:

- looks old;
- is duplicated;
- could be shorter;
- could use newer C++ style.

Modify existing code only when it is:

- directly related to the task;
- the confirmed root cause;
- a confirmed correctness/safety problem;
- necessary for the requested implementation.

---

## 9. Formatting

After **every modification** of:

```text
.cpp
.hpp
.h
```

immediately run:

```bash
clang-format -i path/to/file.cpp
```

Keep the formatter's result.

Do not manually fight `clang-format`.

Do not manually restore indentation or include ordering after formatting.

Do not rewrite an entire file merely to format it.

---

## 10. Code Comments

All new or changed C++ comments must be **in Russian**.

Comment:

- non-obvious logic;
- purpose;
- important assumptions;
- algorithmic decisions;
- important ownership/lifetime constraints.

Do not comment obvious syntax or trivial lines.

---

## 11. Validation

Use the validation level appropriate to the task.

### LSP

For C++ changes, inspect modified files with Clangd.

If the compilation database must be regenerated:

```bash
python Tools\BuildAddOn.py -c config.json -v <version> --lsp
```

Do not commit:

```text
compile_commands.json
Build/LspCompileCommands/
Build/DevKit/
```

### Build

Build the actual target version(s):

```bash
python Tools\BuildAddOn.py -c config.json -v <version>
```

Only claim `compiled` after an actual successful build.

### Runtime

For changes affecting runtime behavior, execute the relevant test.

Standard ArchiCAD test launcher:

```powershell
"D:\SomeStuff_addon\Tools\restart_archicad_for_test.ps1"
```

Only claim `tested` when the changed behavior was actually executed and observed.

Always distinguish:

```text
Verified: ...
Compiled: yes / not performed
Tested: yes / not performed
```

The exact definitions of these states are defined by `SOUL.md`.

---

## 12. Tests

Test files:

```text
Sources/AddOn/TestFunc.cpp
Sources/AddOn/TestFunc.hpp
```

`TestFunc` is active under `TESTING`.

### Testing-task rule

When the task is specifically to write or fix tests:

**Production code is READ-ONLY.**

Production code may be inspected but not modified.

If a test exposes a production bug:

1. stop;
2. report the exact bug;
3. identify file and function;
4. explain why the test exposes it;
5. treat the production fix as a separate task.

Do not modify:

```text
Test_file/*.pln
```

as part of a normal code-testing task.

---

## 13. Git Safety

Before modifying files:

```bash
git status
```

**Never overwrite or discard uncommitted user changes.**

If unrelated uncommitted changes are present, do not modify or reset them.

Never use:

```text
git clean -fdx
git commit --amend
git rebase -i
git push --force
```

Never use:

```text
git add .
git add -A
```

without first inspecting `git status` and `git diff`.

Never stage:

```text
Build/
compile_commands.json
Build/LspCompileCommands/
Build/DevKit/
```

Do not delete files outside the task scope.

Do not overwrite or discard checkpoint/commit history.

---

## 14. Checkpoints

Follow the checkpoint policy from `SOUL.md`.

For this repository, create a checkpoint only after a completed tactical step when:

- the intended change is complete;
- the relevant build succeeds;
- no new compile/LSP errors were introduced;
- the working tree contains only intended changes.

Never create a checkpoint for a known broken state.

Do not start another change on top of an unverified broken state.

---

## 15. IDEA.md

Use `IDEA.md` for tasks spanning multiple sessions.

Keep it short and operational:

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

Update it when a significant step is completed or the resume point changes.

Do not turn `IDEA.md` into a transcript of every tool call.

---

## 16. Important Project Areas

Use these only as navigation hints. Inspect the actual code before making decisions.

| File                         | Main responsibility                             |
| ---------------------------- | ----------------------------------------------- |
| `SomeStuff_Main.cpp/hpp`     | Add-On entry point, interface, menu, observers  |
| `Helpers.cpp/hpp`            | Core helpers, properties, selection, parameters |
| `Propertycache.cpp/hpp`      | Property/classification/attribute/project cache |
| `Sync.cpp/hpp`               | Property synchronization and monitoring         |
| `Roombook.cpp`               | Finish schedule                                 |
| `Spec.cpp`                   | Specification rules                             |
| `Summ.cpp`                   | Property summation                              |
| `ReNum.cpp`                  | Renumbering                                     |
| `Revision.cpp`               | Revision markers                                |
| `Dimensions.cpp`             | Dimensions                                      |
| `ClassificationFunction.cpp` | Auto-classification                             |
| `ResetProperty.cpp`          | Property reset                                  |
| `AutomateFunction.cpp`       | Automation/alignment                            |
| `MEPv1.cpp`                  | MEP functionality                               |
| `CommonFunction.cpp`         | Common element/utility functions                |

The repository may change. Do not blindly trust this table.

---

## 17. Root-Cause Debugging

When debugging:

1. Search OpenViking.
2. Inspect the relevant source and call sites.
3. For `ACAPI_*`, query LightRAG.
4. Determine the root cause.
5. Make the smallest correct fix.
6. Run `clang-format` on changed C++ files.
7. Validate.
8. Review the diff.
9. Save important conclusions to OpenViking.

Classify the root cause when useful:

```text
SDK limitation
API/IFC limitation
implementation bug
wrong assumption
architecture problem
input-data problem
call-site problem
```

Do not add a workaround without understanding what it bypasses.

---

## 18. Diff Review

Before declaring a code change complete:

```bash
git diff
git status
```

Check:

- only intended files changed;
- no accidental formatting damage;
- no unrelated refactoring;
- no debug leftovers;
- no generated files;
- no user changes were overwritten;
- formatter changes are intentional.

For code-review findings use:

```text
[code] -> [problem] -> [minimal fix]
```

When useful, add:

```text
[evidence]
```

If no issue exists:

```text
No issues found in inspected scope
```

---

## 19. BrowserPalette / HTML

BrowserPalette loads HTML directly from disk.

Main interface:

```text
Sources/AddOnResources/RFIX/HTML/Interface_ru.html
```

C++ implementation:

```text
Sources/AddOn/dialogs/BrowserPalette.cpp
Sources/AddOn/dialogs/BrowserPalette.hpp
```

Do not use:

```text
html_to_hpp.py
HTML_Pages.hpp
```

They are not used by this project.

Do not edit `Sources/AddOnResources/` unless the task explicitly requires a resource change.

---

## 20. Pre-PR Checklist

Before a PR/review handoff:

- [ ] Only intended files changed.
- [ ] No user changes were overwritten or discarded.
- [ ] Target AC version(s) are known.
- [ ] Every `ACAPI_*` usage was checked through LightRAG.
- [ ] Modified C++ files were formatted with `clang-format`.
- [ ] LSP checked where applicable.
- [ ] Build performed for relevant version(s).
- [ ] Runtime test performed where applicable.
- [ ] `verified / compiled / tested` reported correctly.
- [ ] No generated build files are staged.
- [ ] No unrelated refactoring was introduced.
- [ ] OpenViking memory updated when required by `SOUL.md`.

---

## 21. Golden Rules for Weak LLMs

When uncertain, follow this exact order:

```text
1. Do not guess.
2. Search OpenViking.
3. Find the actual source in Sources/AddOn/.
4. For ACAPI_* → query LightRAG.
5. Check the actual ArchiCAD version.
6. Inspect call sites and surrounding code.
7. Make the smallest correct fix.
8. clang-format every changed C++ file.
9. Build/test when required.
10. Review git diff.
11. Never destroy user changes.
```

**Verified project information and LightRAG SDK information take priority over model memory.**
