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
python python_scripts/roombook_setup.py --create-classes --port 19723
```

GUI checks the selected instance by default; checking all instances is an
explicit option. Refresh and Run are disabled while a task is active.
Worker errors are displayed separately from an empty instance list.

### Properties tab (GUI)

For each required `some_stuff_fin_*` marker the tab shows a searchable
dropdown (filter by first letters, with a contains-fallback) listing the
project's UserDefined properties. "Apply selected" sets the chosen property's
description to the marker via the add-on command `SetPropertyDescription`
(issue #304). An existing different description is skipped unless the
"Overwrite existing description" option is enabled. Property listing and
application run in a background thread; every applied change is verified by a
read-back and can be undone in Archicad (in-memory only until the project is
saved).

CLI exit codes: `0` — all checks passed; `1` — no Archicad found;
`2` — at least one check failed; `3` — checks incomplete (no explicit failures);
`4` — class creation failed (system not readable, no confirmation, or creation errors).

### Creating finish classes (--create-classes)

Interactive CLI mode backed by the add-on command `CreateClassificationItem`
(namespace `SomeStuffCommand`, issue #303). The stock AC25/AC30 JSON API has no
classification-creation commands, so creation goes through the add-on:

1. Lists existing classification systems (`GetAllClassificationSystems`).
2. Reads the tree of the chosen system (`GetAllClassificationsInSystem`,
   nodes carry item GUIDs) and lets you pick the parent branch (`0` = root).
3. Compares `FINISH_CLASSES` (13 items from `RoombookConstants.hpp`:
   id = marker = description, name = human-readable Russian) against markers
   found in descriptions and creates **only the missing ones**; existing
   classes are never touched, a repeated run is a no-op.
4. Re-runs the read-only classification check and prints the result.

The add-on finds finish classes by searching `some_stuff_fin_*` in item
*descriptions* (`Class_FindFinClass`), so `description` carries the marker.

### Notes

- **Important**: The addon's own command to list property definitions
  (`GetPropertyDefinitions`) was removed. The script uses standard Archicad
  commands instead.

### What it does NOT do

The read-only check mode never creates or modifies definitions.
The only modifying path is the explicit `--create-classes` mode (above): it
creates missing classification items and nothing else. It does not create
properties, favorites, layer combinations, or classification systems — the
system itself must already exist (owner decision, issue #303).

### Output

The script prints:

- Connection status
- List of missing property markers
- List of missing favorites (if detectable)
