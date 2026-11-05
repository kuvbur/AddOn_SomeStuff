# RoomBook JSON: portable agent reference

## Availability and safety

The request shapes below were exercised on a live Archicad 25 Windows test project. Other versions, languages and builds need independent validation. The current development source in `Sources/AddOn/json_commands/RoomBookCommand.cpp` implements explicit `zones`; at the time this package was prepared, that change was uncommitted. Do **not** assume every released SomeStuff binary contains it. An earlier implementation ignores input parameters and can process every eligible zone. Availability of `SomeStuffCommand.RoomBook` is not proof of parameter support. Establish the identity and compatibility of the loaded module before any RoomBook invocation; stop if uncertain.

The inspected current implementation checks each requested zone's GUID, type and editable/visible/access/workspace/renovation filters before invoking the pipeline. Duplicate GUIDs are removed. The pipeline may apply a finish layer combination. If no explicit target is supplied, it uses the selection and can fall back to all eligible zones. Never intentionally send an omitted, empty, null or malformed `zones` list to test a live model. In AC25 SDK experiments, JSON null can disappear during conversion; native schema enforcement of null was not established by those experiments. Never probe compatibility with a mutating call.

## Read-only lookup

Send UTF-8 JSON as HTTP POST to the selected instance's local JSON port. The actual port and project must be established; do not assume the default belongs to the desired instance. A standard-library helper is bundled:

```text
python scripts/find_zones.py --port PORT --number 123
python scripts/find_zones.py --port PORT --name "Room name"
```

Run from the skill folder, with `PORT` replaced by the verified numeric port. Both commands only read the model. Number matching is textual, so `001` differs from `1`. If any zone value is unreadable, the helper exits without returning a match as complete. Its JSON output contains the matches and a count; the agent must independently identify the project and compatible loaded module.

The protocol used for discovery is:

```json
{"command":"API.GetElementsByType","parameters":{"elementType":"Zone"}}
```

```json
{"command":"API.GetPropertyIds","parameters":{"properties":[{"type":"BuiltIn","nonLocalizedName":"Zone_ZoneNumber"},{"type":"BuiltIn","nonLocalizedName":"Zone_ZoneName"}]}}
```

Use the returned `result.elements` and `result.properties` arrays without double-wrapping them:

```json
{"command":"API.GetPropertyValuesOfElements","parameters":{"elements":[],"properties":[]}}
```

The last example illustrates the parameter names only: replace both empty arrays with the values returned by the preceding calls; an empty request is not a successful zone lookup. Check outer `succeeded`, individual errors, array lengths and `propertyValue.status`. `normal` is required to compare a value. Read both names and numbers; reject a partial inventory rather than silently treating unknown rows as absent.

## Targeted execution — only after compatibility and target checks

This illustrative GUID is a placeholder and **must not be sent unchanged**:

```json
{"command":"API.ExecuteAddOnCommand","parameters":{"addOnCommandId":{"commandNamespace":"SomeStuffCommand","commandName":"RoomBook"},"addOnCommandParameters":{"zones":["11111111-1111-1111-1111-111111111111"]}}}
```

Check availability separately using `API.IsAddOnCommandAvailable` and the same `addOnCommandId`, but never treat availability as evidence that `zones` is supported. Preserve the first response. On a timeout, inspect the model before considering any further operation; the call might have completed.

## Interpreting the result

The outer response may have `succeeded:true` while the nested `result.addOnCommandResponse` contains its own error or `status:"returned"`. The latter states that a void entry point returned; it is not proof of full success. In the inspected code, `rooms` is grouped by zone, with `types`/`elements`; the collector filters for successful **new** writes, so it is not a complete list of updated or unchanged elements. Validate GUID uniqueness, existence and zone linkage independently. Compare a pre-run and post-run inventory and distinguish unreported added elements; preserve errors and unknown ownership as unknown. Checking IDs and ownership alone does not prove isolation of all other attributes.

`area` and `length` are accompanied by `area_available` and `length_available`. The source builds wall areas from prepared `OtdWall_GetArea` and slab areas from prepared polygons: these are not necessarily native read-back measurements. For an area comparison, request an applicable native area property for the actual element type and establish which side/quantity it represents. If comparing to `Отделка стен`, read the property on the zone, including its status. The inspected material summary accumulates a wall's area per material layer and can merge finish types that target one property; material rows are not automatically additive physical wall area. Names are project-specific; do not assume a particular localized property name or GUID exists in another project. Do not infer material identity from a matching number alone.

## Evidence limits

A single-zone AC25 test in a known build returned 25 new finish GUIDs, all independently found with target-zone ownership; the model contained one additional new linked GUID absent from that response. Another AC25 test showed prepared and native wall-area totals can differ. These observations demonstrate why independent read-back matters; they do not prove geometric completeness, all-property isolation, repeatability or behavior in other builds. Do not copy test-project GUIDs or private project data into reusable requests.
