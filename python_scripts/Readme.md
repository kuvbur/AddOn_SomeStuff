# Python Scripts for SomeStuff Add-on

## Overview

This directory contains Python utilities for interacting with Archicad via its JSON API.

## roombook_setup.py

### Purpose

Read-only pre-flight check: is the current Archicad project ready for the
Roombook finishing function? It verifies:

1. **Connectivity** — auto-detects running Archicad instances (ports 19723–19743).
2. **SomeStuff add-on loaded** — calls the add-on `Health` command
   (namespace `SomeStuffCommand`); reports code 4010 if the add-on is absent.
3. **Favorites** — `smstf wall/floor/ceil/window` (required) and
   `smstf reveal side/up` (optional, fallback `smstf wall`).
   The stock AC25 JSON API cannot enumerate favorites — the script reports
   this honestly until an add-on command exists.
4. **Zone property markers** — searches `some_stuff_fin_*` markers in
   property *descriptions* (add-on logic: contains marker, not marker + `_`),
   via `GetAllPropertyNames` → `GetPropertyIds` → `GetDetailsOfProperties`.
5. **Classification classes** — `some_stuff_fin_*` items across all
   classification systems, via `GetAllClassificationSystems` +
   `GetAllClassificationsInSystem` (recursive tree with `children`).
6. **Layer combination** — `smstf_fin` suffix check requires an add-on
   command (stock API cannot read layer combinations).

### Requirements

- Python 3.7+ with tkinter (standard library only)
- Archicad running with JSON API enabled (port 19723+)
- No external libraries (uses only standard library)
- Note: Cyrillic in `curl` from the console breaks UTF-8 (error 2001) — the
  script uses `urllib` with explicit UTF-8 encoding, which is safe.

### Usage

```bash
python python_scripts/roombook_setup.py                    # GUI, select a project and port
python python_scripts/roombook_setup.py --cli              # all running instances
python python_scripts/roombook_setup.py --cli --port 19723 # one port
```

GUI checks the selected instance by default; checking all instances is an
explicit option. Refresh and Run are disabled while a task is active.
Worker errors are displayed separately from an empty instance list.

CLI exit codes: `0` — all checks passed; `1` — no Archicad found;
`2` — at least one check failed; `3` — checks incomplete (no explicit failures).

### Notes

- **Important**: The addon's own command to list property definitions
  (`GetPropertyDefinitions`) was removed. The script uses standard Archicad
  commands instead.

### What it does NOT do

The script is read-only: it never creates or modifies definitions.
Creating missing properties/classifications/favorites is a separate command
(pending owner decision) and will require an add-on command for favorites
and layer combinations.

### Output

The script prints:

- Connection status
- List of missing property markers
- List of missing favorites (if detectable)
