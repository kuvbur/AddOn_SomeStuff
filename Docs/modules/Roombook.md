# Roombook

Модуль расчёта отделки: по зонам строит/обновляет отделочные элементы (стены, плиты, балки-плинтусы, объекты, кровли, проёмы), собирает сводку площадей и материалов, синхронизирует связи и свойства.

Строки ниже — 1-based, сверены с диском на момент коммита `b199125` (`[C3-WIP]` roombook-рефакторинг). Машинные данные: `Docs/_generated/symbols.json` (определения, 1-based) и `Docs/_generated/callgraph.json` (рёбра, метка `roombook_refresh` 2026-10-30).

## Файлы модуля

| Файл | Ответственность |
|---|---|
| `Sources/AddOn/roombook/Roombook.cpp` (201 стр.) | Вход `RoomBook_Run`, сбор зон, оркестрация конвейера |
| `Sources/AddOn/roombook/RoombookData.cpp` (9581 стр.) | Чтение зон/конструкций/параметров, `RoomProcessingContext`, словарь избранного, подготовка геометрии отделки |
| `Sources/AddOn/roombook/RoombookDraw.cpp` (2745 стр.) | Писатели native-элементов: Create/Change/Delete, классификация, синхронизация |
| `Sources/AddOn/roombook/RoombookQuantities.cpp` (584 стр.) | Сводка площадей/материалов, запись данных в комнаты |
| `Sources/AddOn/roombook/Roombook.hpp` (+5 .hpp) | Типы `OtdRoom/OtdWall/OtdSlab/OtdBeam`, `RoomBookFinishResult`, объявления API модуля |

Зависимости: `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp`; внутримодульные заголовки `roombook/` (`CoordinateSearchCache.hpp`, `FloorCoreGeometry.hpp`, `RoomAnalyticGeometry.hpp`, `RoomSurfaces.hpp`, `RoomSurfaceCandidates.hpp`, `RoomSurfacePartition.hpp`, `RoomSurfaceFinish.hpp`). [по include]

## Выбор целевых зон

- `RoomBook_Run(results)` сохраняет прежний выбор: доступные выделенные зоны, иначе все доступные зоны. [по коду]
- `RoomBook_Run(results, const GS::Array<API_Guid>& zones)` использует переданный непустой список без чтения выделения. Пустой список на C++-уровне сохраняет автоматический выбор. Внутри создаётся рабочая копия: последующие Clear/Push не изменяют массив вызывающего. [по коду]
- JSON `SomeStuffCommand.RoomBook` принимает необязательный `zones: string[]`. Явный список должен быть непустым; до запуска проверяются форма GUID (с парными фигурными скобками или без), ненулевое значение, чтение заголовка, тип Zone и прежние фильтры editable/visible/access/workspace/renovation. Дубли удаляются с сохранением порядка первого вхождения. Ошибка любого участника отклоняет весь запрос до запуска RoomBook; к выделению/всем зонам такой запрос не переходит. Проверка доступности выполняется в текущем виде до переключения сочетания слоёв конвейером. [по коду]
- Этот вход использует прежние подготовку, поиск существующей отделки по Sync_GUID zone и writers; формат ответа не изменён. `status: returned` по-прежнему означает возврат из void-функции, а не успешное построение всех элементов. Native-приёмка нового параметра — [не проверено].

## Входные точки

| Вход | Цепочка |
|---|---|
| Меню | `MenuCommandHandler` (`SomeStuff_Main.cpp:375`) → `Roombook::RoomBook_Run` (`SomeStuff_Main.cpp:458`) |
| JSON-команда `RoomBook` | `RoomBookCommand::Execute` (`json_commands/RoomBookCommand.cpp:65`) → `Roombook::RoomBook_Run(results, zones)` (`:97`, определение `Roombook.cpp:107`); результат `RoomBookFinishResult` сериализуется в прежний ответ |
| JSON-команда `FavoritesList` | `RoomBookGetFavoritesCommand::Execute` (`RoomBookCommand.cpp:105`) → `Roombook::Favorite_GetDict` (`:108`, def `RoombookData.cpp:8933`) |
| AC22/AC23 | Заглушка `RoomBook_Run` (`Roombook.cpp:21`) — отчёт «Function not work in AC22 and AC23» |

## Конвейер `RoomBook_Run` (Roombook.cpp:102)

| Шаг | Вызов (site) | Определение |
|---|---|---|
| 1. Применить сочетание слоёв отделки, если настроено | `Roombook.cpp:110` | `LayerCombination_ApplyFinishIfExists` (`Roombook.cpp:68`) |
| 2. Собрать целевые зоны (выделение или весь проект) | `Roombook.cpp:113` | `Zone_CollectTargets` (`Roombook.cpp:25`) |
| 3. Контекст обработки | `Roombook.cpp:120` | `RoomProcessingContext_Prepare` (`RoombookData.cpp:4810`) |
| 3а. Индекс прочитанных элементов | `Roombook.cpp:123` | `RoomProcessingContext_BuildElementReadIndex` (`RoombookData.cpp:4835`) |
| 3б. Обработать элементы для данных комнат | `Roombook.cpp:125` | `RoomProcessingContext_ProcessElementsForRoomData` (`RoombookData.cpp:4907`) |
| 3в. Подготовка перегородок | `Roombook.cpp:126` | `RoomProcessingContext_PreparePartitions` (`RoombookData.cpp:4973`) |
| 3г. Чтение параметров | `Roombook.cpp:130` | `RoomProcessingContext_ReadElementParameters` (`RoombookData.cpp:5028`) |
| 4. Прогресс/отмена | `Roombook.cpp:132,161` | `AdvanceProcessPhase` (`CommonFunction.cpp:2651`) |
| 5. Чтение всех элементов (мемо) | `Roombook.cpp:137` | `ElementsRead` (`Helpers.cpp:5583`) |
| 6. Словарь избранного | `Roombook.cpp:139` | `Favorite_GetDict` (`RoombookData.cpp:8933`) |
| 7. Отделка по зонам | `Roombook.cpp:144` | `Otd_GetOtd_ByZone` (`RoombookData.cpp:5888`) |
| 8. Расчёт полов/потолков/стен/откосов | `Roombook.cpp:147` | `OtdRooms_ProcessFinishes` (`RoombookData.cpp:5711`) |
| 9. Сводка материалов в комнаты | `Roombook.cpp:167,169` | `OtdData_AddRoomToMaterialSummary` (`RoombookQuantities.cpp:61`), `OtdData_WriteRoomMaterialData` (`RoombookQuantities.cpp:137`) |
| 10. Проверка конфигурации отделки | `Roombook.cpp:174` | `Roombook_CheckFinishConfiguration` (`RoombookData.cpp:9448`) |
| 11. Рождение/изменение/удаление элементов | `Roombook.cpp:178` | `OtdRooms_Draw` (`RoombookDraw.cpp:2670`) |

## Карта вызовов: от входа до писателей

```
RoomBook_Run (Roombook.cpp:102)
├─ подготовка данных — см. конвейер выше
└─ OtdRooms_Draw (RoombookDraw.cpp:2670)                        [site Roombook.cpp:178]
   ├─ RoofBatch_Validate (RoombookDraw.cpp:2007)                 [site 2683]
   │  └─ Roof_Prepare (RoombookDraw.cpp:1625) → OtdSlab_SetRoofEdgeTrims (1463)
   ├─ OtdRooms_AssignExistingWallFinishes (RoombookDraw.cpp:444) [site 2689]  ← повторное использование
   │  └─ OtdWall_TakeExistingFinish (RoombookDraw.cpp:397)
   ├─ PrepareElementsForUpdate (CommonFunction.cpp:2511)         [site 2705]
   ├─ ACAPI_Element_Delete (deletelist)                          [site 2709]
   ├─ OtdRoom_Draw (RoombookDraw.cpp:2535)                       [site 2723]  ← на каждую комнату
   │  ├─ OtdWall_ShouldDrawFinish (373) → OtdWall_Draw (2499)    [site 2548]
   │  │  └─ Wall_Draw (1183) | Beam_Draw (764) | Object_Draw_Linear (998)   [sites 2509/2512/2515]
   │  ├─ OtdSlab_ShouldDrawFinish (355) → OtdSlab_Draw (2425)    [site 2576]
   │  │  └─ Roof_Draw (1950) | Slab_Draw (2038) | Object_Draw_Poly (2261)   [sites 2442/2446/2454]
   │  ├─ OtdBeam_Draw (2471)                                     [sites 2592/2603]
   │  │  └─ Beam_Draw (764) | Wall_Draw (1183) | Object_Draw_Linear (998)   [sites 2478/2481/2484]
   │  ├─ Class_Set (80)                                          [sites 2566/2587/2598/2609]
   │  └─ Class_Set_OtdOpening (95)                               [site 2568]
   ├─ CollectRoomFinishes (2654)                                 [site 2730]
   └─ OtdElements_SetSync (645)                                  [site 2742]
```

## Рождение, изменение, удаление отделочных элементов

Все записи SDK сосредоточены в `RoombookDraw.cpp`. Шаблон каждого писателя:

1. Прочитать существующий элемент по GUID, если отделка уже существует;
2. посчитать гвард `needupdate` — `Elems_Check_Update` (`RoombookDraw.cpp:27`: сравнение `floorInd`, `layer`, `renovationStatus` по маске) плюс собственные сравнения полей и memo (геометрия, размеры, материалы, сегменты);
3. развилка: `is_new` → `ACAPI_Element_Create`; `needupdate` → `ACAPI_Element_Change` с маской; иначе — no-op: GUID и успешный результат сохраняются, Change не вызывается.

| Тип отделки | Писатель (def) | `ACAPI_Element_Create` | Гвард / `ACAPI_Element_Change` |
|---|---|---|---|
| Отделочная стена | `Wall_Draw` (`RoombookDraw.cpp:1183`) | `:1244` | гвард `:1288`, сравнения и маски до `:1413`, Change `:1415` |
| Отделка-объект по линейному основанию | `Object_Draw_Linear` (`:998`) | `:1153` | гвард `:1133`, Change `:1159` |
| Линейная балка/плинтус | `Beam_Draw` (`:764`) | `:902` | гвард `:799` (вкл. `begC/endC/level`, материалы, memo-сегменты `:801-897`), Change `:904` |
| Отделочная плита | `Slab_Draw` (`:2038`) | `:2186` | гвард `:2066`, Change `:2229` |
| Отделка-объект по полигону | `Object_Draw_Poly` (`:2261`) | `:2403` | гвард `:2394`, Change `:2407` |
| Отделочная кровля | `Roof_WritePrepared` (`:1731`, подготовка `Roof_Prepare` `:1625`) | `:1733` | гвард `:1779`, Change `:1942` |
| Проём в отделочной стене | `OtdOpening_Draw` (`:264`) | `:309` | изменение не выполняется — проём создаётся внутри новой стены; при неудаче прорезания стена удаляется целиком (`:1434`) |

Выбор варианта построения внутри `Otd*`-маршрутизаторов определяется типом шаблона избранного (`draw.favorite.type`): например, `OtdWall_Draw` → `Wall_Draw` / `Beam_Draw` / `Object_Draw_Linear` (sites 2509/2512/2515), `OtdSlab_Draw` → `Roof_Draw` / `Slab_Draw` / `Object_Draw_Poly` (sites 2442/2446/2454).

Удаления:

| Сайт | Что удаляется |
|---|---|
| `Wall_Draw` (`RoombookDraw.cpp:1280`) | старые проёмы (`oldOpenings`) при обновлении отделочной стены |
| `Wall_Draw` (`:1434`) | только что созданная стена при неудачном прорезании проёма («Непрорезанная оболочка даёт лишнюю отделку; удаляем только новый результат») |
| `OtdRooms_Draw` (`:2709`) | `deletelist` — неиспользованная существующая отделка, собранная `OtdRooms_CollectUnusedFinishingElements` (`:319`; сбор без удаления, удаление одним пакетом) |

## Повторное использование существующей отделки

- `OtdRooms_Draw` → `OtdRooms_AssignExistingWallFinishes` (`RoombookDraw.cpp:444`): соответствия назначаются только окончательным участкам; один GUID нельзя использовать в двух зонах;
- → `OtdWall_TakeExistingFinish` (`:397`): существующий отделочный элемент переводится в состояние повторного использования; применимость проверяет `OtdLinearElement_CanDraw` (`:722`);
- после назначения писатель идёт по ветке `ACAPI_Element_Change` (гварды выше), а не создаёт дубликат.

## Классификация

| Функция | SDK-вызов | Вызывается из |
|---|---|---|
| `Class_Set` (`RoombookDraw.cpp:80`) | `ACAPI_Element_AddClassificationItem` `:87` | `OtdRoom_Draw` (sites 2566/2587/2598/2609) |
| `Class_Set_OtdOpening` (`:95`) | `:102` | `OtdRoom_Draw` (site 2568) |

GUID класса выбирается словарём `Class_GetClassGuid` (`RoombookData.cpp:8647`); для проёмов — класс NoSet, при его отсутствии Wall_Main.

## Финализация

- `CollectRoomFinishes` (`RoombookDraw.cpp:2654`) → `AppendRoomFinish` — наполняет `GS::Array<RoomBookFinishResult>` по комнатам (стены, плиты, кровли, плинтусы; площади через `OtdWall_GetArea`, `RoombookQuantities.cpp:545`);
- `OtdElements_SetSync` (`RoombookDraw.cpp:645`) — связи отделки с основаниями и зонами, общая запись накопленных свойств, включая сводку площадей;
- `RoomBookCommand::Execute` сериализует `RoomBookFinishResult` в JSON-ответ.

## Различия версий (текущее поведение)

- AC27+: переопределение материалов сегментов балки — `APIOptional<API_AttributeIndex>` (`hasValue`/`value`, индекс через `ACAPI_CreateAttributeIndex`); в старших версиях — `API_OverriddenAttribute`. [по коду, `RoombookDraw.cpp:890-898`]
- AC27: `ACAPI_ElementGroup_Create` → `ACAPI_ElementSet_Create` (параметра родительской группы нет — передаётся только список и выходной GUID). [по коду DevKit]
- AC26–30: `GetElemTypeID`/`SetElemTypeID` вместо прямого `typeID`; variation через `type` (AC26+). [по коду DevKit]

## Актуальность

- `Docs/_generated/symbols.json`: roombook-файлы пересобраны 2026-10-30 (дисковые строки определений), остальные модули — прогоны 2026-10-15/30.
- `Docs/_generated/callgraph.json`: roombook-срез (807 рёбер) пересобран grep-сканом определений и call-сайтов после `b199125`; входные рёбра из `MenuCommandHandler` и JSON-команд добавлены явно. Остальные срезы — предыдущего прогона (см. `meta`).
