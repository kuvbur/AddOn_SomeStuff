# Roombook

## Неблокирующее разбиение поверхностей (#322)

Контракт AC25 Windows/C++14, рабочая дельта к `a3dc151`, 2026-11-11. [по коду]

- `RoomSurfaceFace_Partition` (`RoombookData.cpp:884`) при отказе проекции отдельного источника сохраняет ошибку и продолжает остальные контакты. Исключение GS при проекции/Boolean не публикует частичный `next`: остаётся прежнее покрытие `cells`. Отказ исходной грани зоны или проверки покрытия по-прежнему исключает эту грань. Допуски не изменены. [по коду]
- `RoomSurfaces_Partition` (`RoombookData.cpp:953`) сохраняет пригодные участки при ошибках других источников/граней. Конфликтующая группа одного face-key исключается целиком; одинаковые дубликаты остаются одним claim. Каждая грань зоны имеет отдельный временный результат и catch: исключение не обрывает соседние грани и не публикует половину грани. `complete=true` только при непустом результате без ошибок; `complete=false` не очищает пригодные `pieces`. [по коду]
- `RoomProcessingContext_PreparePartitions` (`RoombookData.cpp:5005`) выводит каждую `errors` через `msg_rep` с этапом, кодом и GUID зоны. Если результат неполон без конкретной ошибки, выводит общее предупреждение. Неполнота partition больше не возвращает false для всего RoomBook; отдельный отказ `Element_Read` оставлен прежним. API записи и другие gates подготовки/геометрии не меняются. [по коду]
- Ограничение: штатный cleanup существующей отделки не изменён. Прежняя отделка пропущенного участка может оказаться неиспользованной и удалиться; сохранность таких GUID этой правкой не гарантируется. Сводка остаётся расчётной по доступному набору, а не подтверждением полноты или успеха native-записи. [по коду: `RoombookDraw.cpp:3008–3037`]
- Регрессия с реальными SDK25 Geometry/GSRoot и дословными helpers/partition/PreparePartitions: RED 71/24 → GREEN 71/0; preparation/report и исключение отдельной грани — controlled seams. Семь новых постоянных assertions из TestRoombookGeometry отдельно извлечены и исполнены на том же SDK: RED 7/7 → GREEN 7/0. Полный test TU с `TESTING`, C++14, `/WX` скомпилирован; canonical AC25 Windows RelWithDebInfo INT build успешен, Data перекомпилирован и APX перелинкован. Исходная зона с детализированным профилем и native-запись не запускались. [выполненный standalone/build; native не проверено]
- Свежие isolated LSP `publishDiagnostics(version=1)`: Data имеет те же три unused-variable и пять unused-includes, что сохранённый baseline; тест без диагностик, заголовок проверен в контексте обеих TU. Независимое ревью: No issues found in inspected scope. В symbols.json обновлены лишь три изменённые production-функции по свежим documentSymbol; две test-функции сверены без изменения координат. Остальные исторические координаты и callgraph не актуализировались. [LSP/source/hash checks, read-only review]

## Свойства плинтусов на отделочных полах и потолках (#308)

Контракт AC25 Windows, рабочая дельта к `00c6dcc`. [по коду]

- `OtdSlab_WriteSkirt` (`RoombookDraw.cpp:2931`) добавляет значения в общий `paramToWrite` только для валидного OtdSlab с ненулевым GUID и `draw.write_succeeded=true`; успешный no-op допускается. Успех native-writers самих плинтусов не участвует в подсчёте: `length_skirting` остаётся длиной подготовленных отрезков в метрах. [по коду]
- Определения выбираются из PROPERTYCACHE по описаниям `some_stuff_element_tip_skirting` и `some_stuff_element_length_skirting`. Два локальных ParamValue кэшируют поиск в рамках OtdRooms_Draw; `rawName=" "` обозначает отсутствие определения. Найденные свойства обрабатываются независимо, фиктивный ключ в очередь не добавляется. [по коду]
- Длина записывается и при нулевом значении; при `is_equal(length_skirting, 0)` тип очищается. Пустой тип сам по себе не запрещает запись ненулевой длины. Повторные значения заменяются штатным AddParamValue2ParamDictElement, определения в PROPERTYCACHE не изменяются. [по коду]
- Преобразование — ConvertDoubleToParamValue / ConvertStringToParamValue; фактическая запись остаётся в OtdElements_SetSync → ElementsWrite → WriteProperty после успешной draw-транзакции. Прямой записи свойств или GDL в новом helper нет. Фактическое сохранение и повтор в Archicad — [не проверено].

## Безопасный отказ Wall writer (#317)

Адресный контракт AC25 Windows, дельта к `e0dfc7d`, 2026-11-08. Остальные исторические координаты ниже не объявляются актуальными. [по коду]

- `OtdDrawData.require_rollback` — внутренний default-false маркер текущей попытки, не JSON-поле. `Wall_Draw` (`RoombookDraw.cpp:1200`), `OtdWall_Draw` (`:2703`) и `OtdBeam_Draw` (`:2673`) сбрасывают его и флаги успешной записи на входе. [по коду]
- Для существующей стены `Wall_Draw` ставит маркер до Delete старых managed-проёмов, Change стены и попытки создать каждый новый hosted-проём. При последующем отказе маркер сохраняется; существующая родительская стена не удаляется для cleanup. Полный успешный выход, включая no-op, явно выставляет `NoError`, `write_succeeded=true`, `was_created=false` и очищает маркер. [по коду]
- Для новой стены отказ проёма сохраняет локальный cleanup: успешное удаление очищает GUID стены/проёмов и позволяет продолжение; отказ удаления передаёт `deleteErr` и требует общего отката. [по коду]
- `OtdRoom_Draw` (`:2745`) возвращает ненулевой `draw_error` при установленном маркере в циклах стен, потолочных и напольных плинтусов. Остальные writer-ошибки сохраняют прежний continue. `OtdRooms_Draw` (`:2905`) передаёт ошибку через существующий undo callback и не публикует pendingResults/Sync при отказе команды. [по коду]
- Защитные выходы после успешного Get (несовместимый тип/Profile/Poly, чужие hosted-элементы) остаются до разрушительной записи, но теперь явно имеют `APIERR_BADPARS`, не ложный успех. Геометрия, отбор чужих проёмов, R1/#316 и неблокирующие ошибки Roof/#321 не меняются. [по коду]
- SDK25: сигнатуры и undo-scope проверены по установленным ACAPinc.h и HTML; пример `Element_Test/Src/Element_Hotlink.cpp::Do_ChangeHotlink` возвращает ошибку callback для отмены предыдущих изменений. Восстановление конкретной стены/проёмов в Archicad и загрузка нового APX — [не проверено]. In-memory подготовленные GUID при откате не восстанавливаются; прерванный пакет не публикуется как успешный.

## Локальный отказ Mesh-пола / Roof (#321)

- `RoofBatch_Validate` проверяет каждый включённый Mesh-пол отдельно, сбрасывает флаги записи и сохраняет ошибку в `draw.draw_error`; отказ одного участка не завершает общий draw. Неудачный preflight пропускается в matcher и writer, но остаётся в полном JSON-наборе с `write_succeeded=false`. Геометрическая `isValid` не подменяется результатом записи. [по коду]
- `Roof_Prepare` не меняет применимость выбранной многослойки. Если отсутствует `APICWall_ForRoof`, сообщение называет избранное и индекс многослойки и предлагает включить её применение для крыш. Состав не заменяется на Basic, пользовательский атрибут автоматически не редактируется. В SDK25 `APICWall_ForSlab=2`, `APICWall_ForRoof=4` — независимые флаги. [по коду и установленному SDK25]
- `OtdRoom_Draw` после ошибки Roof продолжает остальные плиты, плинтусы и комнаты; не добавляет неудачному результату связи, классификацию или группу. Ошибка чтения старого Roof в matcher также остаётся локальной для Mesh-записи. Общие ошибки удаления, подготовки/отмены и группировки не превращены в успех. [по коду]
- `RoofBatch_CollectUnused` собирает остаток существующей отделки по `(zone, base, role)`, включая прежний `NoSet` fallback. Его удаление отложено до завершения writers: при любом отказе Mesh-участка того же набора остаток сохраняется; при полном успехе удаляется штатно. Переиспользованные GUID уже исключены matcher из всех пулов и не попадают в этот остаток. Обычная отделка сохраняет раннее удаление. [по коду]
- Сводка материалов по-прежнему расчётная и подготовлена до writers; локальная ошибка не превращает её в обмер успешно созданных элементов. `write_succeeded/draw_error` описывают результат записи отдельно. [по коду]
- Native-диагноз AC25 Windows: после изменения владельцем помещения 113, `Roof_Prepare` остановлен перед возвратом на проверке composite153, `flags=2`, Roof-бит отсутствует; API Create ещё не вызван. Прежний глобальный preflight возвращал пустой `rooms`. Это настройка входных данных и отдельная ошибка блокирующей оркестрации, не доказанный дефект геометрии/SDK. [живой отладчик, 2026-11-07]
- AC25 Windows Debug: canonical `BuildAddOn.py -c config.json -v 25` успешен, тот же APX загружен через F5; SHA256 source/APX/обоих сохранённых PLN сверены до/после запуска. Повтор зоны 113 вместо прежнего `rooms=[]` вернул 26 результатов: 22 успешных и 4 неудачных Roof; все четыре прежних Roof GUID сохранились. Полный прогон завершил 25 зон: 1172 результата, 1168 успешных и те же 4 отказа только в 113; successful GUID прочитаны из native inventory, обработаны остальные 24 зоны. Это проверка неблокирующего отказа подготовки, не инъекция ошибок native Create/Change и не приёмка всей геометрии. Модель изменена только в памяти, Save не выполнялся. [native AC25 Windows, 2026-11-07]
- Fresh LSP version1: те же пять unused-variable и одно unused-include, что на точном baseline; новых диагностик нет. Независимое ревью дельты: No issues found in inspected scope. Parent сверил manifest, hashes и все 49 selection/range/name joins. В symbols.json обновлены только 48 Draw-записей (42→48, включая прежние пропущенные helpers/forward declaration); 1026 внешних записей и их порядок сохранены. Остальной generated и callgraph не пересобирались. Дополнительный controlled SDK25/C++14 стенд не исполнялся: после двух ошибок подготовки последняя линковка завершилась LNK2019 ACAPI_Element_Tool; RED/GREEN не заявляются. Другие SDK/macOS и финальный runner перед закрытием issue не проверены.

- Перед коммитом владелец оставил только английский текст диагностики. Единственная дельта к native-снимку a24c88e8 — это сообщение; текущий Draw11bfe814 проверен повторным canonical AC25 Debug build exit0, APXaa2e7e14. Новый native-прогон не выполнялся, host закрыт; оба PLN восстановлены побайтово. 48 Draw-координат перенесены по точному diff и проверены source/name join, внешний generated-срез и порядок неизменны; fresh LSP выше относится к прежнему native-снимку.

## Минимальная площадь отделки (#320)

- `OtdRoom_FilterSmallFinishes` (`RoombookData.cpp:5767`) удаляет из окончательных массивов стен/колонн/откосов и полов/потолков записи с площадью строго меньше `min_area`; равенство сохраняется. Использует `OtdWall_GetArea` с отверстиями и наклоном; для Mesh-пола — площадь поверхности, как в quantities/JSON. Отрицательные и nonfinite значения не объявляются малой площадью. [по коду]
- Вызовы в `OtdRooms_ProcessFinishes` охватывают и раннюю ветку только с плитами, и полный маршрут после проверки provenance, вычитания профильных стен и финального устранения перекрытий, до построения индексов плинтусов. Самостоятельного площадного фильтра линейных плинтусов нет. Малые записи не передаются writers, quantities и JSON. [по коду]
- GUID существующих плит сохраняются после назначения, до изменения контуров; ссылки без сохранившегося элемента возвращаются в штатный unused-список с прежним fallback `NoSet`. Сохранившийся дубликат GUID не возвращается. [по коду]
- Малые grouped-контакты и грани балок пропускаются до отказа адаптера/флага Ambiguous; projected slab ambiguity учитывается по каждому пересечению, а не по сумме малых пересечений. Геометрическое вычитание и исходный partition не меняются. Неоднозначные контакты площадью не меньше порога сохраняют диагностику. [по коду]
- Проверка: AC25 Windows, standalone RED/GREEN и полный TU SDK25/MSVC C++14; это не сборка/загрузка аддона и не native-приёмка. Generated обновлён только для семи затронутых функций по свежему isolated clangd; остальные исторические координаты и callgraph не объявляются актуальными.

## Пустой тип плинтуса (#319)

- `FloorSkirtingSegments_Build` очищает подготовленный напольный массив и прекращает подготовку при пустом `room.tip_floor_skirting`. [по коду]
- `SkirtingSegments_Build` очищает оба массива; пустой `tip_floor_skirting` отключает только Floor, пустой `tip_ceil_cornice` — только Ceil. При обоих пустых типах геометрия не обрабатывается. `beam.tip` берётся из соответствующего поля зоны, включая потолочный тип для Ceil. [по коду]
- `OtdBeam_ProcessFinishes` исключает faces отключённого вида до расчёта толщины/отметки; исходные OtdSlab не меняются. Проверки относятся к пустой строке (`IsEmpty`), не к строке из пробелов. Штатная политика удаления ранее созданной отделки не изменена. [по коду]
- Локальная цель проверки — AC25 Windows; native-создание/удаление и другие версии [не проверено]. Исторические generated-координаты этой правкой не объявляются актуальными: MCP вернул устаревший конец namespace 9586 при текущем файле более 9748 строк.

## Тип существующей линейной отделки (#316)

Адресный контракт AC25 Windows, рабочий `RoombookDraw.cpp` SHA256 `13b75f605590c1771deed948296339cedada8bc3cbe2a74f922b9b9c31f7a5c6`; прочие исторические координаты ниже не актуализированы этим срезом. [по коду]

- `OtdRooms_AssignExistingWallFinishes` (`RoombookDraw.cpp:446`) выбирает reader целевого типа по `wall.draw.favorite.type`, как `OtdWall_Draw`: Wall, Object или Beam. `surface_geometry` нормализует `target.wall.type` только в ветке Wall и не меняет выбранный native-тип. Подбор по библиотечной части Object, роль/основание, одноразовое потребление GUID и штатное удаление остатка сохраняются. [по коду]
- `Object_Draw_Linear` (`RoombookDraw.cpp:1006`) начинает с `draw_error=APIERR_BADPARS`; ранний отказ не считается успешной записью. Если прочитанный GUID не Object, GUID результата очищается и writer не вызывает Create/Change. Успешные Create/Change/no-op записывают фактический код результата в конце функции. Геометрия и политика транзакции не меняются. [по коду]
- Native-переход Wall→Object и повторный запуск в Archicad — [не проверено].

## Полный JSON-набор отделки (#314, 2026-11-06)

Адресный контракт рабочей дельты к `04e4dd9`, AC25 Windows; исторические координаты ниже не пересобраны целиком. [по коду]

- `AppendRoomFinish`/`CollectRoomFinishes` (`RoombookDraw.cpp`) собирают все записи `otdwall`, `otdslab`, `otdbeam_ceil` и `otdbeam_floor` обработанной валидной комнаты, независимо от нового создания, успешной записи, наличия GUID и допуска к writer. Это подготовленный набор текущего запуска, не полный native-инвентарь зоны. Подготовку, ранние отказы и построение не меняют. [по коду]
- `RoomBookFinishResult.is_element` отличает настоящую расчётную запись с null GUID от служебной записи пустой комнаты; в JSON этот внутренний признак не выводится. `is_valid` передаёт валидность wall/slab; у подготовленных beams отдельного флага валидности нет, поэтому он true. [по коду]
- Дополнительные поля элемента JSON: `is_valid`, `drawing_enabled`, `draw_error`. `drawing_enabled` — существующий `OtdWall_ShouldDrawFinish`/`OtdSlab_ShouldDrawFinish` (валидность и настройки комнаты); для beams — валидность комнаты, поскольку отдельного writer-флага для их массивов нет. Это не гарантия пригодности избранного или успешной записи. [по коду]
- `write_succeeded=true, was_created=false` включает и обновление, и no-op; различие между ними не определяется этим ответом. Отключённый/пропущенный writer не становится созданным результатом. Нулевой `otd_guid` допустим; ненулевой GUID без успешной записи сам по себе не доказывает существование в модели. [по коду]
- Площади/длины по-прежнему подготовленные, не native read-back. NaN/Inf заменяются числом 0 с соответствующим `*_available=false`, а не объявляются измеренным нулём. Невалидные записи явно помечены `is_valid=false`. [по коду]
- Публикация остаётся после успешной draw-транзакции; при rollback `pendingResults` не публикуется. Ранний отказ до этой стадии по-прежнему может дать пустой ответ. Запись свойств/Sync идёт отдельно после публикации — `status:returned` не доказывает её успех. Полнота по составам `base_composite` и привязкам result-свойств этой правкой не добавлена. [по коду]

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
