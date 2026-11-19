# Roombook

> Модуль отделки зон: подготовка геометрии, расчёт и запись native-элементов.
> Состояние кода: `36feb07` + незакоммиченные изменения в `Sources/AddOn/roombook/`.
> Номера строк в карточке не держим: координаты символов живут только в `Docs/_generated/symbols.json`.
> Проверка актуальности: `python Tools/check_docs.py`.

## Назначение

`Roombook::RoomBook_Run` собирает данные зон и оснований, готовит отделку, создаёт/изменяет/переиспользует native-элементы и пишет связи с основаниями и зонами. До стадии `OtdRooms_Draw` расчёт в модель не пишет. [по коду]

## Файлы модуля

| Файл | Ответственность |
|---|---|
| `Sources/AddOn/roombook/Roombook.cpp` | Вход `RoomBook_Run`, применение сочетания слоёв `*smstf_fin`, выбор целевых зон, оркестрация конвейера |
| `Sources/AddOn/roombook/RoombookData.cpp` | Чтение зон/конструкций/параметров, `RoomProcessingContext`, снимок поверхностей и partition, словарь избранного, подготовка участков отделки |
| `Sources/AddOn/roombook/RoombookDraw.cpp` | Писатели native-элементов (Create/Change/Delete), переиспользование, классификация, связи, сводка |
| `Sources/AddOn/roombook/RoombookQuantities.cpp` | Сводка площадей/материалов и запись данных в комнаты |
| `Sources/AddOn/roombook/Roombook.hpp` | Типы `OtdRoom`/`OtdWall`/`OtdSlab`/`OtdBeam`, `RoomBookFinishResult`, снимки поверхностей `RoomSurface*`, объявления API модуля |

Вспомогательные заголовки `roombook/`: `CoordinateSearchCache.hpp`, `FloorCoreGeometry.hpp`, `RoomAnalyticGeometry.hpp`, `RoombookConstants.hpp`. Отдельных `RoomSurfaces*.hpp` нет: типы снимка поверхности объявлены в `Roombook.hpp`. [по include]

Зависимости: `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp`. [по include]

## Входные точки

| Вход | Цепочка |
|---|---|
| Меню | `MenuCommandHandler` → `Roombook::RoomBook_Run(results)` |
| JSON-команда `RoomBook` | `RoomBookCommand::Execute` → `Roombook::RoomBook_Run(results, zones)`; ответ сериализует `RoomBookFinishResult` |
| JSON-команда `FavoritesList` | `RoomBookGetFavoritesCommand::Execute` → `Roombook::Favorite_GetDict` |
| AC22/AC23 | заглушка `RoomBook_Run` — отчёт «Function not work in AC22 and AC23» |

## Выбор целевых зон

- `RoomBook_Run(results)` сохраняет прежний выбор: доступные выделенные зоны, иначе все доступные зоны. [по коду]
- `RoomBook_Run(results, zones)` использует переданный непустой список без чтения выделения; пустой список сохраняет автоматический выбор. Работа идёт по копии: Clear/Push не изменяют массив вызывающего. [по коду]
- JSON `SomeStuffCommand.RoomBook` принимает необязательный `zones: string[]`. Явный список должен быть непустым; до запуска проверяются формат GUID (с фигурными скобками или без), ненулевое значение, чтение заголовка, тип Zone и фильтры editable/visible/access/workspace/renovation. Дубли удаляются с сохранением порядка первого вхождения. Ошибка любого участника отклоняет весь запрос до запуска. [по коду]
- Ответ не изменён: `status: returned` означает возврат из void-функции, а не успешное построение всех элементов. Native-приёмка списка зон — [не проверено].

## Конвейер `RoomBook_Run`

| Шаг | Функция | Что делает |
|---|---|---|
| 1 | `LayerCombination_ApplyFinishIfExists` | Применяет сочетание слоёв с именем, оканчивающимся на `smstf_fin`; отсутствие такого сочетания не ошибка |
| 2 | `Zone_CollectTargets` | Только при пустом списке зон: выделенные, иначе все доступные с фильтром editable/visible/access/workspace/renovation; пустой результат прерывает прогон |
| 3 | `RoomProcessingContext_Prepare` | Сбор оснований, контактов поверхностей и параметров по всем зонам |
| 4 | `RoomProcessingContext_BuildElementReadIndex` | Индекс прочитанных элементов; временные GUID зон очищаются перед обработкой оснований |
| 5 | `RoomProcessingContext_ProcessElementsForRoomData`, `RoomProcessingContext_PreparePartitions` | Данные комнат и подготовка перегородок; любой `false` прерывает прогон |
| 6 | `RoomProcessingContext_ReadElementParameters` | Набор параметров и свойств к чтению |
| 7 | `ParamHelpers::ElementsRead` | Массовое чтение свойств элементов |
| 8 | `Favorite_GetDict` | Словарь избранного: выбор шаблона по материалу и типу поверхности |
| 9 | `Otd_GetOtd_ByZone` | Существующая отделка по связям zone (`Sync_GUID`) и индекс zone → основание → класс/роль |
| 10 | `OtdRooms_ProcessFinishes` | Геометрия, откосы, материалы, избранное; native-элементы ещё не создаются |
| 11 | `OtdData_AddRoomToMaterialSummary`, `OtdData_WriteRoomMaterialData` | Сводка по каждому `OtdRoom` с включённым построением; запись данных в комнаты |
| 12 | `Roombook_CheckFinishConfiguration` | Только если остались зоны: проверка классов и свойств перед записью; `false` прерывает |
| 13 | `OtdRooms_Draw` | Переиспользование, удаление неиспользованного, Create/Change по комнатам, связи, результаты |

## Карта вызовов

```
RoomBook_Run
├─ LayerCombination_ApplyFinishIfExists
├─ Zone_CollectTargets                        (только при пустом списке зон)
├─ RoomProcessingContext_Prepare
├─ RoomProcessingContext_BuildElementReadIndex
├─ RoomProcessingContext_ProcessElementsForRoomData
├─ RoomProcessingContext_PreparePartitions
├─ RoomProcessingContext_ReadElementParameters
├─ ParamHelpers::ElementsRead
├─ Favorite_GetDict
├─ Otd_GetOtd_ByZone
├─ OtdRooms_ProcessFinishes
├─ OtdData_AddRoomToMaterialSummary → OtdData_WriteRoomMaterialData
├─ Roombook_CheckFinishConfiguration          (только если остались зоны)
└─ OtdRooms_Draw
   ├─ RoofBatch_Validate → Roof_Prepare
   ├─ OtdRooms_AssignExistingWallFinishes → OtdWall_TakeExistingFinish   ← переиспользование
   ├─ PrepareElementsForUpdate
   ├─ ACAPI_Element_Delete (deletelist)
   ├─ OtdRoom_Draw                            (по каждой комнате)
   │  ├─ OtdWall_ShouldDrawFinish → OtdWall_Draw → Wall_Draw | Beam_Draw | Object_Draw_Linear
   │  ├─ OtdSlab_ShouldDrawFinish → OtdSlab_Draw → Roof_Draw | Slab_Draw | Object_Draw_Poly
   │  ├─ OtdBeam_Draw → Beam_Draw | Wall_Draw | Object_Draw_Linear
   │  ├─ Class_Set
   │  └─ Class_Set_OtdOpening
   ├─ CollectRoomFinishes
   └─ OtdElements_SetSync
```

Выбор варианта внутри `Otd*`-маршрутизаторов определяется типом шаблона избранного (`draw.favorite.type`). [по коду]

## Рождение, изменение, удаление

Все записи SDK сосредоточены в `RoombookDraw.cpp`. Шаблон каждого писателя:

1. Прочитать существующий элемент по GUID, если отделка уже существует.
2. Посчитать гвард: `Elems_Check_Update` (сравнение `floorInd`, `layer`, `renovationStatus` по маске) плюс собственные сравнения полей и memo.
3. Развилка: `is_new` → `ACAPI_Element_Create`; изменилось → `ACAPI_Element_Change` с маской; иначе no-op — GUID и успешный результат сохраняются, Change не вызывается.

| Тип отделки | Писатель | Изменение |
|---|---|---|
| Отделочная стена | `Wall_Draw` | Да, при расхождении гварда |
| Отделка-объект по линейному основанию | `Object_Draw_Linear` | Да |
| Линейная балка/плинтус | `Beam_Draw` | Да; мемо-сегменты входят в гвард |
| Отделочная плита | `Slab_Draw` | Да |
| Отделка-объект по полигону | `Object_Draw_Poly` | Да |
| Отделочная кровля | `Roof_WritePrepared` (подготовка `Roof_Prepare`) | Да |
| Проём в отделочной стене | `OtdOpening_Draw` | Нет: проём прорезается внутри стены; отдельным элементом не обновляется |

Удаления:

- `Wall_Draw` удаляет старые проёмы стены при её обновлении, до создания новых.
- Отказ проёма не удаляет существующую стену: требование отката передаётся общей undo-команде (`draw.require_rollback`). Удаляется только что созданная стена, если прорезать проём в ней не удалось. [по коду]
- `OtdRooms_Draw` удаляет `deletelist` — неиспользованную существующую отделку, собранную `OtdRooms_CollectUnusedFinishingElements` (сбор без удаления, удаление одним пакетом, `PrepareElementsForUpdate` перед ним).

## Повторное использование существующей отделки

- `OtdRooms_AssignExistingWallFinishes` назначает соответствия только окончательным участкам; один GUID не может использоваться в двух зонах.
- Пул кандидатов строится по зоне, основанию и классу/роли (`Class_GetClassGuid`, `Class_GetOtdTypeByClass`); уже использованные GUID и недоступные записи исключаются.
- `OtdWall_TakeExistingFinish` фильтрует совместимость: совпадение типа элемента, для стен — не `APIWtyp_Poly` и не `API_ProfileStructure`, для объекта — `libInd`, для балки — 1 сегмент / 1 схема / 2 среза, для кровли — плоская.
- Внутри пула выбирается **лексикографически минимальный GUID**; геометрия не сравнивается. [по коду]
- Найденный элемент идёт по ветке `ACAPI_Element_Change`, дубликат не создаётся.

## Классификация

| Функция | SDK-вызов | Вызывается из |
|---|---|---|
| `Class_Set` | `ACAPI_Element_AddClassificationItem` | `OtdRoom_Draw` (стены, плиты, балки) |
| `Class_Set_OtdOpening` | `ACAPI_Element_AddClassificationItem` | `OtdRoom_Draw` (проёмы) |

GUID класса выбирается словарём `Class_GetClassGuid`; для проёмов — класс NoSet, при его отсутствии Wall_Main.

## Финализация

- `CollectRoomFinishes` → `AppendRoomFinish` наполняет `GS::Array<RoomBookFinishResult>` по комнатам (стены, плиты, кровли, плинтусы); площади считает `OtdWall_GetArea`.
- `OtdElements_SetSync` пишет связи отделки с основаниями и зонами и общую запись накопленных свойств, включая сводку площадей.
- `RoomBookCommand::Execute` сериализует `RoomBookFinishResult` в JSON-ответ.

## Видимость оснований

- При обработке оснований зоны проверка видимости слоя и реконструкции выполняется перед проверкой класса отделки. Она применяется к кандидатам поверхностей, обработке связанных элементов, спискам Slab/Mesh и участкам балок. Участки балок дополнительно проверяются по отображению конструкций; класс сегмента проверяется отдельно. [по коду]
- Политика ошибок чтения классификации: в сборе кандидатов ошибка делает набор неполным, в legacy-проверке трактуется как отсутствие класса отделки. Путь generic Opening отдельно не изменён; полная видимость по нему в Archicad не проверена. [по коду]
- При пересчёте штатная очистка может удалить прежнюю отделку исключённого скрытого основания; владелец подтвердил такую политику. [решение владельца; по коду]

## Различия версий (текущее поведение)

- AC27+: переопределение материалов сегментов балки — `APIOptional<API_AttributeIndex>` (`hasValue`/`value`, индекс через `ACAPI_CreateAttributeIndex`); в младших версиях — `API_OverriddenAttribute`. [по коду DevKit]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create` (без параметра родительской группы). [по коду DevKit]
- AC26–30: `GetElemTypeID`/`SetElemTypeID` вместо прямого `typeID`; variation через `type` (AC26+). [по коду DevKit]

## Актуальность

- `Docs/_generated/symbols.json`: собран через clangd MCP; координаты — единственный источник номеров строк для этого модуля.
- `Docs/_generated/callgraph.json`: roombook-срез — [не проверено], собран до выделения `RoombookData.cpp`/`RoombookDraw.cpp` и к текущему коду не соответствует. Пересобрать перед использованием.
- `Docs/_generated/source_hashes.json`: хеши исходников для `Tools/check_docs.py`; обновляются одновременно с карточкой.
- История правок по issue (#294, #308, #314, #316, #317, #319–#322, #326, #327, #329–#336) — в `IDEA_ARCHIVE.md`, раздел «Roombook: истории правок по issue».
