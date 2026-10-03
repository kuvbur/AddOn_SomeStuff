# Roombook — Спецификация отделки

> Основа: 1e67983 (2026-09-22); дополнение #195 — после 0cfe635; P3a #260 — d8fb525; P3b #260 — 9e041dc; P3c #260 — рабочее дерево после 29a6606 (2026-10-03, AC25). Номера строк остальных подсистем ниже относятся к основе и требуют обновления по живому коду. Назначения — по именам функций (при разборе модуля уточнять).

## Назначение
Спецификация отделки: генерация ведомостей отделочных материалов и работ из модели. [по коду]

## Точка входа
- `RoomBook` (Roombook.cpp:674; AC22–23 — заглушка :23) ← `MenuCommandHandler` (SomeStuff_Main.cpp; граф содержит старые номера строк) и `SomeStuffCommand.RoomBook` (JSON API AC25–29; вызывающий выбирает экземпляр Archicad по HTTP-порту). JSON-команда возвращает время до выхода из `RoomBook`; успех расчёта отдельно не подтверждается. [по коду / DevKit-25 API_AddOnCommand]
- `GetTargetZones` (Roombook.cpp:34) — выбор выделенных редактируемых зон или fallback на все доступные; возвращает false при ошибке/пустом списке [по коду]
- `PrepareRoomProcessingContext` (:92) — заполняет контекст: `Class_FindFinClass` → `GetStories` → `Floor_FindAll`; контейнеры живут до выхода из `RoomBook`. [по коду]
- `AdvanceProcessPhase` (:101) — единственная точка обновления прогресса: увеличивает `nPhase`, сообщает фазу и возвращает признак отмены; на AC27+ `ACAPI_ProcessWindow_SetNextProcessPhase`/`IsProcessCanceled`, иначе `ACAPI_Interface` с `APIIo_SetNextProcessPhaseID`/`APIIo_IsProcessCanceledID`. Раньше эти блоки были скопированы в девяти местах. `nPhase = 1` сбрасывается в `RoomBook` перед `ProcessWindowGuard`. [по коду]
- `BuildElementReadIndex` (:112), `ProcessElementsForRoomData` (:126) — индекс, прогресс/отмена, `ClearZoneGUID`, классификация и dispatch. [по коду]
- `PrepareReadParams` (:183), `ReadElementParameters` (:185) — подготовка и чтение параметров; `ReadParamsForRoomBook` хранит параметры окон и комнат. Временный `paramDict` создаётся в `ReadElementParameters`, очищается между типами и не хранится в `RoomProcessingContext`; `paramDict_favorite` остаётся в контексте. Порядок чтения не менялся. [по коду]
- `ProcessRoomFinishes` (:516) обходит комнаты; `ProcessSlabFinishes` (:209), `ProcessWallFinishes` (:289), `ApplyFavoriteAndMaterialData` (:272) сохраняют порядок создания/настройки отделки. [по коду]
- `BuildMaterialSummaryForRooms` (:380), `WriteRoomMaterialData` (:485) используют общий `MaterialSummary`; `paramToWrite` затем передаётся `SetSyncOtdWall`. [по коду]
- `RemoveUnusedFinishingElements` (:571), `PrepareElementsForUpdate` (:609) — сбор GUID на удаление и разблокировка/резервирование; отказ в резервировании завершает `RoomBook` до отрисовки. [по коду]
- Комментарий `REFACTOR PLAN` в Roombook.cpp:654-673 описывает этот рефакторинг. [из комментария]
- Исторический список вызовов из Docs/_generated/body_scan.json (96 до #195) устарел после извлечения helpers; актуальные вызовы сверять по `Sources/AddOn/Roombook.cpp`. Подсистемы: `Param_*`, `OtdData_*`, `OtdWall/OtdBeam/Opening/Floor_*`, `Class_*`, `Favorite_*`, Sync, PROPERTYCACHE и SDK ProcessWindow/Teamwork. [по коду; полный граф — не проверено]

## Функции по подсистемам [по коду / по имени]

### Зоны и сбор данных
`Otd_GetOtd_ByZone` (:763), `BuildOtdByParent` (:881), `Otd_GetOtd_Parent` (:922), `CollectRoomInfo` (:1211), `ClearZoneGUID` (:3399), `Edges_GetFromRoom` (:3330), `Edge_FindOnEdge` (:3949), `Edge_FindEdge` (:3980), `typeinzone`/`reducededges` — глобальные переменные namespace (:25/:28) [по коду]

`BuildOtdByParent` преобразует результат `SyncGetSubelement` из формы `GUID базового элемента -> GUID дочерней отделки` в индекс `база -> тип отделки -> GUID отделки`. Внешний ключ остаётся GUID базы, тип определяется по внутреннему GUID отделочного элемента; неизвестные дочерние GUID пропускаются. [по коду; #193]

### Данные отделки (OtdData)
`OtdData_GetColumnfFormat` (:817), `OtdData_CalcForRoom` (:908), `OtdData_WriteToRoom` (:1001), `OtdData_AddValueToDict` (:1143), `OtdWall_GetArea` (:1172)

### Создание элементов отделки
`OtdWall_Create_FromWall` (:1417), `OtdWall_Create_FromColumn` (:1676), `OtdWall_Add_One` (:1647), `Opening_Create_One` (:1368), `Opening_Add_One` (:1615), `OpeningReveals_Create_One` (:3437), `Floor_FindAll` (:1773), `Floor_FindInOneRoom` (:1819), `Floor_Create_All` (:1864), `Floor_Create_One` (:1917)

### Параметры (Param)
`Param_ToParamDict` (:2085), `Param_GetForWindowParams` (:2099), `Param_GetForBase` (:2151), `Param_GetForRooms` (:2295), `Param_Property_FindInParams` (:2533), `Param_Property_Read` (:2582), `Param_Material_Get` (:2619), `Param_SetToRooms` (:2647), `Param_SetToBase` (:2968), `Param_SetComposite` (:3031), `Param_SetToWindows` (:3145), `Param_AddUnicElementByType` (:5292), `Param_AddUnicGUIDByType` (:5320)

### Разделение стен и материалы
`OtdWall_Delim_All` (:3594), `OtdWall_Delim_One` (:3730), `SetMaterialByType` (:3799), `SetMaterialFinish_ByComposite` (:3909), `SetMaterialFinish` (:3920), `RoomRedProc` (:3274)

### Отрисовка (Draw) и дефолты
`Draw_Elements` (:3997), `OtdWall_Draw` (:4157), `OtdBeam_Draw_Beam` (:4186), `OtdBeam_Draw_Object` (:4258), `OtdWall_Draw_Object` (:4296), `OtdWall_Draw_Wall` (:4455), `Opening_Draw` (:4543), `Floor_Draw` (:4624), `Floor_Draw_Slab` (:4655), `Floor_Draw_Object` (:4756); дефолты: `OtdBeam_GetDefult_Beam` (:4239), `OtdBeam_GetDefult_Object` (:4269), `OtdWall_GetDefult_Object` (:4428), `OtdWall_GetDefult_Wall` (:4514), `Opening_GetDefult` (:4602), `Floor_GetDefult_Object` (:4901), `Floor_GetDefult_Slab` (:4925)

### Классификация отделки (Class)
`Class_SetClass` ×3 (:4952/4963/4976), `Class_GetClassGuid` (:4984), `Class_GetOtdTypeByClass` (:5031), `Class_FindFinClass` (:5075), `Class_IsElementFinClass` (:5277), `SetSyncOtdWall` (:5146)

### Избранное (Favorite)
`Favorite_GetType` (:5332), `Favorite_GetDict` (:5347), `Favorite_ReadComposite` (:5417), `Favorite_FindName` (:5454), `Favorite_GetByName` ×2 (:5554/5572)

### Прочее
`Check` (:5594, до :5685 — [назначение не установлено; вероятно, итоговая проверка])

## Зависимости
- `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp` [по include]

## Инварианты и подводные камни
- Утечка memo при ошибке GetMemo (:2028-2034) — **исправлена** (FIX 2026-09-12) [из комментария + проверено]
- AGENTS.md §16: recreation of finish elements — out of scope [из AGENTS.md]
- Глобальные переменные namespace (`min_dim` в ревью-заметках) — thread-safety ограничение [из ревью; не проверено в этой сессии]
- `reducededges` (:28) — адрес `RoomEdges` для callback `RoomRedProc`/`RoomReductionPolyProc`. Присваивается в `Edges_GetFromRoom` (:3403) перед вызовом SDK и обнуляется сразу после возврата (:3410); адрес локального `rdges` не переживает выход из функции. Владение глобалом соответствует контракту SDK: `RoomReductionPolyProc` — синхронный callback (`APIdefs_Callback.h`, AC25–29), пример DevKit использует тот же паттерн «указатель → вызов → чтение». Повторный вход в `Edges_GetFromRoom` до обнуления невозможен: вызов синхронный. [по коду + DevKit AC25–29 + LightRAG]
- Подсчёт пробелов/разделителей для выравнивания текста (`Roombook.cpp:1030`, `:1040`, `:1251`) с 2026-09-28 (#221) идёт через `CommonFunction::DoubleToInt32` — при выходе за диапазон Int32 подставляется граница и выводится сообщение `msg_rep`. [по коду]
## Версии
- AC27 (`#170`): материалы сегментов балки — `API_OverriddenAttribute` (`overridden` + `attributeIndex`) заменён на `APIOptional<API_AttributeIndex>` (`hasValue` + `value`, индекс через `ACAPI_CreateAttributeIndex`). [по коду DevKit + сборка AC27]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create`; в новом API нет параметра родительской группы, поэтому аддон передаёт только список элементов и выходной GUID. [по коду DevKit]
