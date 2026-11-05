---
name: archicad-zone-finish
description: Use when updating a named or numbered zone finish.
version: 0.3.0
---

# Create or update zone finishes using SomeStuff

## When to use

Use for an explicit user request to create or update finishes in one or more zones identified by exact number, name or GUID. This changes the open model. The assistant needs local HTTP access to the intended running Archicad instance and a SomeStuff build that supports explicit `zones`. Read `references/room-finish-json.md` for exact request shapes, response semantics, version limits and the safety boundary. The companion `scripts/find_zones.py` performs **read-only** discovery; it does not create finishes.

## Before changing the model

1. Identify the intended running Archicad instance and project. Query `API.GetProductInfo` and the SomeStuff `Health` command where available. If more than one instance/project could be intended, ask; never choose a port merely because it is 19723. Do not rebuild, restart, close or save a project without the user's permission. A Python 3 stdlib client is optional; use the available HTTP tools if Python is unavailable.
2. Find zones with `API.GetElementsByType`, resolve `Zone_ZoneNumber` and `Zone_ZoneName` using `API.GetPropertyIds`, and read values with `API.GetPropertyValuesOfElements`. Alternatively run the packaged read-only helper; exact invocation is in the reference. Preserve leading zeroes. Match the requested number/name exactly and validate read statuses, counts and order. Check an input GUID's syntax and independently establish it identifies a Zone in the intended project. Missing/unreadable rows are unknown, not non-matches. If zero, incomplete, or several matches exist, stop and ask the user to resolve the ambiguity; do not guess.
3. Verify support for the nonempty `zones` parameter **in the binary actually loaded by that instance**, not just in source, documentation or a successful availability check. Evidence can be an identified loaded build known to contain the feature or previously observed native acceptance for that exact loaded build. Confirm the project and selected zone's name, number and GUID. If build identity or support is unknown, stop: an old command can ignore `zones` and run on every eligible zone. Never probe it with a mutating request or a malformed list.
4. Check finish configuration, available favorites, classifications and property definitions as described in `references/finish-configuration-json.md`. Treat incomplete reads as unknown, not absent. If setup changes are required, explain the proposed favorites, classes and property-description changes and get explicit approval for those changes separately from the request to finish a zone. Never create duplicates, overwrite a favorite, replace a whole property description or change classifications silently; verify every approved change by read-back. Stop if setup is incomplete or unknown and cannot safely be resolved.
5. Confirm the command is available with `API.IsAddOnCommandAvailable`. Collect a read-only pre-run inventory, including zone ownership where available, and the requested property values if comparison is requested. Store the first response and the before/after evidence outside the user's project. Do not unlock, unhide or reserve elements or alter layer combinations to make the request work.

## Execute once

Send `API.ExecuteAddOnCommand` for `SomeStuffCommand.RoomBook` with `addOnCommandParameters.zones` containing exactly the verified GUIDs as a nonempty string array. Never omit the field, send an empty array or use `null`: without a valid explicit route the legacy implementation can process all eligible zones. Zone eligibility is checked before the optional finish layer-combination switch; the command may switch a combination itself. A timeout is not evidence of failure: do not retry automatically.

## Verify and report

Read `result.addOnCommandResponse` and inspect errors, the returned zone GUIDs and element fields. `status: returned` means only that the entry point returned, not that every operation succeeded. In the inspected implementation, the response contains successfully created elements, not a complete inventory of unchanged or updated elements. Independently read back returned GUIDs and ownership, and compare the pre/post inventories for additions, removals and changed ownership. Explain what the chosen inventory does **not** cover; unchanged GUID/ownership does not prove unrelated properties or geometry stayed unchanged. Do not assume every added element appears in the response.

If the user requests areas or property comparison, read the actual properties after execution, distinguish prepared response `area` from native measured areas, and honor `area_available`. Property text can aggregate separate material layers and types; do not sum those rows as a single physical area. Check material mapping and measurement semantics before comparing, and report uncertainty or discrepancies rather than declaring equality from a coincidental number. The response is not the full Archicad report window; disclose if that report cannot be read.

Report the zone, project, actual observed changes, model read-back, errors and validation limits. Do not claim a full geometry check, successful updates, repeatability, other-version support or saved project unless actually verified. See the reference for known AC25 evidence and safe HTTP examples.
