# json_commands — JSON-команды аддона (AC25–29)

## Z7 #252 — текущий scoped diagnostic contract

- TESTING && ServerMainVers_2500, read-only `RoomGeometryDiagnostic`: приоритет `partitionSelfTest=true` → `transformSelfTest=true` → `partitionSurfaces=true` → обычный guid/collectRoomInfo/Inventory. `partitionSurfaces` требует guid зоны; без guid InvalidInput/partitionComplete=false, не Inventory. `partitionSelfTest` выполняет partition/beamFinish/native-adapter/canonical-grouping checks; фактический итог60checks/0failed, не полный TestKit. [по коду + parent runtime]
- `ReadZoneSurfacePartition` возвращает status Partitioned/ReadError, candidateSet=RoomRelations, candidateSetComplete/zoneSnapshotComplete/partitionComplete, кандидатные/zone/partition ошибки, sources и excludedFinish, Matched/Unmatched/Ambiguous pieces с world-контурами/нормалями/planeOffset/полным source-face provenance и раздельными площадями; beamFinish содержит низ/LeftSide/RightSide из beamPart, его beamErrors не заменяются общим partitionComplete. Экспорт не доказывает skin/component ownership. [по коду]
- `collectRoomInfo=true` действительно вызывает production CollectRoomInfo и сериализует сохранённый снимок без запуска создания отделки; без диагностических флагов сохраняется прежний geometry serializer (schemaVersion2/surfaceSnapshotVersion1). `RoomBookCommand::Execute` по-прежнему содержит отдельный TESTING surfaceWalls writer branch; без него вызывает обычный production RoomBook, теперь использующий surface-based геометрическую подготовку. NoValue schema/returned не доказательство корректного построения или отмены. [по коду]
- Фактическая приёмка native writer предыдущего behavior-equivalent запуска:60/60 selftests,63→63 элементов, полные ориентированные body contours ≤1e−6м/max6.5868125745221e−7м. Известны31skip/4trapezoids2.375999498м²/3out-of-scope returns; #252 OPEN. Свежий scoped MCP/generated, source hashes и ограничения — Docs/modules/Roombook.md §Z7 и scratch/z7-docs-scoped/final/generated-audit.json; runtime — scratch/z7-writer/deterministic-final-summary.json. [parent runtime + MCP/source audit]

## Z5 #252 — актуальная доступность диагностического входа

- Ограничения TESTING+AC25 сняты с RoomGeometryDiagnostic, DrawRoomSurfaceWalls и связанных вызовов. Текущий guard: TESTING && ServerMainVers_2500, как у существующего JSON API. Для регистрации добавлена прежняя развилка имён SDK AC27+. Исторические сведения TESTING+AC25 ниже описывают состояние Z3/Z4, не текущий guard. [по коду]
- Геометрическая диагностика сериализует общий RoomSurfaceSnapshot: `GetZone3DPolygons_StrictAPI` (tests/TestRoombookGeometry.cpp:452), `RoomGeometryDiagnosticCommand::Execute` (:770). По умолчанию guid читает `ReadRoomSurfaces` (:806); collectRoomInfo=true для зоны действительно вызывает `CollectRoomInfo` (:793) и сериализует `room.surfaceSnapshot` (:794), не создавая отделку. Дополнительные поля surfaceSnapshotStored/legacyCollectResult/legacyRoomValid/legacyEdgeCount/finishWallCount/finishSlabCount не подменяют status/readComplete; schemaVersion2 сохранён, surfaceSnapshotVersion1 добавлен, вершины имеют world, полигоны normalWorld/planeOffset. Без guid остаётся Inventory (:533). [по коду]
- Boolean transformSelfTest=true имеет приоритет над guid/collectRoomInfo (:772–775) и вызывает `RunSurfaceTransformChecks` (:21): status Passed/Failed, checks[], failed;28реальных C++ проверок affine/shear/translation, rotation/reflection, отверстий/локальных координат/нормалей/плоскости и7отказов. В runtime AC25 получено28checks/0failed и28ok в текущей VS pane родителя. Это проверка общего transform, не fixture nonidentity тела SDK. [по коду + parent real JSON/VS]
- Финальный AC25 Debug APX `fdb0d8750c476b80554d96b9002f38b90b78cf98d22b7f5331521fe3dbe6a827`, PID38880/Run:16GUID legacy projections совпали с baseline,16повторов exact; OtdRoom хранит37полигонов зоны, stored=true/Extracted/errors0, legacy valid=true/edges16, оба finish counts0, повтор снимка exact.125полигонов/594world vertices/4отверстия, plane gap4.809999989419111e−7м ≤1e−6;14ID API.GetAllElements и оба saved PLN hashes unchanged. Invalid GUID →ReadError/readComplete=false/headerError; collectRoomInfo для стены →InvalidInput/readComplete=false. Evidence: Hermes scratch/z5-roombook-surfaces-20261011-121537/runtime-current/final-summary.json. [parent build + executed runtime assertions]
- Scoped generated1114symbols/555edges,8fresh isolated MCP symbol queries/34hierarchy queries;467проверок end coordinates по live code.908unrelated symbols/415edges/order unchanged; inherited external stale3endpoints/2sites явно сохранены в meta, не исправлены. Real Get3DInfo/GetComponent failure/deleted/multiple bodies, другие версии и полный TestKit/final runner не проверены; rejected Text/Label/Line enum при выборе negative fixture не доказывает SDK-ошибку reader. #252 OPEN, commit/push нет. Evidence: Hermes scratch/z5-docs-scoped/generated-audit.json. [MCP + live source joins; граница проверки]

> Карточка создана 2026-09-25 по коду ветки `llm_test` (HEAD `13c1948`). Модуль восстановлен в #205 (RoomBook) и #207/#208 (Spec); AC22–24 не поддерживаются — весь код под `#if defined(ServerMainVers_2500)`. Номера строк — определения в `.cpp`, 1-based (проверены grep/read). Своя справка по API: `Sources/AddOn/json_commands/How JSON Commands work.md` [из файла].
>
> Обновлено 2026-09-27 (#213): добавлена команда `SyncAll`; строка `Health`-решения сместилась в `How JSON Commands work.md` (:132 заголовок, :149 решение) [по коду].
>
> Обновлено 2026-09-27 (AC26, #170): в `CommandBase` добавлен `IsProcessWindowVisible` (метод `API_AddOnCommand` существует только с AC26, под `#ifdef ServerMainVers_2600`); номера строк `CommandBase.hpp/.cpp` ниже — после этой правки [по коду].

> Z3 #252, 2026-10-10, рабочее дерево поверх c323a2f: `RoomGeometryDiagnostic` только TESTING+AC25, расширенный material/profile export. Контракт и ограничения — в секции диагностики ниже; отдельная tests/TestRoombookGeometry.md отсутствует и не восстановлена. Координаты прочих команд не освежались. [по коду + fresh MCP + real JSON]

## Z4 #252 — TESTING-параметры RoomBook для готовых плоскостей (2026-10-11)

- Рабочее дерево поверх c323a2f, AC25 Windows. `RoomBookCommand::Execute` (:38) при TESTING+AC25 и наличии surfaceWalls вызывает `DrawRoomSurfaceWalls` (tests/TestRoombookGeometry.cpp:577); без поля обычный RoomBook сохранён. Schema остаётся NoValue, это экспериментальный явный вход, не production JSON-схема. [по коду + fresh MCP]
- Требуются zoneGuid существующей зоны и непустой surfaceWalls[]: parentGuid, begX/begY/endX/endY, zBottom, height, slantAngle (радианы), surface (индекс1..32767); favorite необязателен. Проверяются конечность, минимальная высота/длина, угол(0,π), чтение зоны/родителя и этажей. Реальный контакт/произвольные контуры и structural ownership этим входом не валидируются. [по коду]
- Необязательные boolean `isolateFinish` и `writeLinks` по умолчанию false. Первый создаёт/принимает только доступный новый слой отделки с группой0 и переводит на него новые стены, не меняет существующие слои. Второй использует прежнюю классификацию и `SyncSetSubelementScope(base element/zone, addToParent=false)→ElementsWrite` только новых GUID; полного SetSyncOtdWall/SyncArray нет. До записи проверяются property cache и наличие finish classification. [по коду]
- Ответ Created: createdCount/created[] (guid,parentGuid,zoneGuid,readError,type,layerIndex), linksAttempted. Echo GUID и linksAttempted не доказывают запись. InputError/ZoneReadError/StoryReadError/ParentReadError/InvalidEdge/PropertyDefinitionReadError/FinishClassificationNotFound/LayerCreateError/LayerReadError/WriterError возвращают createdCount0. WriterError не различает все внутренние этапы; Class_SetClass не возвращает SDK error, ElementsWrite result не агрегируется. [по коду]
- Linked-batch24/24 точных3D-тел,2наклонные,48/48 фактических GUID-свойств status=normal; отдельное повторное чтение24тел/48свойств прошло. Новый слой group0 устранил прежние7превышений, после корректного принятия коллинеарных вершин24кандидата/16явных пропусков. Max plane gap2.3820259009710298e−7м, symdiff1.622498112887025e−8м², пороги1e−6. Saved PLN hashes unchanged; source wall/zone before-after JSON equal, parent-column Empty не доказывает состояние сегментов. [real JSON + assertions + hashes]
- Обычный RoomBook не переведён на3D-путь; это частичная приёмка #252. Undo/Redo, production reader/material rules и оставшиеся structural joins not verified. AC25 canonical build APX43f5b1c9… загружен PID40136; diagnostic/command direct LSP0, Roombook прежние13. Generated1065symbols/522edges, inherited stale external endpoints явно отмечены. Runner/TestKit не заявляются; #252 OPEN, commit/push нет. Артефакты scratch/z4-native-finish/linked-batch. [граница проверки + build/MCP/LSP]

## Назначение

Подсистема HTTP/JSON-команд ArchiCAD (`API_AddOnCommand`): внешний агент (по HTTP-порту экземпляра Archicad) может вызвать модульные операции аддона — построение отделки (RoomBook), non-interactive построение спецификации (Spec) и полную синхронизацию (SyncAll). [по коду]

## Файлы

- `CommandBase.cpp/hpp` — базовые классы команд, общие хелперы ответов
- `RoomBookCommand.cpp/hpp` — команда `SomeStuffCommand.RoomBook` (#205)
- `SpecCommand.cpp/hpp` — команда `SomeStuffCommand.Spec` (#207/#208)
- `SyncAllCommand.cpp/hpp` — команда `SomeStuffCommand.SyncAll` (#213)
- `HealthCommand.cpp/hpp` — read-only шаблон `SomeStuffCommand.Health` (в регистратор НЕ включён, см. #205 / How JSON Commands work.md:149)
- `JsonCommandRegistrar.cpp/hpp` — `RegisterJsonCommands()`, вызывается из `Initialize` (SomeStuff_Main.cpp:569)
- `How JSON Commands work.md` — внутренняя справка [из файла]

## Ключевые типы

| Тип | Где | Назначение |
|-----|-----|-----------|
| `CommandBase` | CommandBase.hpp:17 | Базовый `API_AddOnCommand`: namespace `SomeStuffCommand` (CommandBase.cpp:8), политика `ScheduleForExecutionOnMainThread` (CommandBase.cpp:23) [по коду] |
| `ReadOnlyCommand` | CommandBase.hpp:68 | Подкласс без undo; унаследован только `HealthCommand` (не зарегистрирован) [по коду] |
| `ModifyCommand` | CommandBase.hpp:87 | Подкласс с поддержкой undo; в актуальном коде не используется — команды-изменения наследуют `CommandBase` напрямую [по коду] |
| `RoomBookCommand` | RoomBookCommand.hpp:7 | Команда без параметров, free-form ответ [по коду] |
| `SpecCommand` | SpecCommand.hpp:7 | Команда со схемой параметров (обязательный `placementPoint`) [по коду] |
| `SyncAllCommand` | SyncAllCommand.hpp:7 | Команда без параметров, free-form ответ (`SyncAll`) [по коду] |
| `HealthCommand` | HealthCommand.hpp:7 | Команда статуса/версии, read-only [по коду] |
| `CommonSchema` | CommandBase.hpp:15 | `Used`/`NotUsed` — режим общей схемы ответа; все актуальные команды используют `NotUsed` [по коду] |

## Публичный API

| Функция/метод | .cpp:строка | Назначение |
|---------------|-------------|-----------|
| `RegisterJsonCommands()` | JsonCommandRegistrar.cpp:20 | Регистрирует RoomBook/Spec/SyncAll через версионный SDK-вызов; дополнительно RoomGeometryDiagnostic под TESTING && ServerMainVers_2500 (:21–30), ошибка — DBprnt. Вне ServerMainVers_2500 пустая заглушка (:65). [по коду] |
| `CommandBase::GetNamespace` | CommandBase.cpp:18 | Возвращает `SomeStuffCommand` [из комментария] |
| `CommandBase::GetExecutionPolicy` | CommandBase.cpp:23 | `ScheduleForExecutionOnMainThread` — модифицирующие команды только в главном потоке [из комментария] |
| `CommandBase::IsProcessWindowVisible` | CommandBase.cpp:32 | AC26+ (`#ifdef ServerMainVers_2600`): сообщает Archicad о показе окна процесса — `true` (команды длительные, в главном потоке) [по коду; DevKit-пример AddOnCommandTest] |
| `RoomBookCommand::GetName` | RoomBookCommand.cpp:19 | Имя команды `RoomBook` [по коду] |
| `RoomBookCommand::Execute` | RoomBookCommand.cpp:34 | Запускает `Roombook::RoomBook()`; ответ `{status:"returned", elapsedSeconds}` — `void`-функция не сообщает ошибку/отмену [по коду; IDEA.md Decisions #205] |
| `SpecCommand::GetName` | SpecCommand.cpp:20 | Имя команды `Spec` [по коду] |
| `SpecCommand::GetInputParametersSchema` | SpecCommand.cpp:25 | JSON-схема: `placementPoint{x,y}` обязательна, `ruleNames[]` опциональна (minItems 1) [по коду] |
| `SpecCommand::Execute` | SpecCommand.cpp:57 | Без `placementPoint` — ошибка `APIERR_BADPARS`; иначе `LoadSyncSettingsFromPreferences` → `Spec::SpecAll(syncSettings, ruleNames?, &placementPoint, &runResult)`; ответ: `status` completed/failed, `resultCode`, счётчики `elementsToCreate/Modify/Delete`, `elapsedSeconds` [по коду] |
| `SyncAllCommand::GetName` | SyncAllCommand.cpp:21 | Имя команды `SyncAll` [по коду] |
| `SyncAllCommand::Execute` | SyncAllCommand.cpp:36 | Параметров нет; `LoadSyncSettingsFromPreferences(syncSettings, true)` → `PROPERTYCACHE().Update()` → `SyncAndMonAll(syncSettings)`; ответ `{status:"returned", elapsedSeconds}` — `void`-функция не сообщает ошибку/отмену/признак раннего выхода по `ResetProperty()` [по коду; #213] |
| `HealthCommand::Execute` | HealthCommand.cpp:34 | Ответ `{status:"ok", version: <из ID_ADDON_STRINGS VersionId>, addon:"SomeStuff"}` [по коду] |
| `CreateErrorResponse` / `CreateSuccessResponse` | CommandBase.cpp:58 / 68 | Свободные хелперы ответов (`error{code}`, `message` / `success:true`) [по коду] |
| `GetGuidFromObjectState` / `CreateGuidObjectState` (2 перегрузки) | объявлены в CommandBase.hpp:109-119 | Хелперы GUID-обёрток; определения не найдены в .cpp — [назначение не установлено: потенциально мёртвый код, не проверено] |

## Карточки

### `RegisterJsonCommands() -> void`
- Расположение: `Sources/AddOn/json_commands/JsonCommandRegistrar.cpp:20`
- Назначение: установка обработчиков RoomBook/Spec/SyncAll и дополнительной read-only RoomGeometryDiagnostic под TESTING && ServerMainVers_2500. Диагностический endpoint не запускается в TestKit и сам не создаёт отделку; production читает общий RoomSurfaces. [по коду]
- Контракт: вызывается один раз из `Initialize` (SomeStuff_Main.cpp:569), до `BrowserPalette::RegisterPaletteControlCallBack`. `#include "ACAPinc.h"` обязан предшествовать проверке версии (макрос даёт этот заголовок) — иначе пустая единица трансляции и ошибка на линковке [из How JSON Commands work.md:17-18].
- Побочные эффекты: регистрация HTTP-эндпоинтов на порту экземпляра Archicad; `DBprnt` при ошибке установки.
- Вызывает: SDK-регистрацию трёх обычных команд; под TESTING && ServerMainVers_2500 также регистрацию RoomGeometryDiagnostic. Для всех четырёх сохранена развилка имён SDK AC27+; DBprnt при отказах. [по коду]
- Вызывается из: `Initialize` (SomeStuff_Main.cpp:569). [grep]

### `SyncAllCommand::Execute(parameters, processControl) -> GS::ObjectState`
- Расположение: `Sources/AddOn/json_commands/SyncAllCommand.cpp:36`
- Назначение: запускает полную синхронизацию тем же `SyncAndMonAll`, что и пункт меню `SyncAll_CommandID` (без `DimRoundAll`, записи настроек и обновления меню). [по коду; #213]
- Контракт: параметров нет (`GetInputParametersSchema` — `NoValue`); `processControl` игнорируется — отмена не поддерживается. Ответ `{status:"returned", elapsedSeconds}` означает только возврат из `SyncAndMonAll` (`void`). Настройки перечитываются через `LoadSyncSettingsFromPreferences(syncSettings, true)`; состав обхода определяют флаги `wallS/widoS/objS/cwallS` из локального JSON-конфига. [по коду; #213]
- Побочные эффекты: `PROPERTYCACHE().Update()` (как в `MenuCommandHandler` перед вызовом команды меню); модификация БД внутри `ACAPI_CallUndoableCommand` самой `SyncAndMonAll`; append-записи в Report через `msg_rep` (окно не показывается — `ACAPI_WriteReport(msg, false)`). Ранний выход при `ResetProperty() == true` наружу не сигнализируется. [по коду]
- Вызывает: `LoadSyncSettingsFromPreferences` (dialogs/SyncSettings.cpp), `PROPERTYCACHE().Update()` (Propertycache.hpp), `SyncAndMonAll` (Sync.cpp:193). [grep]
- Вызывается из: диспетчеризации Archicad по имени команды (внешний HTTP-вызов); в коде вызовов нет. [по коду]
- Runtime: AC25, test_25.pln, порт 19723 — `{"succeeded":true,"result":{"addOnCommandResponse":{"status":"returned","elapsedSeconds":4.1818899}}}`; корректность синхронизации и отмена — not verified. AC26–29 — not verified.

### `RoomBookCommand::Execute(parameters, processControl) -> GS::ObjectState`
- Расположение: `Sources/AddOn/json_commands/RoomBookCommand.cpp:34`
- Назначение: запускает полное построение отделки без UI. [из комментария]
- Контракт: параметров нет (`GetInputParametersSchema` — `NoValue`); `processControl` игнорируется — отмена не поддерживается. Ответ `status:"returned"` означает только возврат из `RoomBook` (функция `void`) [по коду; IDEA.md Decisions #205].
- Побочные эффекты: модификация БД (создание/пересоздание элементов отделки) в главном потоке; длительность ~15–318 с (замеры runtime AC25: 15.09 с и 318.39 с, #205/#206).
- Вызывает: `Roombook::RoomBook` (Roombook.cpp, см. Docs/modules/Roombook.md).
- Вызывается из: диспетчеризации Archicad по имени команды (внешний HTTP-вызов); в коде вызовов нет. [по коду]

### `SpecCommand::Execute(parameters, processControl) -> GS::ObjectState`
- Расположение: `Sources/AddOn/json_commands/SpecCommand.cpp:57`
- Назначение: non-interactive построение спецификации: не открывает интерфейс, создаёт элементы по текущим правилам. [из комментария]
- Контракт: обязательный `placementPoint{x,y}` (тип числовой) — иначе `APIERR_BADPARS`; `ruleNames` (массив строк, minItems 1) опционален; передаётся в `Spec::SpecAll` вторым аргументом как `GS::Array<GS::UniString>*` или `nullptr`. Ответ: `status` completed/failed + `resultCode` (GSErrCode) + счётчики `elementsToCreate/elementsToModify/elementsToDelete` из `Spec::SpecRunResult` + `elapsedSeconds` [по коду].
- Побочные эффекты: `LoadSyncSettingsFromPreferences` — чтение настроек (свой путь, не диалог); `Spec::SpecAll` — создание/изменение/удаление элементов БД в главном потоке.
- Вызывает: `LoadSyncSettingsFromPreferences` (dialogs/SyncSettings.cpp), `Spec::SpecAll` (spec/Spec.cpp). [grep]
- Вызывается из: диспетчеризации Archicad по имени команды (внешний HTTP-вызов); в коде вызовов нет. [по коду]

## Зависимости

- `ACAPinc.h`, `api_headers/APIEnvir.h`, `ObjectState.hpp`, `OnExit.hpp` (CommandBase) [по include]
- `Roombook.hpp` (RoomBookCommand), `dialogs/SyncSettings.hpp`, `spec/Spec.hpp` (SpecCommand), `dialogs/SyncSettings.hpp`, `Propertycache.hpp`, `Sync.hpp` (SyncAllCommand), `CommonFunction.hpp`, `Constants.hpp`, `api_headers/ResourceIds.hpp` (HealthCommand/Registrator) [по include]

## Диагностика покрытия контактной поверхности — исторический этап Z3 (#252, AC25)

- Read-only `RoomGeometryDiagnostic`, TESTING+AC25; успешный geometry response имеет `schemaVersion=2`. Финальный `tests/TestRoombookGeometry.cpp`: `GetZone3DPolygons_StrictAPI` :429, `ReadPolygon` :354, `ReadModelMaterial` :64, `ReadProfile` :101, `ReadStructure` :230, `Inventory` :493, `Execute` :583. `readComplete` относится к geometry traversal, ошибки UMAT/атрибутов проверяются отдельно. [по коду + fresh clangd MCP]
- `polygon.material` сохраняет `pgon.iumat` — snapshot-local UMAT index, **не** surface/building-material attribute. `modelMaterials` разрешает его через `API_UmatID` → `umat.mater.head.index`;0 означает generated material. Экспортируются surface attribute/name и `error/attributeError`; `texture.fileLoc` освобождается после копирования как в official AC25 `3D_Test.c`. [код + AC25 headers/sample]
- `body.parentGuid`, owner/segment GUID, wall instance `stretchedProfile`, hatch GUID/`structureComponentId`, buildingMaterial/core/finish, raw coords/subPolyEnds/edge metadata сохраняются раздельно. `ReadProfile` использует базовый массив `GetProfileEdgeDataPtr() + i`, без неподтверждённого vertex→edge crosswalk. `ReadStructure` экспортирует skins/override flags/veneer; это не автоматический polygon→component join. Attribute profile — шаблон, не instance placement; assembly `stretchedProfiles` колонн в fixture пусты. [финальный код + real JSON]
- Python diagnostic ограничивает контакт конечными контурами/отверстиями с противоположными SDK-нормалями. Wall instance placement принят по3полным сечениям: единственный из±поперечных кандидатов прошёл symmetric-difference area gate. Это измерение fixture, не общая SDK-формула. Side ribbons hatches пересекаются с контактным фрагментом; resolved/unresolved/ambiguous площади разделены без snap/buffer/repair. [исполненный prototype, scratch/z3-material-20261010-200402]
- Runtime:37zone faces/72source faces/36fragments; все36surface attributes разрешены. Structural joins:13фрагментов обычных сегментов/6profile-wall fragments/16unresolved profile-column/1ambiguous wall. Компоненты0/1 различены при одинаковом surface5. Actual segment profile placement, general skin mapping, active overrides/veneer, curved/slanted/taper/end/junction handling и полный TestKit — not verified. Production Roombook не изменён. Отсутствующая `tests/TestRoombookGeometry.md` не восстановлена, контракт хранится здесь. [real JSON + assertions + scope limits]
- Fresh MCP:19diagnostic symbols/25edges сопряжены с финальным форматированным source;1045unrelated symbols/488edges и порядок сохранены. SDK/macro/full cross-TU completeness не заявляется. План `.hermes/plans/2026-10-03_112816-roombook-zone-surfaces-252.md`, текущий Z3 в IDEA.md и Docs/_progress.md. [MCP + source join]

## Инварианты и ограничения

- Все `.cpp` под `#if defined(ServerMainVers_2500)`; на AC22–24 `RegisterJsonCommands()` — пустая функция, модуль не компилируется. [по коду]
- Каждый JSON API HTTP-порт соответствует конкретному запущенному экземпляру Archicad — выбор «куда» делает вызывающий через порт, параметром не передаётся. [IDEA.md Decisions #205]
- Не реинтродуцировать JSON-команды «в ствол» через BrowserPalette-мост: мост и JSON-команды — разные каналы (мост = инлайн-функции `RegisterACAPIJavaScriptObject`), см. ARCHITECTURE.md §JS Bridge. [по коду]
- `HealthCommand` намеренно не регистрируется — остаётся шаблоном read-only команды (решение #205). [из How JSON Commands work.md:149]
- `SyncAllCommand` вызывает `SyncAndMonAll` as-is: прогресс-окно и фазы не подавляются и `Sync.cpp` не правится — осознанное решение #213, а не недоделка; не «оптимизировать» его без отдельной задачи. [по коду; #213]
- Конфликт имен: `CreateErrorResponse`/`CreateSuccessResponse` существуют и как свободные (CommandBase.cpp:58/68), и как protected-методы `ReadOnlyCommand`/`ModifyCommand` — дублирование API; актуальные команды используют свободные. [по коду]
- AC26 добавил в `API_AddOnCommand` чистый виртуальный `IsProcessWindowVisible` — без его реализации команды не инстанцируются (`GS::NewOwned<…>` в `JsonCommandRegistrar.cpp:17/22/27`); реализация — в `CommandBase` под `#ifdef ServerMainVers_2600`, AC25–24 её не компилируют. [по коду; сборка AC26]
