# Roombook — Спецификация отделки

> Основа: 1e67983 (2026-09-22); дополнение #195 — после 0cfe635; P3a #260 — d8fb525; P3b #260 — 9e041dc; P3c #260 — рабочее дерево после 29a6606 (2026-10-03, AC25). P3d #260 — рабочее дерево поверх 6231b74 (2026-10-03, AC25). Номера строк остальных подсистем ниже относятся к основе и требуют обновления по живому коду. Назначения — по именам функций (при разборе модуля уточнять).

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
- Правка P3d: в теле `RoomBook` (:609) сняты пять `auto &`-алиасов на поля контекста
  (`storyLevels`, `deletelist`, `exsistot_byzone`, `roomsinfo`, `paramDict_favorite`) — обращения идут
  через `context.<поле>`. Оставшиеся алиасы (`finclass`, `finclassguids`, `paramToRead`) сохранены:
  поля используются в нескольких соседних вызовах. Порядок операций не менялся. [по коду]
- Исторический список вызовов из Docs/_generated/body_scan.json (96 до #195) устарел после извлечения helpers; актуальные вызовы сверять по `Sources/AddOn/Roombook.cpp`. Подсистемы: `Param_*`, `OtdData_*`, `OtdWall/OtdBeam/Opening/Floor_*`, `Class_*`, `Favorite_*`, Sync, PROPERTYCACHE и SDK ProcessWindow/Teamwork. [по коду; полный граф — не проверено]

## Функции по подсистемам [по коду / по имени]

### Зоны и сбор данных
`Otd_GetOtd_ByZone` (:758), `BuildOtdByParent` (:876), `Otd_GetOtd_Parent` (:917), `CollectRoomInfo` (:1206), `ClearZoneGUID` (:3394), `Edges_GetFromRoom` (:3325), `Edge_FindOnEdge` (:3944), `Edge_FindEdge` (:3975), `typeinzone`/`reducededges` — глобальные переменные namespace (:25/:28) [по коду]

`BuildOtdByParent` преобразует результат `SyncGetSubelement` из формы `GUID базового элемента -> GUID дочерней отделки` в индекс `база -> тип отделки -> GUID отделки`. Внешний ключ остаётся GUID базы, тип определяется по внутреннему GUID отделочного элемента; неизвестные дочерние GUID пропускаются. [по коду; #193]

### Данные отделки (OtdData)
`OtdData_GetColumnfFormat` (:812), `OtdData_CalcForRoom` (:903), `OtdData_WriteToRoom` (:996), `OtdData_AddValueToDict` (:1138), `OtdWall_GetArea` (:1167)

### Создание элементов отделки
`OtdWall_Create_FromWall` (:1412), `OtdWall_Create_FromColumn` (:1671), `OtdWall_Add_One` (:1642), `Opening_Create_One` (:1363), `Opening_Add_One` (:1610), `OpeningReveals_Create_One` (:3432), `Floor_FindAll` (:1768), `Floor_FindInOneRoom` (:1814), `Floor_Create_All` (:1859), `Floor_Create_One` (:1912)

### Параметры (Param)
`Param_ToParamDict` (:2080), `Param_GetForWindowParams` (:2094), `Param_GetForBase` (:2146), `Param_GetForRooms` (:2290), `Param_Property_FindInParams` (:2528), `Param_Property_Read` (:2577), `Param_Material_Get` (:2614), `Param_SetToRooms` (:2642), `Param_SetToBase` (:2963), `Param_SetComposite` (:3026), `Param_SetToWindows` (:3140), `Param_AddUnicElementByType` (:5287), `Param_AddUnicGUIDByType` (:5315)

### Разделение стен и материалы
`OtdWall_Delim_All` (:3589), `OtdWall_Delim_One` (:3725), `SetMaterialByType` (:3794), `SetMaterialFinish_ByComposite` (:3904), `SetMaterialFinish` (:3915), `RoomRedProc` (:3269)

### Отрисовка (Draw) и дефолты
`Draw_Elements` (:3992), `OtdWall_Draw` (:4152), `OtdBeam_Draw_Beam` (:4181), `OtdBeam_Draw_Object` (:4253), `OtdWall_Draw_Object` (:4291), `OtdWall_Draw_Wall` (:4450), `Opening_Draw` (:4538), `Floor_Draw` (:4619), `Floor_Draw_Slab` (:4650), `Floor_Draw_Object` (:4751); дефолты: `OtdBeam_GetDefult_Beam` (:4234), `OtdBeam_GetDefult_Object` (:4264), `OtdWall_GetDefult_Object` (:4423), `OtdWall_GetDefult_Wall` (:4509), `Opening_GetDefult` (:4597), `Floor_GetDefult_Object` (:4896), `Floor_GetDefult_Slab` (:4920)

### Классификация отделки (Class)
`Class_SetClass` ×3 (:4952/4963/4976), `Class_GetClassGuid` (:4979), `Class_GetOtdTypeByClass` (:5026), `Class_FindFinClass` (:5070), `Class_IsElementFinClass` (:5272), `SetSyncOtdWall` (:5141)

### Избранное (Favorite)
`Favorite_GetType` (:5327), `Favorite_GetDict` (:5342), `Favorite_ReadComposite` (:5412), `Favorite_FindName` (:5449), `Favorite_GetByName` ×2 (:5554/5572)

### Прочее
`Check` (:5594, до :5685 — [назначение не установлено; вероятно, итоговая проверка])

## Зависимости
- `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp` [по include]

## Инварианты и подводные камни
- Утечка memo при ошибке GetMemo (:2028-2034) — **исправлена** (FIX 2026-09-12) [из комментария + проверено]
- AGENTS.md §16: recreation of finish elements — out of scope [из AGENTS.md]
- Глобальные переменные namespace (`min_dim` в ревью-заметках) — thread-safety ограничение [из ревью; не проверено в этой сессии]
- `reducededges` (:28) — адрес `RoomEdges` для callback `RoomRedProc`/`RoomReductionPolyProc`. Присваивается в `Edges_GetFromRoom` (:3398) перед вызовом SDK и обнуляется сразу после возврата (:3410); адрес локального `rdges` не переживает выход из функции. Владение глобалом соответствует контракту SDK: `RoomReductionPolyProc` — синхронный callback (`APIdefs_Callback.h`, AC25–29), пример DevKit использует тот же паттерн «указатель → вызов → чтение». Повторный вход в `Edges_GetFromRoom` до обнуления невозможен: вызов синхронный. [по коду + DevKit AC25–29 + LightRAG]
- Подсчёт пробелов/разделителей для выравнивания текста (`Roombook.cpp:1030`, `:1040`, `:1251`) с 2026-09-28 (#221) идёт через `CommonFunction::DoubleToInt32` — при выходе за диапазон Int32 подставляется граница и выводится сообщение `msg_rep`. [по коду]
## Версии
- AC27 (`#170`): материалы сегментов балки — `API_OverriddenAttribute` (`overridden` + `attributeIndex`) заменён на `APIOptional<API_AttributeIndex>` (`hasValue` + `value`, индекс через `ACAPI_CreateAttributeIndex`). [по коду DevKit + сборка AC27]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create`; в новом API нет параметра родительской группы, поэтому аддон передаёт только список элементов и выходной GUID. [по коду DevKit]
