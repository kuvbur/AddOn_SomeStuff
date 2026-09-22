# Roombook — Спецификация отделки

> Хеш коммита: 1e67983 (2026-09-22). Полный перечень функций из clangd documentSymbols (namespace Roombook, 5686 строк, ~70 функций); строки — .cpp (1-based). Назначения — по именам функций (комментарии к большинству отсутствуют; при разборе модуля уточнять).

## Назначение
Спецификация отделки: генерация ведомостей отделочных материалов и работ из модели. [по коду]

## Точка входа
- `RoomBook` (:57, до :660 — оркестратор, ~600 строк) ← `MenuCommandHandler` (SomeStuff_Main.cpp:457) [из callgraph.json]
- Комментарий в шапке: цель рефакторинга — «thin orchestrator with small single-purpose helpers» (Roombook.cpp:56-115) [из комментария]
- Вызывает (полный список — Docs/_generated/body_scan.json, 96 вызовов): подсистемы `Param_*` (Get/Set для комнат/базы/окон/состава), `OtdData_*`, `OtdWall/OtdBeam/Opening/Floor_*_Draw` + `_GetDefult`, `Class_*`, `Favorite_*`, создание (`OtdWall_Create_FromColumn`, `Opening_Create_One`, `Floor_Create_All/One`, `Floor_FindAll/InOneRoom`, `ClearZoneGUID`, `Edges_GetFromRoom`, `Draw_Elements`, `SetMaterial*`, `SetSyncOtdWall`, `SetElemTypeID`, `UnhideUnlockElementLayer`), Sync (`SyncArray`, `SyncSetSubelementScope`, `LoadSyncSettingsFromPreferences`, `GetRelationsElement`), PROPERTYCACHE; SDK: Element_Create/Change/Delete/Group/Classification, Teamwork Reserve, Grouping, ProcessWindow (отмена). [по коду, grep тела]

## Функции по подсистемам [по коду / по имени]

### Зоны и сбор данных
`Otd_GetOtd_ByZone` (:662), `Otd_GetOtd_Parent` (:742), `CollectRoomInfo` (:1211), `ClearZoneGUID` (:3399), `Edges_GetFromRoom` (:3330), `Edge_FindOnEdge` (:3949), `Edge_FindEdge` (:3980), `typeinzone`/`reducededges` — глобальные переменные namespace (:25/:28) [по коду]

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
- Глобальные переменные namespace (`reducededges`, `min_dim` в ревью-заметках) — thread-safety ограничение [из ревью; не проверено в этой сессии]
