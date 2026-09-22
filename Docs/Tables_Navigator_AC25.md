# Schedules (Navigator) and TableRenderer — Verified Reference for Archicad 25

Source documents merged and translated: `Tables_Navigator_AC25.md` (review date 2026-09-17) and `TableRenderer_AC25.md` (review date 2026-09-18, Issue #177). Project: AddOn_SomeStuff. Target version for both: **AC25**; porting to AC22–24/26–29 not verified.

**Status:** verified documentation edition. The Navigator part is a read-only review task (no code changed, no build/runtime run for that review itself). The TableRenderer part is a review and refinement of a draft spec against the installed SDK, not a rewrite; runtime/self-test results referenced below were produced separately, as logged in §11 and §12.9.

**Merge note:** the two source documents are companion documents — the Navigator document explicitly points to the TableRenderer document as the verified renderer spec, and cross-checking them found no contradictions (matching bounding-box figures, matching self-test timestamps, matching `CC_Cyrillic = 14` code, consistent unit conventions). They are combined here into one continuous reference, with the TableRenderer content placed in §12 and cross-referenced from the Navigator implementation log (§11) instead of being duplicated.

Sources:

- Navigator document source: a plain-text file (378 lines, an experience map for Ext-Table on AC29) — requirements/hypotheses from another project, not SDK documentation. Some AC29 function names do not exist in AC25; key discrepancies are noted inline.
- TableRenderer document source: draft spec `table-renderer-tz.md` (user attachment, 219 lines). Changes relative to the draft are marked **Correction**.
- Search method for both: LightRAG `http://127.0.0.1:9621/query` (`only_need_context: true`, `local`/`naive` modes; `hybrid` returned empty for these identifiers — a mode signal, not an absence of data), cross-checked against DevKit-25 headers and examples.

**Legend:** **Verified (SDK25)** — a header/documentation/example contract; **Proposal** — a design decision, not an SDK requirement; **not verified** — no evidence yet, a runtime check is needed.

Path abbreviations, all relative to `D:/SomeStuff_addon/Build/DevKit/APIDevKit-25/`:

- `H/` = `Support/Inc/`
- `F/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Functions/`
- `S/` = `Documentation/HTML/APIDevKit/APIHTMLLibrary/Structures/`
- `N/` = `Examples/Navigator_Test/Src/`

---

## 1. Architecture

A custom schedule — not a native Interactive Schedule. Verified mechanism from Navigator_Test:

**Custom viewpoint in the Project Map → MyDraw for viewing → CreateIDFStore for generating the drawing content.**

Model data, filters, grouping, table layout, styles, and pagination are our own implementation. Navigator_Test copies floor-plan elements (`N/NavigatorUtility.cpp:41–73`); it does not implement tables.

**Proposal:** separate the schedule definition and its formatting from the calculated rows, the 2D representation, and the selection state. Use a single 2D-content generator for both the window and the IDF. Actual printing/placement/updating of the schedule on a layout in SomeStuff is **not verified**; an end-to-end runtime prototype is needed.

## 2. Exact AC25 Call-Form Mapping

Below are **call forms**, not declarations of separate functions with these names: in AC25, Navigator and Database use the `ACAPI_Navigator` and `ACAPI_Database` dispatchers.

| In the AC29 source                           | AC25 call form                                                                                                                                                                                                                                                                                                                | Source                                                                               | Status                   |
| -------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------ | ------------------------ |
| `ACAPI_Navigator_RegisterCallbackInterface`  | `ACAPI_Navigator(APINavigator_RegisterCallbackInterfaceID, &callback)`                                                                                                                                                                                                                                                        | `F/APINavigator_RegisterCallbackInterfaceID.html`, `N/NavigatorTest.cpp:134`         | Verified (SDK25)         |
| `ACAPI_Navigator_GetNavigatorVPRootGroups`   | `ACAPI_Navigator(APINavigator_GetNavigatorVPRootGroupsID, &rootGuids)`; result `GS::Array<API_Guid>`                                                                                                                                                                                                                          | `F/APINavigator_GetNavigatorVPRootGroupsID.html`, `N/NavigatorUtility.cpp:164–170`   | Verified (SDK25)         |
| `ACAPI_Navigator_GetNavigatorVPItem`         | `ACAPI_Navigator(APINavigator_GetNavigatorVPItemID, &vpData)`; input `vpData.guid`                                                                                                                                                                                                                                            | `N/NavigatorWindowHandling.cpp:127–132`                                              | Verified (SDK25)         |
| `ACAPI_Navigator_CreateNavigatorVPItem`      | `ACAPI_Navigator(APINavigator_CreateNavigatorVPItemID, &vpData)`; data in/out                                                                                                                                                                                                                                                 | `F/APINavigator_CreateNavigatorVPItemID.html`                                        | Verified (SDK25)         |
| `ACAPI_Navigator_ChangeNavigatorVPItem`      | `ACAPI_Navigator(APINavigator_ChangeNavigatorVPItemID, &vpData)`; data in, GUID selects the existing record                                                                                                                                                                                                                   | `F/APINavigator_ChangeNavigatorVPItemID.html`                                        | Verified (SDK25)         |
| `ACAPI_Navigator_GetNavigatorVPItemChildren` | `ACAPI_Navigator(APINavigator_GetNavigatorVPItemChildrenID, &parentGuid, &childrenGuids)`                                                                                                                                                                                                                                     | `F/APINavigator_GetNavigatorVPItemChildrenID.html`, `N/NavigatorUtility.cpp:186–200` | Verified (SDK25)         |
| Deleting a schedule                          | `ACAPI_Navigator(APINavigator_DeleteNavigatorVPItemID, &guid)`                                                                                                                                                                                                                                                                | `N/NavigatorCallbackInterface.cpp:74–91`                                             | Verified (SDK25 example) |
| `ACAPI_Window_NewWindow`                     | `ACAPI_Database(APIDb_NewWindowID, &windowPars, (void*)handlerProc)`                                                                                                                                                                                                                                                          | `N/NavigatorWindowHandling.cpp:156–165`                                              | Verified (SDK25 example) |
| `ACAPI_Drawing_StartDrawingData`             | `ACAPI_Database(APIDb_StartDrawingDataID, &scale)`; the dispatcher's second parameter after scale, `API_PenType**`, is optional                                                                                                                                                                                               | `F/APIDb_StartDrawingDataID.html`                                                    | Verified (SDK25)         |
| `ACAPI_Drawing_StopDrawingData`              | `ACAPI_Database(APIDb_StopDrawingDataID, &idfStore, &boundingBox)`; outputs `GSPtr` and `API_Box`                                                                                                                                                                                                                             | `F/APIDb_StopDrawingDataID.html`, `N/NavigatorCallbackInterface.cpp:36–45`           | Verified (SDK25)         |
| `ACAPI_ProjectOperation_CatchProjectEvent`   | **Does not exist.** `ACAPI_Notify_CatchProjectEvent(GSFlags, APIProjectEventHandlerProc*)`                                                                                                                                                                                                                                    | `H/ACAPinc.h:1300–1301`                                                              | Verified (SDK25)         |
| `ACAPI_Attribute_GetAttributesByType`        | **Does not exist.** `ACAPI_Attribute_GetNum` + `ACAPI_Attribute_Get`                                                                                                                                                                                                                                                          | `H/ACAPinc.h:412–414`                                                                | Verified (SDK25)         |
| `ACAPI_ProjectSetting_GetStorySettings`      | **Does not exist.** `ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr)`                                                                                                                                                                                                                                       | `H/APIdefs_Environment.h:88`                                                         | Verified (SDK25)         |
| `ACAPI_AddOnObject_*`                        | Free functions with the `ACAPI_AddOnObject_` prefix and the suffixes `CreateObject`, `CreateUniqueObject`, `CreateUniqueObjectMore`, `CreateClientOnlyObject`, `GetObjectList`, `GetObjectContent`, `ModifyObject`, `DeleteObject`, `GetObjectGuidFromName`, `GetUniqueObjectGuidFromName`, `GetClientOnlyObjectGuidFromName` | `H/ACAPinc.h:1262–1282`                                                              | Verified (SDK25)         |

### Registration and interface

`INavigatorCallbackInterface` exists in AC25. The callback object must stay alive for as long as Archicad uses it; its lifetime is managed by the add-on (`F/APINavigator_RegisterCallbackInterfaceID.html`, Remarks).

The source omitted two required pure-virtual methods: `ExecuteMergePostProcess` and `GetElemsForDrawingCheck`. The full list of required methods is in `H/INavigatorCallbackInterface.hpp:82–90`:
`OpenView`, `OpenSettings`, `ExecuteMergePostProcess`, `CreateIDFStore`, `GetElemsForDrawingCheck`, `NewItem`, `DeleteItem`, `RenameItem`, `GetIcon`. Context-menu settings and TW texts have default implementations (`:91–94`, `:108–129`).

Exact AC25 declarations:

```cpp
virtual GSErrCode CreateIDFStore (const API_Guid& viewPointID, double scale, double& clipBoxWidth, double& clipBoxHeight, GSPtr& idfStore, API_Box& boundingBox, double& paddingX, double& paddingY, GS::Array<API_Guid>& elems) const = 0;
virtual GSErrCode GetElemsForDrawingCheck (const API_Guid& viewPointID, GS::Array<API_Guid>& elems) const = 0;
virtual GSErrCode ExecuteMergePostProcess () const = 0;
```

These are reference declarations from `H/INavigatorCallbackInterface.hpp:84–86`, not a ready-made implementation class.

Handling the file lifecycle is a separate registration: `ACAPI_Register_NavigatorAddOnViewPointDataHandler()` in `RegisterInterface`, setting up the merge/save-old/convert-new handlers in `Initialize` (`F/ACAPI_Register_NavigatorAddOnViewPointDataHandler.html`; `N/NavigatorTest.cpp:84–145`). Registering the callback interface alone is not enough to consider merge/migration implemented.

## 3. Navigator Identity and Constraints

**Verified (SDK25):**

- When creating a leaf node, the SDK ignores the input `guid` and always issues a new one. Root/group nodes accept an explicitly given GUID. Save the returned GUID; do not try to assign it to a leaf node yourself (`F/APINavigator_CreateNavigatorVPItemID.html`, Remarks).
- `ChangeNavigatorVPItemID` changes an existing node by GUID but ignores `parentGuid` and `itemType`: this call cannot move a node or change its type (`F/APINavigator_ChangeNavigatorVPItemID.html`).
- In Teamwork it is forbidden to create, change, or delete roots/groups. Changing a node that is not reserved returns `APIERR_NOTMINE`. This restriction was absent from the original AC29 map.

**Proposal:** preserve the schedule's GUID across a normal recalculation. This is a necessary design condition for stable links, but whether all Drawing/formatting variants survive updates still requires a runtime check. Merging into another PLN is a separate case involving identity reassignment, not a promise of an unchanging GUID across all projects.

The `ScheduleInternal:<serial>` counter is a user-defined format, not an API requirement. Collisions during merge/Teamwork must be resolved explicitly; a local increment alone does not prove uniqueness.

## 4. MyDraw: Window, Identifier, and Refresh

- `APIWind_MyDrawID` allows real 2D elements but not construction elements; user input and editing are disabled (`F/APIDb_NewWindowID.html`, Remarks). **Verified (SDK25)**. An editor in DG/BrowserPalette is a possible separate UI, not a required SDK choice.
- `API_NewWindowPars.userRefId` — an `API_Guid` returned in notifications (`H/APIdefs_Database.h:256–264`). The example sets it to the viewpoint's GUID (`N/NavigatorWindowHandling.cpp:156–162`).
- The callback receives `(const API_Guid& userRefId, API_NotifyWindowEventID notifID)`; events are Activate, Rebuild, and Close (`F/APICustomWindowHandlerProc.html`).
- Identity is available not only via Activate: the example gets `API_WindowInfo.databaseUnId.elemSetId` by querying the current window and comparing it against its own windows (`N/NavigatorWindowHandling.cpp:195–215`). Do not use `index` as the schedule's key.
- Opening a new window is forbidden at notification level: `APIERR_REFUSEDCMD` (`F/APIDb_NewWindowID.html`). Do not open windows arbitrarily from change handlers.

**Discrepancy within SDK25:** the old example in `F/APIDb_NewWindowID.html` uses `userRefCon`, which is absent from the current structure; `S/API_WindowInfo.html` still describes the old custom-window-by-index approach. For the implementation, use the current `H/APIdefs_Database.h:256–264`, the GUID-based callback signature, and `N/NavigatorWindowHandling.cpp:125–168,195–215` — not these outdated HTML fragments.

### Reset is not Rebuild

`APIDb_ResetCurrentDatabaseID` **deletes all elements** of its own drawing database (`F/APIDb_ResetCurrentDatabaseID.html`). `APIDb_RebuildCurrentDatabaseID` rebuilds the current database; the documentation does not confirm that this call alone necessarily destroys the last valid content (`F/APIDb_RebuildCurrentDatabaseID.html`).

In Navigator_Test, `APIWindowGuard` calls Reset in its constructor and SetZoom/Rebuild in its destructor (`N/NavigatorWindowHandling.cpp:111–122`). This is **not a database-switch/restore guard** and not a rollback.

**Proposal:** show a warning without destructive regeneration; prepare data and layout before Reset, and do not mark content as successful on error. A last-good guarantee after a mid-geometry-creation error is **not verified**; a separate recovery strategy is needed.

## 5. IDF and Validators

`StartDrawingDataID` redirects element creation into a temporary store; only 2D elements are allowed, including images. Nested sessions are forbidden (`APIERR_NESTING`). A 1:100 scale is passed as the number 100, not 0.01. The custom pen table holds exactly 255 entries and is set before generation (`F/APIDb_StartDrawingDataID.html`).

`StopDrawingDataID` ends the session and returns an opaque serialized `GSPtr` stream, not a drawing GUID. Returning a stream from `CreateIDFStore` and creating a drawing element yourself are different paths. Obtaining the IDF alone does not mean the drawing has been placed on a layout.

In the implementation, Start/Stop must be balanced only for a session that was successfully started by the same code; the original generation error must be preserved and not overwritten by a cleanup result. The `N/NavigatorCallbackInterface.cpp:36–45` example is not a model of complete error handling: it calls Stop unconditionally and does not check the result of copying elements.

`API_WindowValidatorInfo` stores the window GUID, `elemList`, and `checkSumList` (`S/API_WindowValidatorInfo.html`). The example itself checks validity on activation, updates the validator, handles Rebuild, and destroys the validator on Close (`N/NavigatorWindowHandling.cpp:15–82`). This is **not an autonomous subscription to any property/filter change**.

**Proposal:** dependencies include the current set of source elements and the schedule definition. Changes to the set via filter, new elements, project settings, and dependencies external to the GUID must be handled separately. `GetElemsForDrawingCheck` returns the elements the drawing depends on; a window validator alone is not enough to promise automatic updates of a placed Drawing in every case.

## 6. Storage and Memory Ownership

### 6.1 Viewpoint payload

**Verified (SDK25):** `API_NavigatorAddOnViewPointData` is not POD. It cannot be initialized via `memset/BNZeroMemory/BNClear`. The `data` member is owned by the structure itself: the copy constructor copies the handle, move transfers and clears the source, and the destructor frees it (`H/APIdefs_Navigator.h:226–283`; `S/API_NavigatorAddOnViewPointData.html`, Remarks).

Do not manage `.data`'s lifetime with a separate `BMKillHandle`, `BMHandleToHandle`, or a second HandleGuard. Do not create a shallow alias between owners. `F/APINavigator_CreateNavigatorVPItemID.html` explicitly states that the destructors will free `nodeVP.data` and `nodeVP2.data`. `ChangeNavigatorVPItemID` takes const input; the API does not document any transfer of ownership. The source's generalization about ownership after Change is replaced by this contract.

The viewpoint payload is saved in the project: `S/API_NavigatorAddOnViewPointData.html`, member `data`; `F/APINavigatorAddOnViewPointDataSaveOldFormatHandlerProc.html`, parameters `currentFormatVPDataArray`/`oldFormatVPDataArray` and Remarks. **A separate Add-On Object is not required by the SDK.** It remains an architectural option for data that needs its own lifecycle. The speed/limits of large payloads have not been checked here.

### 6.2 Add-On Object

Signatures cross-checked against `H/ACAPinc.h:1262–1280`. The table gives call forms, where the variables assume the corresponding declared types.

| AC25 form                                                     | Contract                                                                                                                                    | Source                                           |
| ------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------ |
| `ACAPI_AddOnObject_CreateObject(name, content, &guid)`        | `name`: const UniString&, `content`: const GSHandle&, GUID is an out-parameter; content is copied, the caller frees its own input handle    | `F/ACAPI_AddOnObject_CreateObject.html`          |
| `ACAPI_AddOnObject_ModifyObject(guid, &newName, &newContent)` | GUID is a const reference; name/content are pointers, `nullptr` can be passed for one to keep the old field but not both; content is copied | `F/ACAPI_AddOnObject_ModifyObject.html`          |
| `ACAPI_AddOnObject_GetObjectContent(guid, &name, &content)`   | both output pointers are required; Archicad allocates the handle, the caller frees it with `BMKillHandle`                                   | `F/ACAPI_AddOnObject_GetObjectContent.html`      |
| `ACAPI_AddOnObject_GetObjectGuidFromName(name, &guid)`        | if absent — `APINULLGuid`; with several identical names, which GUID is returned is undefined; not suitable for unique objects               | `F/ACAPI_AddOnObject_GetObjectGuidFromName.html` |
| `ACAPI_AddOnObject_DeleteObject(guid)`                        | GUID is passed by const reference; deletes from the project database; ownership is required in Teamwork                                     | `F/ACAPI_AddOnObject_DeleteObject.html`          |

Regular object names are **not unique**. So "find by name → create" does not guarantee a singleton, especially with concurrent creation by Teamwork users.

`ACAPI_AddOnObject_CreateUniqueObject(name, &guid)` creates an object with no initial content; a Modify call is required afterward. The name is unique among unique objects but may match a regular object's name; lookup is via `ACAPI_AddOnObject_GetUniqueObjectGuidFromName`. Modify of a unique object cannot change the name: `newObjectName = nullptr`, `newObjectContent != nullptr`. Creating a unique object triggers a **Full Receive + Full Send** in online Teamwork, and is forbidden offline (`F/ACAPI_AddOnObject_CreateUniqueObject.html`). This is not a free replacement for a regular registry.

Client-only objects are not sent to the server (`F/ACAPI_AddOnObject_CreateClientOnlyObject.html`). Modify/Delete of regular shared objects require ownership (`APIERR_NOTMINE`); reserving may trigger Receive Changes (`F/ACAPI_AddOnObject_ReserveObjects.html`). Do not assume that owning a viewpoint automatically gives ownership of a separate object.

The common persistence-contract source: `Documentation/HTML/APIDevKit/APIHTMLLibrary/Level3/AddOnObject_Manager_id.html`: bytes are saved in the project database; merge/old-format-conversion callbacks are supported. Save/reopen and Teamwork have not actually been exercised.

**Proposal:** choose a single source of truth: viewpoint payload or a separate object with an explicit GUID link. Do not duplicate the mutable table in two registries. Version the serialization from the first version onward: magic/version, sizes, encoding, bounds checks, an unknown-version policy. Do not serialize raw `std::map`/`std::vector`/`GS::UniString` or a machine layout struct via memcpy.

## 7. Model Selection and Metadata

### 7.1 Available AC25 APIs

| Task                       | AC25                                                                                                                                                                                           | Source                                                                                   |
| -------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------- |
| Element Type, Layer, Story | `ACAPI_Element_GetHeader(&header, mask)`; fields `typeID`, `layer`, `floorInd`                                                                                                                 | `H/ACAPinc.h:573–574`, `H/APIdefs_Elements.h:180–191`                                    |
| Layers                     | instead of the new `ACAPI_Attribute_GetAttributesByType`: `ACAPI_Attribute_GetNum(API_LayerID, &count)`, then `ACAPI_Attribute_Get(&attribute)` with header.typeID/index                       | `H/ACAPinc.h:412–414`, `F/ACAPI_Attribute_GetNum.html`, `F/ACAPI_Attribute_Get.html`     |
| Stories                    | instead of `ACAPI_ProjectSetting_GetStorySettings`: `ACAPI_Environment(APIEnv_GetStorySettingsID, &storyInfo, nullptr)`; free `storyInfo.data`                                                 | `H/APIdefs_Environment.h:88`, `F/APIEnv_GetStorySettingsID.html`, `S/API_StoryInfo.html` |
| Classification systems     | `ACAPI_Classification_GetClassificationSystems`, `ACAPI_Classification_GetClassificationSystemRootItems`, `ACAPI_Classification_GetClassificationItemChildren`                                 | `H/ACAPinc.h:1465,1477,1471`                                                             |
| Element classifications    | `ACAPI_Element_GetClassificationItems(guid, pairs)`; pairs is an output by reference, pairs of system GUID / item GUID                                                                         | `H/ACAPinc.h:852`, `F/ACAPI_Element_GetClassificationItems.html`                         |
| Property definitions       | `ACAPI_Property_GetPropertyDefinitions(groupGuid, definitions)`; definitions by reference, `APINULLGuid` = all; `ACAPI_Property_GetPropertyGroup(group)` requires `group.guid` to be filled in | `H/ACAPinc.h:1433–1435`, corresponding F/\*.html                                         |
| Property value             | `ACAPI_Element_GetPropertyValue(elemGuid, definitionGuid, property)` — the third parameter is an **API_Property&**, not a pointer                                                              | `H/ACAPinc.h:904–906`, `F/ACAPI_Element_GetPropertyValue.html`                           |
| Display string             | `ACAPI_Property_GetPropertyValueString(property, &text)` — property is a const reference, output is `UniString*`                                                                               | `H/ACAPinc.h:1456–1457`, `F/ACAPI_Property_GetPropertyValueString.html`                  |

Caveats:

- `GetNum` returns the **maximum index**, not the number of live records; deleted attributes leave gaps.
- The story above the topmost one in `API_StoryInfo` is virtual: do not include it as a regular story in a user filter.
- Read a property value only when `API_Property_HasValue`; `NotAvailable`/`NotEvaluated` is not an empty string (`S/API_Property.html`).
- The display string depends on the Project Preferences units and expands enumerations. Compare numbers and enumerations by their typed values; use the string only for output. The function name given is available since API25; the documentation notes the former name `APIAny_GetPropertyValueStringID`, but implementations in other versions were not checked here.
- `ACAPI_Element_GetElemList` uses `APIFilt_None` by default; `API_ZombieElemID` denotes all types. A single such call is not proof of traversing every independent project database (`F/ACAPI_Element_GetElemList.html`). Define the meaning of "whole project" separately.
- A read-only selection filter must not implicitly exclude other users'/non-editable elements. Choose `onlyEditable` deliberately; free the marquee handle (`F/ACAPI_Selection_Get.html`). A saved selection as a fixed set of GUIDs and "current selection on every refresh" are different requirements.

**SomeStuff:** take "Monitor" values from `PROPERTYCACHE()` rather than adding an independent read of every property for every element. In `.property` — only definitions; values are looked up separately per element (`AGENTS.md`, §6). The API table above is a reference, not a directive to bypass the cache.

### 7.2 Current database

AC25: `ACAPI_Database(APIDb_GetCurrentDatabaseID, &original)` → `ACAPI_Database(APIDb_ChangeCurrentDatabaseID, &model)` → restore the original with error checking. This changes the background current database for database-dependent calls, not the foreground window (`F/APIDb_ChangeCurrentDatabaseID.html`).

The original claim that "GetHeader necessarily breaks in MyDraw" is too categorical: a mask exists for reading the floor-plan header without switching. The correct name is **`APIElemMask_FromFloorplan`** (`H/APIdefs_Elements.h:227`); `F/ACAPI_Element_GetHeader.html` still has the old `APIElemMask_FloorPlan`.

A mandatory failure of `GetPropertyValue` from MyDraw is not confirmed by the contract. DatabaseGuard is a project-level way to control context and restoration, not proof of a specific failure. Data is preferably collected before starting the IDF session/changing MyDraw; do not mix model reading and generation without checking contexts.

## 8. Events and Updates

`ACAPI_ProjectOperation_CatchProjectEvent` from the source is replaced in AC25 by `ACAPI_Notify_CatchProjectEvent(GSFlags, APIProjectEventHandlerProc*)` (`H/ACAPinc.h:1300–1301`). But a rename alone is not enough:

- Project events and element observation are different things. For the latter: `ACAPI_Notify_InstallElementObserver`, `ACAPI_Element_AttachObserver(guid)`; for new elements — `ACAPI_Notify_CatchNewElement(nullptr, handler)` (`H/ACAPinc.h:673,1314–1317`).
- `APINotifyElement_PropertyValueChange`, `APINotifyElement_ClassificationChange`, Change/Edit/Delete, Undo/Redo, BeginEvents/EndEvents exist in AC25 (`H/APIdefs_Callback.h:216–232`).
- `APINotify_PropertyDefinitionChanged`, `APINotify_ClassificationItemChanged`, and visibility/units changes are separate events. **`API_AllProjectNotificationMask = 0x00000FFF` does not include them** (`H/APIdefs_Callback.h:155–172`). Set masks based on the schedule's dependencies; do not rely on the word "All."
- Watching only already-selected rows misses an element that later first satisfies the filter. Additions, classification/property changes, and the candidate set must be reconsidered.
- Notifications are not the place for a heavy global recalculation (`Documentation/HTML/APIDevKit/APIHTMLLibrary/Level3/notification_manager_id.html`). EndEvents marks the end of a series, not a guarantee of a safe, free UI context (`S/API_ElementDBEventID.html`).

**Proposal:** a notification marks the schedule as stale; the recalculation runs at an allowed point and consolidates the accumulated changes. A first prototype can update explicitly and on open. Full auto-recalculation completeness is a runtime task, not a default Navigator property.

AC25 has `ACAPI_Command_CallFromEventLoop` — a call to a **registered add-on command**, not an arbitrary lambda (`H/ACAPinc.h:1241–1246`, `F/ACAPI_Command_CallFromEventLoop.html`). The SDK example uses this special bridge from a worker; this is not permission to read/change the model in the background. Do not introduce new threads or a command mechanism in SomeStuff without a separate architecture discussion. CatchProjectEvent is not a thread dispatcher.

## 9. Palette, Formatting, and the Custom Table Model

There is no MyDraw flag among `API_PalEnabled_*` (`H/APIdefs_Interface.h:436–446`). However, this does not mean it is safe to simply "not register a `DG::Palette`." The SDK requires notifying the API about the palette via `ACAPI_RegisterModelessWindow`; registration provides special hide/disable/close messages, ties into the Work Environment, and is linked to keeping the add-on in memory (`F/ACAPI_RegisterModelessWindow.html`; `F/APIPaletteControlCallBackProc.html`). `ACAPI_UnregisterModelessWindow` is documented for ending use/FreeData, not as a way around MyDraw.

**not verified:** whether a specific palette disappears in MyDraw, and a safe workaround. Check separately if an editor is needed; for the first read-only schedule it can be omitted. Manual cells/styles are optional UI, not a Navigator function.

Project recommendations, not SDK contracts:

- A row may correspond to a single element, a group of elements, or part of an element. A stable logical row key and a link to the source GUIDs are needed; row position and a single `elementGuid` do not cover every schedule.
- `row * cols + col` works for addressing a cell in the current snapshot, but not for preserving manual values after sorting/grouping/changing `cols`. These need stable row and column keys and a conflict policy.
- A full recalculation of rows with a stable schedule GUID is acceptable; incremental refresh is not required before measurements/requirements for manual overrides exist.
- A sparse container is a choice based on fill rate and measurements, not automatically the best option. SUM/COUNT formulas and manual expressions are different amounts of work; computed totals do not necessarily require storing the user's expression.
- Styles, merges, row heights, column widths, text wrapping, header repetition, and pagination are a self-contained engine. There is no Ext-Table source here; the portability of its "Reflow" as-is has not been checked.
- A font family name is preferable to an index for a portable format, but font availability, fallback, and metrics on Windows/macOS need checking.
- UTF-8 is the proposed storage format. Do not treat `GS::UniString(L"…")` as a universal encoding fix or a replacement for resource localization. The cause of GRC errors and the correctness of specific literals/build settings in SomeStuff have not been investigated here.
- Re-read/invalidate project metadata when the project changes and on attribute/definition changes. A singleton UI does not guarantee data freshness. When a property/classification is removed, surface an explicit state; do not silently treat it as empty.
- Backup → persist → geometry update is not an atomic SDK transaction. Rolling back a separate storage can also fail, especially in Teamwork. A recovery policy is needed separately; do not promise a rollback with a single guard.
- Reset or transfer Selection/HUD by stable keys when the table model changes. A global keyboard hook is not needed without a separate requirement and a focus-owner check.

## 10. Runtime Checks Required Before Implementation Guarantees

1. Own viewpoint → MyDraw → drawing on a layout → repeated update without duplicates or lost references.
2. Save/close/reopen PLN; unknown payload version; merge with name/serial collisions and GUID reassignment.
3. Teamwork: initially missing root, node/separate-object reserve/release, owner conflicts, offline, cancelling Full Send/Receive.
4. Data from MyDraw/the plan; restoring the current database on every error path; window/IDF consistency.
5. New/deleted elements, crossing a filter boundary, PropertyValueChange, ClassificationChange, metadata, Undo/Redo, Receive Changes.
6. Missing property/classification, NotAvailable/NotEvaluated, zero row count, long Russian text, merges, header wrapping.
7. Error before and after Reset/StartDrawingData: last-good preservation, memory cleanup, no unclosed IDF session.
8. Large tables: selection/layout/render time and storage size; properties read via a consistent cache path.
9. The palette in MyDraw — only if the editor is in scope; Windows/macOS separately.

This document does not claim these scenarios have been implemented. Current state: the AC25 contract review is complete; the schedule framework and the general table-drawing class live in `Sources/AddOn/table/` (`TablesNavigator.hpp/.cpp`, `TableRenderer.hpp/.cpp`). Next step: agree on the first schedule, its storage, and its acceptance criteria.

## 11. Implementation Log

### Goal of the first implementation step

Build a safe framework for the schedules subsystem in Navigator/MyDraw for AC25 that:

1. Compiles with the add-on.
2. Is wired into the standard `RegisterInterface` and `Initialize` phases.
3. Creates a root Navigator section named `SomeStuff Schedules` / `SomeStuff Каталоги`, but does not yet create real schedules.
4. Fixes the correct boundaries of the future implementation: definition → snapshot rows → single renderer → MyDraw/IDF.
5. Does not get mixed up with the existing `Spec`, `Summ`, and the HTML Monitor.

### Implementation plan

- [x] Set up a separate module `Sources/AddOn/table/TablesNavigator.cpp/.hpp` (originally created under `Sources/AddOn/`, later moved).
- [x] Describe the minimal structures: `ColumnDefinition`, `RowIdentity`, `ScheduleDefinition`, `TableCell`, `TableRow`, `TableSnapshot`.
- [x] Add an `INavigatorCallbackInterface` skeleton with all AC25 pure-virtual methods: `OpenView`, `OpenSettings`, `ExecuteMergePostProcess`, `CreateIDFStore`, `GetElemsForDrawingCheck`, `NewItem`, `DeleteItem`, `RenameItem`, `GetIcon`.
- [x] Add a Navigator registration switch and enable it once the root-section name is agreed.
- [x] Implement `TablesNavigator::RegisterInterface()` as the future `ACAPI_Register_NavigatorAddOnViewPointDataHandler()` call site.
- [x] Implement `TablesNavigator::Initialize()` as the future setup of the merge/save-old/convert-new handlers and `APINavigator_RegisterCallbackInterfaceID`.
- [x] Wire `TablesNavigator::RegisterInterface()` and `TablesNavigator::Initialize()` from `SomeStuff_Main.cpp`.
- [x] Fix the name of the future Navigator section: `SomeStuff Schedules` / `SomeStuff Каталоги`.
- [x] Implement `TablesNavigator::EnsureNavigatorRoot()` and call it on `APINotify_AllInputFinished`.
- [x] Verify the runtime scenario of `SomeStuff Schedules` / `SomeStuff Каталоги` appearing in the Project Map after opening `test_25`.
- [x] Implement creation of the first node-catalog under `SomeStuff Schedules` / `SomeStuff Каталоги` via `NewItem`.
- [x] Implement the general table-drawing class `TableRenderer` (layout + render) per the verified edition of the spec — see §12 below for the full spec.
- [x] Implement the IDF path: a balanced `StartDrawingData`/`StopDrawingData`, preserving the original generation error, correct bounding box/padding/elems.
- [/] Implement a safe MyDraw renderer without destroying last-good content on error: the window opens and draws content; a last-good recovery strategy for an error mid-geometry-creation is not implemented.
- [x] Move the table code into its own folder, `Sources/AddOn/table/`.
- [/] Debug the text in the schedule window: Cyrillic was rendered with the wrong glyphs (user, 2026-09-18). Fixed the "layout computed before the window database reset" ordering and taking `charCode` from the font; the result is pending user verification.
- [/] `StopDrawingDataID` bounding box: the grid and fills match the calculation, the text layer inflates the bounding box. Cause **not verified** — see §12.8.
- [ ] Implement the first read-only schedule: row source, columns, stable row keys, validator dependencies.
- [ ] Choose and implement persistence: viewpoint payload or Add-On Object, format version, unknown-version policy.
- [ ] Implement the first catalog's content: columns, filters, stable row keys instead of the demo prototype.

### Done in code

- `TablesNavigator.hpp` separates the schedule's intent (`ScheduleDefinition`) from the calculated snapshot (`TableSnapshot`).
- The Navigator section name is moved into `ID_ADDON_STRINGS` / `ID_ADDON_STRINGS_ENG` as `SomeStuff Каталоги` / `SomeStuff Schedules`; the current resource is chosen via `isEng()`. Stable root GUID: `4CDA3758-ECF8-4B03-BC1C-5C6F444E4F31`.
- The root section's `displayId` is left empty: the Navigator shows `displayId + displayName`, and a non-empty internal ID produces a garbled label of the form "old name + new name." The internal key stays in `ScheduleDefinition::internalId`.
- `EnsureNavigatorRoot()` looks for an existing root by GUID or name, updates `displayName`/`displayId` on mismatch, and creates the root if it does not exist.
- `NewItem()` creates a leaf node-catalog under the selected root/group or next to the selected node, assigns a unique `SS001`, `SS002`... and a localized name `Новый каталог` / `New Schedule`, then immediately opens `OpenSettings()` for editing the ID/Name.
- `OpenSettings()` / `RenameItem()` open a compact DG dialog for ID/Name and save changes via `APINavigator_ChangeNavigatorVPItemID`; the root section is not editable through this dialog.
- `DeleteItem()` deletes non-root elements via `APINavigator_DeleteNavigatorVPItemID`; Teamwork ownership and conflict scenarios remain a runtime task.
- The root is created in `ProjectEventHandlerProc` on `APINotify_AllInputFinished`; an error does not interrupt opening the project and must be checked separately in Teamwork/runtime.
- `RowIdentity` does not reduce to a row position or a single GUID: a row can correspond to an element, a group, or an aggregate.
- `TableSnapshot` is intended as the common renderer input for both the open MyDraw window and `CreateIDFStore`.
- `CreateIDFStore` is implemented: `APIDb_StartDrawingDataID` → `TableRenderer::Draw` → `APIDb_StopDrawingDataID`; `clipBoxWidth/Height` are supplied before the session opens, a drawing error is not overwritten by a cleanup error, IDF memory is freed via `BMKillPtr`.
- `OpenView` opens the MyDraw window and creates content in the same call (as `NavigatorCallbackInterface::OpenView` does in the example): `APIDb_NewWindowID` → layout is computed BEFORE resetting the window database → `APIDb_ResetCurrentDatabaseID` → drawing → `APIDb_SetZoomID` + `APIDb_RebuildCurrentDatabaseID`. Before the reset, it is checked that the current window is the schedule window.
- `ScheduleWindowHandlerProc` handles subsequent `APINotifyWindow_Rebuild`/`_Activate`/`_Close`; the window is reused via `APIDb_SetWindowIdID` + `APIDo_ChangeWindowID`, the validator is `APIDb_Build/Check/Rebuild/DestroyWindowValidatorID`.
- `GetElemsForDrawingCheck` no longer triggers a render: a freshness check should not mutate the database.
- The general table-drawing class has been moved into `Sources/AddOn/table/TableRenderer.hpp/.cpp`: input — cell data + formatting rules; internally — input validation, column widths (`Auto`/`Fixed`, min/max), distribution for multi-column merges, text wrapping, row heights, grid nodes, suppressing merged interior lines, range fills, and drawing in the order "fills → grid → text." (See §12 for the full spec.)
- Units: rules are in paper millimeters, layout output is in model units (`mm * scale / 1000`), where `scale` is the same value passed to `APIDb_StartDrawingDataID`.
- Text is drawn one line per element (`API_TextType` has no vertical alignment), the line position is computed by the renderer; `charCode` is taken from the typeface itself.
- The node's demo content is `TableRenderer::SetPrototypeContent()` (a 3×3 table with a merge and a narrow `Fixed` column) as a temporary prototype pending the finish schedule's rows.
- The self-test `TableRenderer::RunSelfTest()` under `TESTING`, called from `TablesNavigator::EnsureNavigatorRoot` once per session: logs layout sizes, error codes, the drawing-data bounding box, and the per-layer isolation (`fills`/`grid`/`texts`).
- Merge/save-old/convert-new handlers exist as extension points but are not called while the registration switch is off.
- `SomeStuff_Main.cpp` calls the module in the normal lifecycle; with the current switch off, these calls return `NoError`.

### Intentionally not done

- Catalog content is not implemented: real columns, filters, rows from the model (currently a demo prototype).
- No payload is written to the viewpoint, and no Add-On Object is created.
- No Navigator icons or new menu items have been added.
- Teamwork, merge, save/reopen, and layout placement are not supported.
- A last-good strategy is not implemented: an error partway through creating window geometry will leave the window partially filled.

### Validation status

#### 2026-09-18 — general table-drawing class (#177) and a visible schedule window

- Verified edition of the renderer spec — merged into §12 below.
- A node's `OpenView` opens a MyDraw window (`APIDb_NewWindowID`, `API_NewWindowPars.userRefId = node's guid`) and creates content **in the same call** — as `NavigatorCallbackInterface::OpenView` does in `Navigator_Test`: `OpenWindow` → `APIWindowGuard` (reset via `APIDb_ResetCurrentDatabaseID`) → `TableRenderer::Draw` → `APIDb_SetZoomID` + `APIDb_RebuildCurrentDatabaseID`.
  Gotcha (verified at runtime, 2026-09-18): if content is only created in the window handler, the window opens **empty** — when the window is created, `APINotifyWindow_Rebuild` is not delivered to the handler (the line `TablesNavigator schedule window render err` was missing from `test_results.txt`). The handler is kept for subsequent Rebuild/Activate.
  Before the reset, it is checked that the current window is indeed the schedule window (`APIDb_GetCurrentWindowID`, `typeID == APIWind_MyDrawID` and `databaseUnId.elemSetId == node's guid`), otherwise the reset would affect a different window (e.g. the floor plan). This approach is taken from `RegenerateContentIfAppropriate` in the example.
- `CreateIDFStore` is implemented: `APIDb_StartDrawingDataID` → drawing → `APIDb_StopDrawingDataID`; `clipBoxWidth/Height` are supplied before the session opens, a drawing error is not overwritten by a cleanup error, IDF memory is freed via `BMKillPtr`.
- `GetElemsForDrawingCheck` no longer triggers a render (a freshness check should not mutate the database).
- The node's content is a demo prototype table (`TableRenderer::SetPrototypeContent`), pending the finish schedule's rows.
- The drawing-data bounding box in the self-test does not match the calculated one (see §12.8 below) — cause not verified, per-layer isolation added.

#### 2026-09-18 — state at the point of stopping (after the move into `Sources/AddOn/table/`)

- Code location: `Sources/AddOn/table/TablesNavigator.hpp/.cpp` and `Sources/AddOn/table/TableRenderer.hpp/.cpp`. The build picks up subfolders (`GLOB_RECURSE` in `Tools/CMakeCommon.cmake:193-197`), `Sources/AddOn` is already in the include paths (`:272`), so CMake was not changed; the external include was updated to `table/TablesNavigator.hpp`.
- Self-test (AC25 Windows Debug, 14:21, `test_25`): layout computes correctly — the `Fixed` column is exactly `0.020`, wrapping the long text gives a line of `0.011`; `Draw err = 0`, `StopDrawingData err = 0`; `font=594 charCode=14` (`14` = `CC_Cyrillic`, `GSRoot/CH.hpp:78`).
- Per-layer bounding-box isolation: `fills` = 0.055552 × 0.018, `grid` = 0.060582 × 0.018 (matches the calculation), `texts` = 1.528275 × 0.181667 — the text layer inflates the bounding box. Cause **not verified**, analysis in §12.8 below.
- Open, pending user verification: Cyrillic text in the schedule window was rendered with the wrong glyphs. Fixed: (a) the ordering "layout is computed before the window database reset" — otherwise the text defaults are taken from an empty window, and (b) `charCode` is now taken from the typeface rather than from `ACAPI_Element_GetDefaults`. What exactly caused it has not been proven; another visual check is needed.
- Not checked at runtime: placing the schedule on a layout, updating after model edits, Teamwork, save/reopen.

SDK contracts for the points used were cross-checked against DevKit-25 and `Navigator_Test`.
Compiled: AC25 Windows Debug built by the standard runner on 2026-09-18 14:21 (`build=True`). Earlier the same day — 12:16, 14:05, 14:09, 14:19; builds after 14:05 include the MyDraw window.
Runtime tested: `test_25` opens via the runner; `test_results.txt` contains `TablesNavigator::EnsureNavigatorRoot : created schedules root`, `TablesNavigator::NewItem : created schedule node`, the self-test and per-layer-isolation lines, `TEST : end`, 483 lines of `: ok`, 0 `ERROR IN TEST`. Visually confirmed by the user: the schedule window opens; the table content needs more work (text). The complete schedule, the MyDraw editor, and layout placement are not verified.

---

## 12. TableRenderer — Table-Drawing Class

Review date: 2026-09-18. Issue: #177. Target version: **AC25**.

Source: draft spec `table-renderer-tz.md` (user attachment, 219 lines). This is a review and refinement of the draft against the installed SDK, not a rewrite. Changes relative to the draft are marked **Correction**.

### 12.1 Correction to the draft's architectural assumption (draft §1)

Draft: "draws the table with lines/text/fills through the existing render pipeline (`ACAPI_Drawing_StartDrawingData` / `Line` / `Text` / `Hatch` / `ACAPI_Drawing_StopDrawingData`)."

**Correction.** These functions do not exist in AC25. The drawing mechanism is "redirected element creation":

1. `ACAPI_Database (APIDb_StartDrawingDataID, [double* scale], [API_PenType** pens])` — opens an internal store; **all further 2D-element creation goes into it**, not into the project database. `APIERR_NESTING` if a session is already open (nesting is forbidden). Source: `H/APIdefs_Database.h:91`, `F/APIDb_StartDrawingDataID.html` — Verified (SDK25).
2. Inside the session — `ACAPI_Element_Create (&element, &memo)` with ordinary 2D elements: `API_LineID`, `API_HatchID`, `API_TextID`. Source: `Examples/Element_Test/Src/Element_Basics.cpp:1175-1193` (line/circle in drawing data), `Navigator_Test/Src/NavigatorCallbackInterface.cpp:38` (the real `CreateIDFStore` path) — Verified (SDK25).
3. `ACAPI_Database (APIDb_StopDrawingDataID, GSPtr* idfMem, API_Box* boundBox)` — returns the serialized IDF. The caller frees `idfMem`'s memory (`BMKillPtr`). Source: `H/APIdefs_Database.h:92`, `F/APIDb_StopDrawingDataID.html` — Verified (SDK25).

Consequences for the class's boundaries:

- `Draw` **does not open or close** the session: the session owner is the caller (`NavigatorCallbackInterface::CreateIDFStore`). The class draws into an already-open store.
- "Only 2D elements (including figures/pictures) are allowed" (`F/APIDb_StartDrawingDataID.html`) — text, lines, and fills are permitted; 3D/MEP elements are not.
- The order of primitives within the session equals the order elements are created, so draft §5 (fills → grid → text) still stands.

### 12.2 Units of Measurement (correction, draft §2–§3)

**Verified (SDK25):**

- `APIAny_GetTextLineLengthID` returns the line length **in mm** (`F/APIAny_GetTextLineLengthID.html`).
- `API_TextLinePars.wSize` — "Character height in mms"; `drvScaleCorr`: true — "scale the text to the model," false — "in mms" (`S/API_TextLinePars.html`, `H/APIdefs_Goodies.h:304-318`).
- `API_TextType.size` — "char height in mm" (`H/APIdefs_Elements.h`).
- `APIDb_StartDrawingDataID.dScale` — "defines the scaling from paper to model. For example, for 1:100 scaling, pass 100" (`F/APIDb_StartDrawingDataID.html`).

**Proposal (adopted in the implementation).** All formatting rules are given in **paper millimeters** (font size, padding, fixed widths/heights) — these are the SDK's text units.

- `ComputeLayout` computes geometry in mm and converts it to model units (mm → m) with the factor `scale / 1000`, where `scale` is the same value passed to `APIDb_StartDrawingDataID`.
- `GetTotalWidth / GetTotalHeight / GetColumnWidth / GetRowHeight` return **model units** (what `Draw` works in).
- Text is measured with `drvScaleCorr = false` (mm), so it does not depend on the scale.

The exact correspondence between "rule mm ↔ model units in drawing data" is checked visually: **not verified** (see §12.8).

### 12.3 Input Data (draft §2, with refinements)

#### 12.3.1 Cell data (what to draw)

Draft: `int row/col`, `GS::UniString text`, `fontNameOverride`, `hAlignOverride`.

**Corrections:**

- Types are project-specific: `Int32`, `GS::Array` (the project uses `GS::Array`, not `std::vector`; the draft itself allows fixing the signatures after checking against the project's types).
- `fontNameOverride` → **font index** (short). In the SDK, text addresses a font by attribute index (`API_TextType.font` — "font index"), not by name. Resolving a name to an index is a separate helper (`ACAPI_Attribute_Search`) and only at the rule level.
- `HAlign` gets a `Default` value — "take it from the rules," as originally intended in the draft.
- Added `VAlign vAlignOverride` by symmetry (the SDK has no vertical text alignment, see §12.5.3 — the renderer emulates it).

The sparse representation is kept: a cell missing from the array is considered empty.

#### 12.3.2 Rules (how to draw)

The set of draft fields is kept; refinements:

- `ColumnRule.fixedWidth / minWidth / maxWidth` — in mm.
- `RowRule.fixedHeight / minHeight` — in mm.
- `gridLineWeight` → **`gridPenIndex` (short)**: in ArchiCAD, line thickness is set by pen (`API_LineType.linePen.penIndex` — Verified (SDK25), `Element_Basics.cpp:4136-4147`), not by a "weight" in mm.
- Added: `layerIndex`, `lineTypeIndex`, `textPenIndex`, `fillPenIndex`, `fillContourPenIndex`, `defaultFontIndex`, `defaultFontName`, `defaultFaceBits`. A value of `0` means "take from `ACAPI_Element_GetDefaults`"; `layerIndex = 0` is not guessed manually.
- Fills — as a **list of ranges** (resolving an open question from the draft), analogous to `mergedRanges`: `FillRange { MergedRange range; API_AttributeIndex fillIndex; short fillPen; short contourPen; }`. If `fillIndex` is invalid, the range's fill is not drawn.

### 12.4 The ComputeLayout Algorithm (draft §4 — adopted, with a measurement refinement)

12.4.1 Input validation: `rows/cols` against the actual cell range; `MergedRange` — a valid rectangle within the table bounds; overlapping merges → an input error (the renderer does not resolve them). Adopted unchanged.

12.4.2 Column widths (`Auto`): the maximum required width across unmerged cells (text + 2×`cellPaddingH`) → redistribution for multi-column merges proportionally to the current widths, `Fixed` columns are left untouched → a final clip to `minWidth`/`maxWidth`. `Fixed` — `fixedWidth` used directly. **Adopted unchanged.**

12.4.3 Row heights (`Auto`): after the column widths, accounting for the number of lines after wrapping. **Adopted unchanged.** The order "widths first, then heights" is mandatory.

12.4.4 Text wrapping. **Refinement (resolving an open question from the draft):**

- Line-width measurement — `ACAPI_Goodies (APIAny_GetTextLineLengthID, &pars, &lenMm)`, `pars.index = -1` (last line / no terminator), `drvScaleCorr = false` (mm), `wSize` = font size in mm, `wFont` = font index, `wSlant = PI/2` (plain), `lineUniStr` = text. Source: `F/APIAny_GetTextLineLengthID.html`, `S/API_TextLinePars.html` — Verified (SDK25).
- Wrapping is done by the **renderer**, not by Archicad: split by words, then character-by-character if a word is wider than the available width. The line count from this step feeds into the row heights (12.4.3).
- `API_TextType.nonBreaking = false` ("wrap around destination rect") exists, but its interaction with `width`/`height`/`multiStyle` in drawing data has not been checked — **not verified**, so at this first layer wrapping is computed by the renderer itself.

### 12.5 Draw Order (draft §5 — adopted, with a refinement on the method)

1. Fills (`API_HatchID`) — one element per range.
2. Grid (`API_LineID`) — segments between grid nodes, except ones strictly interior to merges.
3. Text (`API_TextID`).

#### 12.5.1 Grid and merges

The draft's algorithm (full segment list → subtract interior segments for `MergedRange`) is adopted. The "strictly interior" check is by nodes: a segment is not drawn if both of its endpoints lie inside the merge rectangle (not on its border).

#### 12.5.2 Elements: verified fields (Verified (SDK25))

- Line: `element.header.typeID = API_LineID`, `line.linePen.penIndex`, `line.ltypeInd`, `line.begC`, `line.endC`; `ACAPI_Element_Create (&element, nullptr)`. Source: `Element_Basics.cpp:4136-4147`.
- Fill: `header.typeID = API_HatchID`, `hatch.fillInd`, `hatch.fillBGPen`, `hatch.fillPen.penIndex`, `hatch.ltypeInd`, `hatch.contPen.penIndex`, `hatch.poly.nCoords/nSubPolys/nArcs`, `memo.coords`/`memo.pends` (`BMhAllClear`, 1-based, the closing point duplicates the first) → `ACAPI_Element_Create` → `ACAPI_DisposeElemMemoHdls`. Source: `Element_Basics.cpp:177-211`.
- Text: `header.typeID = API_TextID`, `memo.textContent = BMhAllClear ((len + 1) * sizeof (GS::uchar_t))` + `GS::ucscpy` (`text.ToUStr ()`), `text.loc`, `text.size` (mm), `text.font`, `text.pen`, `text.anchor`, `text.just`, `text.nonBreaking` → `ACAPI_Element_Create` → `ACAPI_DisposeElemMemoHdls`. Source: `Element_Basics.cpp:3985-4010`.
- `API_JustID`: `APIJust_Left = 0`, `APIJust_Center`, `APIJust_Right`, `APIJust_Full` (`H/APIdefs_Elements.h:431-434`).
- `API_AnchorID`: `APIAnc_LT/MT/RT/LM/MM/RM/LB/MB/RB` (`H/APIdefs_Elements.h:2195-2205`).
- `header.layer`: a value from `ACAPI_Element_GetDefaults` — not invented manually (the examples have a "magic" constant `header.layer = 5`, which must not be repeated).

#### 12.5.3 Vertical alignment — emulated by the renderer

**`API_TextType` has no vertical-alignment field** (the full field list was checked against `H/APIdefs_Elements.h`) — there is only `anchor` ("kind of text center") and `just` ("justification of text").

**Proposal.** Since the renderer itself knows the line count and their height, it computes the position itself: one text line = one `API_TextID` with `anchor = APIAnc_LB`, and `loc.x`/`loc.y` are computed from `hAlign`/`vAlign` and the measured line width. This is deterministic and does not depend on the unverified semantics of the `just` + `anchor` combination.

Cost: more elements than "one text per cell"; a multi-line text as a single element is a possible optimization after a visual check (**not verified**).

### 12.6 Units in the Rules: Mapping to the Draft

| Draft                 | Implementation                     | Reason                                  |
| --------------------- | ---------------------------------- | --------------------------------------- |
| `int row/col`         | `Int32`                            | project style                           |
| `std::vector<T>`      | `GS::Array<T>`                     | project style                           |
| `fontNameOverride`    | `fontIndexOverride` (short)        | `API_TextType.font` — an index          |
| `HAlign` (no Default) | `HAlign::Default` + `..._Override` | "Default → from the rules"              |
| `gridLineWeight` (mm) | `gridPenIndex` (short)             | line thickness = pen                    |
| fills "schematic"     | `GS::Array<FillRange>`             | ranges, like `mergedRanges`             |
| `void Draw`           | `GSErrCode Draw`                   | creating an element can return an error |

### 12.7 Mini-test (draft §7) — moved into a self-test under `TESTING`

The scenario from the draft is used as a runtime check (`TableRenderer::RunSelfTest`), called from `TablesNavigator::EnsureNavigatorRoot` under `#ifdef TESTING`, once per session:

1. a 3×3 table with no merges, `Auto` — widths equal the max text width plus padding;
2. a merge (0,0)-(0,1) with long text — the widths of columns 0/1 grow, column 2 is unchanged;
3. column 1 is `Fixed`, the text does not fit → wraps by words, the row height grows;
4. grid: the interior line inside a merge is absent, the perimeter is drawn.

The self-test logs the line measurement (mm), the resulting widths/heights, and the drawing-data bounding box (`DBprnt`), then frees the IDF memory. Nothing is written to the project database.

### 12.8 Not Verified (needs a runtime/visual check)

- The exact correspondence of the "mm → model units" factor when `scale != 1.0` in drawing data (checked visually on a real table in MyDraw/IDF).
- **The drawing-data bounding box does not match the calculated one.** Self-test observation, 2026-09-18: the table is 0.0606 × 0.018 model units, but `APIDb_StopDrawingDataID` returned 1.528275 × 0.181667. A session with a single 10 mm line gives the correct bounding box of 0.01, so the issue is not in the drawing-data mechanism itself.
  **Per-layer isolation (run 2026-09-18 14:05) localized the cause to the text layer:** `fills` = 0.055552 × 0.018 (inside the table), `grid` = 0.060582 × 0.018 (exactly the table), `texts` = 1.528275 × 0.181667 (the overall bounding box is set entirely by them).
  So each text element carries a bounding box of roughly 1.5 m × 0.18 m, i.e. considerably larger than the `API_TextType.width/height` we set (35.55 mm × 2.5 mm), or the given box is not taken into account at all.
  Negative result: explicitly setting `API_TextType.width/height` (the "the element carries the default box from `ACAPI_Element_GetDefaults`" hypothesis) did not change the bounding box.
  Next hypothesis (unchecked): a text element may need `memo.paragraphs` — in the `Element_Basics.cpp:3985-4010` example it is allocated for multi-style text, and without it Archicad may take paragraph/box parameters from defaults. Check: a session with a single text element with explicitly small `width/height`.
  Until this is resolved, view the table in MyDraw: there the frame sets the window, not the IDF bounding box.
- The behavior of `API_TextType.anchor` + `just` for multi-line text as a single element.
- Whether `ACAPI_Element_GetDefaults` is available inside an open drawing-data session (if not — the renderer should report an error, not guess field values).
- Resolving a font name via `ACAPI_Attribute_Search` for non-Latin names (`API_Attr_Head.name` — `char[API_AttrNameLen]`; searching by GUID → Unicode name → name is documented; whether built-in fonts have a Unicode field has not been checked).
- The stacking order of fills/lines/text in the final IDF when elements are created in the same order.
- Behavior in Teamwork and during merge (outside the scope of this task).

### 12.9 Implementation Status (TableRenderer)

- `Sources/AddOn/table/TableRenderer.hpp/.cpp` — the class (geometry + render).
- `TableRenderer::SetPrototypeContent ()` — a demo 3×3 table (a merge in the header, a narrow `Fixed` column). This is a temporary content prototype pending the finish schedule's rows.
- Self-test — `TableRenderer::RunSelfTest ()`, `#ifdef TESTING`, called from `TablesNavigator::EnsureNavigatorRoot` (once per session, after the project opens).
- Navigator wiring in `TablesNavigator.cpp`:
  - `OpenView` → the MyDraw window (`APIDb_NewWindowID` + `API_NewWindowPars.userRefId`, resetting the window's current database → drawing → `APIDb_RebuildCurrentDatabaseID`);
  - `CreateIDFStore` → `APIDb_StartDrawingDataID` → `TableRenderer::Draw` → `APIDb_StopDrawingDataID` with correct cleanup and drawing-error priority over cleanup-error;
  - `GetElemsForDrawingCheck` no longer draws anything (freshness check only).
- Runtime-confirmed (self-test, AC25 Windows Debug, 2026-09-18 12:16): `Draw` err 0, `StopDrawingData` err 0; layout: the `Fixed` column = exactly 0.020, wrapping the long text gave a line height of 0.011.
- Visual check of the table in the MyDraw window is up to the user (requires restarting Archicad with the fresh add-on).
- Finish-schedule content and generating rows from the model — the next task.

---

## 13. Combined Validation Summary

Verified: the contracts listed above, checked against the installed DevKit-25 and the cited sources; the `API_TextType`/`API_HatchType`/`API_TextLinePars` formats and the codes `CC_Cyrillic = 14`, `APIInvalidAttributeIndex = 0`, `APIFace_Plain = 0`.
Compiled: AC25 Windows Debug, last build 2026-09-18 14:21 (runner, `build=True`).
Tested: the root section, node-catalog, and MyDraw window are created in `test_25`; the self-test layout/render passes with zero error codes. Text rendering in the window (Cyrillic), the IDF text bounding box, layout placement, and model-driven updates are **not verified**.
