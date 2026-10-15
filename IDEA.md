﻿﻿# Current Task Roombook: отделка по поверхностям (#252)

## Task

Продолжение переноса на поверхности 3D-зоны, SDK AC25 Windows. Reader, partition/beamPart и подключение production-создания/расчёта выполнены в проверенном объёме. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/252. План: .hermes/plans/2026-10-11_115207-roombook-zone-surfaces.md.

## Scope

По команде владельца scope текущего рефакторинга дополнен очисткой ненужной диагностики, упрощением данных/ветвлений и минимальной осмысленной организацией файлов Roombook. Подробный план C1–C9 — в Next Step / «Читаемость и минимальная структура»; новая issue не нужна. В этом ходе только план, реализация ещё не выполнялась.

Создание и расчёт отделки из проверенных участков: стены, колонны/сегменты, перекрытия и балки (низ и бока), штатные writers и материальные правила. Один участок — одно основание; неоднозначные/непредставимые участки не создавать. Представимые негоризонтальные грани — штатные наклонные стены на отдельном слое с группой пересечения 0. Исходные конструкции и сохранённые PLN не менять. AC25 Windows; остальные SDK отдельно, AC_25 gates не возвращать. Docs/generated и коммит только по отдельной команде. По явному решению владельца дополнительно завершить восемь однопараметрических вызовов AdvanceProcessPhase в Roombook.cpp, передав существующий nPhase (завершено). Пользовательский перенос CommonFunction.cpp/.hpp не менять.

## Status

WAITING_FOR_ACCEPTANCE: переход на одну стену с прямоугольными вырезами и реальными base_guid проверен на AC25 Windows; selftests75/75, native335→335, покрытие представимых контактов и повтор в1e-6м PASS. Ограничение: две полные грани с диагональным краем20.498776м² пропускаются целиком, исходный проём4F6A150D-A60B-4ABA-927D-0FC46F0D3FE7 пока не имеет отделочного проёма. Подробности Archive/«Одна стена с проёмами». #252 OPEN; Docs/generated и commit по Scope отложены до команды владельца.

## Last Completed

2026-10-14: #272 оставшиеся API-кэши реализованы (команда владельца, AC25 Windows): ACAPI_Element_Get кэшируется в AddSurfaceFinishPiece, BuildRoomSurfaceFinishes (балки), OtdWall_Create_FromWall и Floor_FindInOneRoom; ACAPI_Element_GetRelations зоны читается через ReadZoneRelations и в discovery (ReadRoomSurfaceCandidates), и в CollectRoomInfo — общий value-снимок RoomRelationValues; подписи Roombook.hpp/cpp синхронизированы (Otd_GetOtd_ByZone, Class_IsElementFinClass, OtdWall_Create_FromWall получили RoomSurfaceCache*); отчёт «RoomBook source route» расширен counters element/classification/relation/connected. Исключено владельцем: ACAPI_Favorite_Get, ACAPI_Attribute_Get. Не кэшируется: ACAPI_Element_GetConnectedElements с фильтром типа (повтор только на граничных стенах, нужен составной ключ — отложено), однократные чтения (зона CollectRoomInfo, Opening_Create_One), memo-владеющие хендлы, dead code (OtdWall_Create_FromColumn, Floor_Create_All/One), write/draw-стадия. LSP 0 диагностик по 4 файлам, clang-format PASS, canonical BuildAddOn.py AC25 Debug success. Runtime не выполнялся.

2026-10-12: Z9 направление верхних откосов исправлено и проверено AC25 Windows: локально удалён повторный разворот Wall-fallback;24адресных RED→GREEN в двух RoomBook,75/75selftests,335→335native/links без дублей. Canonical BuildAddOn compile/link success; подробности Archive/Z9 direction. Полная приёмка глубин/материальных правил и остальных проёмов остаётся открытой.

2026-10-12: одна surface-стена с проёмами — проверено в ортогональном объёме, AC25 Windows. Реальные opening base_guid сохранены; финальная сборка, LSP, selftests75/75, независимое покрытие и native335→335 в1e-6м PASS. Две диагональные грани остаются явно неподдержанными; детали и границы проверки в Archive/«Одна стена с проёмами».

2026-10-12: Z9 — подготовлено восстановление consumer-пути откосов; существовавшая заготовка сохранена, добавлены opening-only guards. Свежий direct LSP текущего файла и HEAD независимо сравнены: version1, 13/13 одинаковых severity/code/message, новых ошибок нет. Состав ACAPI-вызовов не изменён, ReNum checksum сохранён; clang-format и git diff --check PASS. Artifact: scratch/z9-reveal-restore/static-validation.json. Compiled: not performed; Tested: not performed для изменённого кода. Runtime заблокирован вопросом сохранности модели; задача не завершена.

2026-10-12: проверка двух новых окон AC25 завершена с FAIL — боковые и верхние откосы отсутствуют. Два реальных RoomBook: 107→107 элементов, native-тела и links совпадают, окна и сохранённые PLN не изменены. Причина по коду — потеря openings при замене legacy-стен поверхностными. Исправление не выполнялось; подробности Archive/Z8, #252 OPEN.

2026-10-11: пункт 3 исправлен и проверен. GroupSurfaceWallPieces объединяет представимые копланарные стеновые контакты только с одинаковым полным source-face provenance (base/type/geometry/body/polygon/material), zoneBody и направлением нормали, затем применяется прежний BuildSurfaceWalls. AC25 Windows Debug canonical BuildAddOn.py compile/link success; свежий direct LSP без новых диагностик; self-tests 60/60, восемь новых regressions и ERROR IN TEST=0 подтверждены в актуальной Отладка. Реальные deterministic/deterministic-repeat:63→63 (47 стен+16 перекрытий),126 links,63 layer-properties/group0,15 исходных GUID сохранены. Все ориентированные native-контуры отдельных тел совпадают в допуске1e-6 м (max drift6.5868126e-7 м), bodywiseEqualRounded5=true; независимое покрытие прежних трёх стен и шести контактных плоскостей двух балок PASS в обоих запусках. Сохранённые PLN hashes unchanged. Артефакт scratch/z7-writer/deterministic-final-summary.json; APX SHA2564f4fb65acf699f79db6669d63931fd6a9f7f6cf965074fe1c122e3ed3b9f95c4. Пункт 2 классифицирован, но четыре трапеции остаются непредставимыми прежним writer; полное создание всех контактов не заявляется. Commit/push нет.

Предыдущий проверенный этап:

2026-10-11: исправлены три крупные пропущенные стеновые грани (536DEA6F, FA9703B0, 2F066511): точное разложение контуров с вырезами/отверстиями через BuildSurfaceWalls, прежние writers и материальные правила. Восстановлено 98.701516 м²; обе направленные разности проверены по исходным контактам в двух реальных запусках AC25. Canonical BuildAddOn.py Debug compile/link success; fresh direct LSP: адаптер/тесты 0, Roombook прежние 13 диагностик. Self-test 50/50 Passed, новые пять ok подтверждены в Отладка. Создано 61 элемент (45 стен, 16 перекрытий); повтор удалил 61 и создал 61, исходные 15 GUID сохранены. 122 связи и 61 элемент на слое с intersectionGroupNr=0 прочитаны. Контакт/сторона толщины проверены; max plane gap 4.81e-7 м. Покрытие сохраняется, но разбиение трёх стен на отдельные тела между запусками изменилось — идентичность каждого тела НЕ подтверждена. Оба сохранённых PLN hashes unchanged. Артефакты scratch/z7-writer/coverage{,-repeat}-wall-coverage.json и coverage-final-summary.json; APX SHA-256 0574aab3c19ee8eaae36c7a30847fe873970605d5200fda8cb61358ddd19b4be. Archicad оставлен в VS Run с результатом; commit/push нет.

## Next Step

### Читаемость и минимальная структура — часть текущего рефакторинга

#### Цель и границы

По решению владельца целевая структура — четыре файла реализации по назначению: общий; сбор и анализ данных; расчёт площадей для записи в свойства; отрисовка элементов. Предыдущая схема сохранения отдельных Surfaces/Partition/Finish как самостоятельных production-модулей отменена. Новая issue не нужна: это часть текущего рефакторинга #252 и связанных #272 (ранее также #267/#268). Проверяемая платформа — AC25 Windows, остальные SDK/macOS отдельно.

Сейчас меняется только план. Реализация, сборка, взаимодействие с Archicad, коммит/push и закрытие issue не выполняются. Текущие пользовательские/параллельные изменения и незавершённые функциональные задачи сохраняются. CommonFunction/Helpers/Sync/ReNum, ресурсы, исходные конструкции и сохранённые PLN вне scope; внешние callers можно минимально поправить при будущей смене include/границы. Docs/generated обновляются только по ранее установленному отдельному разрешению.

#### Целевая организация файлов

Все пути ниже относительно Sources/AddOn/roombook. Названия новых файлов — предложение; смысл четырёх частей задан владельцем.

| Файл | Содержание | Чего в нём не должно быть |
|---|---|---|
| Roombook.cpp — общий | RoomBook, последовательность этапов, общий прогресс/отмена, управление жизненным циклом запуска, координация финальной записи | Обхода полигонов, расчёта площадей, тел writers и длинных предметных алгоритмов |
| RoombookData.cpp — сбор и анализ | Чтение зон/оснований/проёмов/свойств/избранного, run-local кэши, классификация входов, выбор оснований, analytic/native, сопоставление поверхностей, подготовка геометрии OtdWall/OtdSlab, правила материалов/высот/откосов | Создания/удаления элементов и записи рассчитанных площадей в модель |
| RoombookQuantities.cpp — площади и свойства | Расчёт площадей готовой отделки, группировка по материалу/виду/типу помещения, форматы столбцов и подготовка значений свойств | Повторного сбора геометрии из SDK, изменения подготовленной геометрии или создания элементов |
| RoombookDraw.cpp — отрисовка | Draw_Elements, стены/балки/объекты/проёмы/перекрытия, defaults для создания, удаление старой отделки, классификация результата, формирование связей с зоной/основанием | Повторного выбора оснований, новых правил разбиения геометрии или собственного подсчёта ведомости |

Roombook.hpp — общий контракт: существующие Otd-типы и действительно необходимые межфайловые объявления. Не создавать автоматически ещё три hpp, новый файл типов, классы-менеджеры или новый общий framework. Локальные helpers остаются в своём cpp; наружу выходят только необходимые границы этапов. Общий файл не становится местом для произвольных «общих функций».

Старые RoomSurfaces/RoomSurfaceCandidates/RoomSurfacePartition/RoomSurfaceFinish cpp/hpp поглощаются частью сбора и анализа и общим контрактом, затем удаляются после проверки callers и сборки. Matcher и адаптеры остаются понятными секциями RoombookData.cpp, но не отдельными production-файлами. Коллизии anonymous helpers разрешать минимально: одинаковый алгоритм оставить один; похожие с разными инвариантами не объединять вслепую. Линейный/плоскостной/площадной допуски не менять.

Четыре файла — четыре cpp реализации, а не обещание удалить любой вспомогательный заголовок ценой потери тестов. CoordinateSearchCache.hpp/RoomAnalyticGeometry.hpp/FloorCoreGeometry.hpp сейчас используются production и SDK-независимыми проверками; их судьба — отдельный пункт census. Не сохранять дополнительные headers автоматически, не вводить предлагаемый прежде RoomGeometry.hpp автоматически, не копировать algorithms в tests. Если чистый тестируемый kernel требует отдельного header, объяснить конкретную необходимость до переноса; это не пятый production-этап.

#### Проверенные границы и важные оговорки

- RoomBook (Roombook.cpp:1545–1622) уже задаёт последовательность: сбор → параметры/правила → сводка площадей → подготовка обновления → Draw_Elements → SetSyncOtdWall. При переносе сохранить именно существующий порядок и ранние выходы, а не перестроить pipeline по именам новых файлов.
- BuildMaterialSummaryForRooms (:1028), WriteRoomMaterialData (:1122), CollectMaterialAreasForRoom (:1933), OtdData_CalcForRoom (:1985), OtdData_WriteToRoom (:2067), OtdData_AddValueToDict (:2185) — текущие кандидаты части площадей. Точные границы остальных OtdData helpers и формата сверить при census.
- WriteRoomMaterialData подготавливает значения в summary.paramToWrite; название не означает немедленную запись в модель. Не переносить фактический commit свойств раньше создания и привязки элементов. Разделить владение расчётом значений и общий момент их применения, не вводя второй независимый writer.
- SetSyncOtdWall (:6380) обрабатывает связи созданной отделки и общий paramToWrite; он не относится к арифметике площадей. Размещение в части отрисовки/завершения записи; общий файл сохраняет момент вызова. Уточнить весь хвост функции до реализации, не менять транзакции по предположению.
- OtdWall_GetArea распределить в часть площадей после проверки всех потребителей: геометрический helper может использоваться и подготовкой. Нельзя дублировать его реализацию ради границы файлов.
- RoomSurfaceSource.routeReason используется диагностическим consumer для PendingPreparedGeometry, profile assertion и JSON, поэтому удалять одно поле изолированно нельзя. По новому решению удалить диагностический consumer/export вместе с полем, если все его потребители только диагностические; если census найдёт необходимое production-состояние, сохранить только это состояние без текстовой трассировки. Проверку реального fallback сохранить поведенческим тестом. Комментарий «не читается» не считать доказательством ненужности.
- Tools/CMakeCommon.cmake:376–386 собирает cpp/hpp через GLOB_RECURSE CONFIGURE_DEPENDS: ручной source list не нужен; после переноса требуется reconfigure и проверка состава translation units.
- Ранее обнаруженный include-цикл относится к наивной попытке сохранить Finish.hpp → Roombook.hpp и добавить обратный include. Он не доказывает невозможность выбранной владельцем структуры. Общие типы объявить в существующем Roombook.hpp в порядке зависимостей, старые циклические includes убрать. Размер файла сам по себе также не отменяет выбранную структуру: сначала удаление лишнего, затем предметные секции.

#### План C1–C9

##### C1. Карта переноса и исходная проверенная точка

- [ ] Снова прочитать IDEA/status/diff, сохранить точные копии затрагиваемых исходников в Hermes scratch. Не откатывать/стэшить пользовательское дерево.
- [ ] Составить полную таблицу существующих функций/типов: текущий файл/символ → одна из четырёх частей → callers → локальная/межфайловая граница. Включить private/static helpers, caches, defaults, Favorite/Class, OtdData, legacy и TESTING-exports. Сначала generated callgraph, затем clangd/targeted source; miss графа не означает отсутствие вызовов.
- [ ] Отдельно разнести read-only получение избранного/классификаций и defaults/классификацию при создании. Не делить функции по префиксу, если побочные эффекты относятся к другому этапу.
- [ ] Установить текущий baseline и действительные FAIL. Исправление подтверждённого функционального дефекта — отдельная проверяемая дельта в существующей задаче; не маскировать его переносами.

##### C2. Локальная очистка параллельно завершению функциональных задач

- [ ] По мере проверки пола/откосов/кэшей удалять временные probes, повторные данные и комментарии истории именно этого участка. Не объявлять незавершённые задачи принятыми из-за чистоты кода.
- [ ] Проверить кандидаты удаления: ReadRoomSurfaceZBounds и его bounds-storage/counters (поиск нашёл только declaration/definition), дублирующий coordinateSearchStats, wrapper PrepareReadParams, legacy-остров OtdWall_Create_FromColumn/Floor_Create_All/One. Перед удалением проверить callers/условные версии/экспорты; production-правила полов сохранить.
- [ ] Упростить ветвления и структуры только по доказанной ненужности; сохранить реальные body/contour/hole, происхождение основания, неоднозначность, cache key/error/lifetime. Не вводить дополнительные уровни контейнеров или универсальные абстракции.

##### C3. Диагностика

- [ ] По прямому решению владельца убрать из рабочего кода Roombook всё, относящееся к диагностике: отладочные DBprnt/DBtest/msg_rep и дампы, probes, трассировку, timing/cache/route counters, диагностические поля/структуры, сбор данных только ради отчётов, отладочные команды/JSON-экспорты и их регистрацию. Перед удалением проверить побочные эффекты и действительных потребителей. Это требование очистки кода, а не отключения вывода.
- [ ] Удалить диагностическую обвязку полностью, включая находящуюся под TESTING; не оставлять её «на будущее», не прятать за макросами/флагами и не переносить в отдельный logging/diagnostic-файл. Прежнее исключение для адресной диагностики и counters при приёмке кэшей отменено. Дальнейшие расследования вести отладчиком: breakpoints, locals/watch, call stack, без постоянной инструментализации production-кода.
- [ ] Сохранить функциональную обработку ошибок: проверки результатов, коды/статусы, отказ от неполного результата, освобождение ресурсов и необходимые пользователю сообщения о невозможности операции. Это не диагностическая трассировка. Автоматические тесты и их assertions не удалять ради чистого production-кода; минимальный вывод тестового runner оставить в тестах, не в рабочих алгоритмах. Тесты, проверяющие диагностическую строку/счётчик, перевести на проверку поведения, а не просто убрать failing assertion.
- [ ] Убрать исторические номера issue/шагов/чекпоинтов и REFACTOR PLAN из затронутых комментариев/названий проверок. Оставить суть поведения, русские комментарии только для неочевидного.

##### C4. Общий контракт и общий файл

- [ ] Разложить существующие общие типы в Roombook.hpp по зависимостям: входные снимки/основания → результаты анализа → Otd-данные → необходимые границы этапов. Проверить варианты SDK; не добавлять новый файл типов.
- [ ] Оставить RoomBook координатором существующего порядка; общий nPhase/progress/контекст не размножать по cpp. Внутренний контекст раскрыть только в реально необходимом объёме, без новой иерархии.
- [ ] Убрать циклы старых includes; не экспортировать каждый static helper. До переноса каждой группы проверить символы/глобальные данные и сохранить время жизни кэшей до записи.

##### C5. Выделить расчёт площадей и свойств

- [ ] Перенести MaterialSummary, BuildMaterialSummaryForRooms, WriteRoomMaterialData, семейство OtdData и принадлежащие им area/format helpers в RoombookQuantities.cpp по карте C1.
- [ ] Сохранить правила учёта проёмов/уклона/материальных слоёв, форматы, порядок группировки и rawname/rawname_bytype. Не заменять расчёт геометрии итоговой ведомостью и не пересчитывать входы из SDK.
- [ ] Убрать из расчётного цикла несвойственный ему отбор зон на создание только отдельной проверяемой дельтой; сначала сохранить фактическую семантику BuildMaterialSummaryForRooms, которая сейчас делает оба действия. Не менять фильтры создания при механическом переносе.
- [ ] Проверить площади и подготовленные значения свойств до/после, включая общий тип помещения, отсутствие свойств/материала, отверстия и повтор. Пока фактическая запись остаётся в прежнем месте.

##### C6. Выделить отрисовку

- [ ] Перенести Draw_Elements, OtdWall/OtdBeam/Opening/Floor writers, их defaults и только связанные с созданием helpers в RoombookDraw.cpp. Рабочие rules/подготовку геометрии оставить в Data.
- [ ] Здесь же разместить удаление устаревшей отделки, классификацию результата, связи и SetSyncOtdWall по карте эффектов; общий файл сохраняет порядок вызовов и gates резервирования/отмены. Не менять scope undo, API-параметры и обработку ошибок.
- [ ] Проверить compile/link и адресные writer-regressions. Затем в разрешённом host-сценарии проверить реальные тела/классификацию/связи, отсутствие лишних записей исходных конструкций, удаление старых GUID и повтор без дублей.

##### C7. Собрать единый файл сбора и анализа

- [ ] Перенести в RoombookData.cpp текущие RoomSurfaces/RoomSurfaceCandidates/RoomSurfacePartition/RoomSurfaceFinish и относящиеся к сбору/подготовке части Roombook.cpp. Секции: чтение/кэши; основания; конечные контакты; подготовка стен/проёмов/балок; полы/потолки; параметры/материалы.
- [ ] При объединении translation units сверить одинаковые anonymous helpers/константы. Одинаковое не размножать, различающиеся инварианты не объединять вслепую; допуски и алгоритмы не менять.
- [ ] Упорядочить соседство функций по чтению production-пути, удалить ненужные wrappers/копии, не дробить обратно на reader/partition/adapter-файлы.
- [ ] Обновить callers/includes и решить судьбу чистых testable kernels по C1. Старые cpp/hpp удалять после переноса всех consumers и успешной проверки; случайные внешние файлы не удалять.

##### C8. Проверка поведения и читаемости

- [ ] После каждого проверяемого переноса/изменения: clang-format затронутых cpp/hpp/h с сохранением SDK include-order, scoped diff/diff-check, fresh clangd baseline/final; canonical python Tools/BuildAddOn.py -c config.json -v 25 при разрешённом состоянии VS. Новая ошибка или регрессия — остановка и отмена только собственной дельты, не пользовательского файла целиком.
- [ ] Исполнить имеющиеся host/standalone regressions по проверенным командам. Если перенос затрагивает SDK-независимый executable, подтвердить сохранение общей production-реализации и его исполняемости; не считать компиляцию выполнением тестов.
- [ ] С разрешения на безопасной фикстуре выполнить host/selftests и RoomBook + повтор, прочитать актуальную VS «Отладка». Не останавливать чужую debug-сессию и не сохранять PLN.
- [ ] Сравнить native геометрию/покрытие/проёмы, материалы, настоящие base/zone links и значения итоговых свойств, а не только количество элементов. Покрыть стены/профили/балки/откосы/полы/потолки/перепады/ядро в реально доступных сценариях. Полноту reader подтвердить; непроверенное оставить not verified.
- [ ] Прочитать конечные четыре файла как пользователь: из общего виден сценарий; Data отвечает за подготовку; Quantities — за свойства; Draw — за создание. Нет временного шума, лишних переходов, исторических названий или обратного дробления на технологические файлы.

##### C9. Завершение

- [ ] Сверить фактическую карту с четырьмя выбранными владельцем частями, объяснить каждый оставшийся вспомогательный header. Отменённые прежние схемы не оставлять в pending backlog.
- [ ] По отдельному разрешению обновить Docs/modules/Roombook.md и scoped generated; архивировать только завершённое/проверенное, сохранить остальные активные задачи и ограничения.
- [ ] Новую issue не создавать. Коммит/чекпоинт/push только по отдельной команде, только intended files.

#### Приёмка

Основная реализация разделена по четырём пользовательским обязанностям, а не по внутренним стадиям геометрического алгоритма. Общий файл показывает порядок работы; сбор/анализ не пишет модель; расчёт площадей готовит правильные значения; отрисовка сохраняет существующий контракт создания/удаления/связей и момент применения свойств. В рабочих алгоритмах и их TESTING-ветках нет диагностических сообщений, dumps/exports, counters и диагностических полей/обвязки; исключение «оставить для будущей отладки» запрещено. Разбор причин выполняется отладчиком. Функциональные проверки ошибок, необходимые пользовательские сообщения и самостоятельные автоматические тесты сохранены. Тестируемая логика не продублирована в тестах. Ранее проверенные поведение, правила и repeat сохраняются в реально выполненном объёме. SDK/macOS и неисполненные сценарии — not verified.

#### Состояние, следующий шаг, checkpoint

PLAN_UPDATED_BY_OWNER: выбрана четырёхчастная структура; прежняя целевая карта отменена. Дополнительно принято полное удаление диагностической обвязки из рабочего кода, включая TESTING; прежнее сохранение адресной диагностики отменено. Предпочтительный способ дальнейшего расследования — отладчик. Реализация C1–C9 не выполнена. Следующий шаг — C1: карта распределения каждой функции/типа по четырём файлам, общие границы и текущий baseline; затем очистка рядом с завершением действующих задач. Разрешение на автономную реализацию/перезапуск/commit этим планом не подразумевается.

Last Checkpoint: нового нет; существующие записи других задач не переопределяются. Verified: текущая координация RoomBook, подготовка summary/свойств и начало writer/связей прочитаны по исходнику. Порядок include/все межфайловые границы будущего переноса — not verified до C1. Compiled/Tested: not performed, изменён только план.

### #272 — ускорение 3D RoomBook и API-кэши

#272 API-кэши — реализованы частично; прежнее заявление о полном завершении снято по результатам review ниже. Команда владельца: все кэши, кроме ACAPI_Favorite_Get и ACAPI_Attribute_Get.

### #272 — review реализации API-кэшей (AC25 Windows)

Scope: только добавленные Element/Classification/ZoneRelations/Connected caches и их production-callers. По команде владельца RC1–RC4 исправлены; AC25 Windows. План: [x] checked connected-result/error и полный ключ; [x] callers и итоговый отчёт; [x] targeted regression/format/LSP/canonical build; [/] общий host-прогон (один FAIL вне cache checks, описан ниже). Helpers/Favorite/Attribute и чужие изменения сохранены; нового checkpoint нет.
- Исправлено: RoomSurfaces.hpp/.cpp теперь кэширует raw GetConnectedElements по GUID/type/filter/variation/renovationFilterGuid вместе с error и исходными items, включая пустые/частичные ошибочные ответы. ReadConnectedElementsCached — общий production storage/helper, тесты вызывают его с callback без настоящих SDK queries. CollectRoomInfo читает Window/Door с прежними editable/access/workspace фильтрами, проверяет error и освобождает zonememo при отказе; void GetRelationsElement больше не используется. PrepareSource и индекс Window/Door используют ту же raw-читалку с renovation-only фильтром. Итоговый ReportRoomReadCounters вызван после ProcessRoomFinishes, до writes. connectedReads теперь соответствует одному реальному connected SDK query на miss. ReadZoneRelations по-прежнему кэширует только успешные snapshots.
- Validation исправления: canonical BuildAddOn.py -c config.json -v25 Debug success; APX SHA256 2f804c5c5b2c74b3c3f5527cf6f6df6303ad2782c8094464ae0d201b88f58415, все пять изменённых source/test файлов не новее APX. clang-format --dry-run --Werror и git diff --check PASS; MCP force_refresh RoomSurfaces/Roombook/TestRoombookGeometry 0 diagnostics. В уже работающем VS Run/PID56976/test_25 загружен Debug/SomeStuff.apx, API port19723 принадлежит этому PID. Нашими действиями Archicad не запускался/не закрывался; read-only RoomGeometryDiagnostic partitionSelfTest выполнил 12/12 новых connected-cache checks PASS (JSON и текущие DBtest строки панели «Отладка» подтверждены).
- Ограничение общего прогона: 247 checks, 246 PASS, 1 FAIL — «272 profile explicitly requests cached native fallback». TestRoombookGeometry.cpp:285 проверяет routeReason==ProfileWall, а текущий BuildPreparedWallSource при отказе печатает эту строку через DBprnt вместо присваивания routeReason (:234–247); cache-fix этот участок не менял. Профильную геометрию/чужие правки не исправлять в этой задаче. Общий selftest НЕ GREEN. Runtime native connected-query semantics, полный RoomBook final counters/геометрия/повтор и A/B not verified; callback-regression их не заменяет.
- Evidence: Hermes scratch/fix272-selftests.json и fix272-manifest.json; новая regression tests/ConnectedElementsCacheChecks.hpp подключена в RunSurfacePartitionChecks. Next Step: отдельно согласовать/profile-test FAIL и production host-приёмку кэшей; issue #272 остаётся открытой. Last Checkpoint: нового нет, commit/push/Docs/generated по Scope не выполнялись; две исходные disabled breakpoint не менялись.
- P2 / RC1: RoomSurfaces.cpp:421–436 безусловно кэширует результат void GetRelationsElement; Helpers.cpp:875–877 не возвращает ошибку GetTypeByGUID, :971–997 не передаёт ошибки Window/Door queries вызывающему. Пустой или частичный ответ после сбоя становится обычным cache hit, повтор не обращается к SDK. Минимальный fix: checked чтение с сохранением кодов ошибок/полноты, без изменения общего Helpers API; consumer должен явно отличать ошибку от успешного пустого списка. Фактический сбой SDK/потеря геометрии в runtime не воспроизводились.
- P2 / RC2: Roombook.cpp:479–484 (API_OpeningID в PrepareSource) и :595–598 (Window/Door) остаются прямыми GetConnectedElements. PrepareSource вызывается для общего основания в нескольких комнатах (:695), поэтому повторные Opening queries не устранены. Минимальный fix: кэш сырого результата с полным ключом GUID/type/filter/variation/дополнительных SDK-параметров и error; не смешивать renovation-only запросы с editable/access/workspace запросами Helpers.cpp. Отсрочка этих кэшей не была отдельным решением владельца; требование «все кроме двух» пока не выполнено.
- P2 / RC3: Roombook.cpp:727–756 печатает counters в PrepareRoomPartitions; позднее Otd_GetOtd_ByZone (:1561) читает classification cache, а ProcessRoomFinishes (:1423) вызывает ReadElement для surface walls (:1159) и beams (:1298). Отчёт не содержит этих поздних reads/hits и не является итогом прогона. Минимальный fix: итоговый отчёт после ProcessRoomFinishes, до writers; connectedReads сейчас считает вызовы helper, не число raw SDK calls (обычно type lookup + Window + Door).
- P3 / RC4: clang-format --dry-run --Werror FAILED для RoomSurfaces.cpp/.hpp (включая новый блок :358–439); прежнее общее format PASS неверно. Минимальный fix отдельным шагом: clang-format только нужных файлов, проверить diff на посторонние изменения. git diff --check для roombook PASS.
- Дополнительная сверка документации: ReadZoneRelations (:407–409) НЕ кэширует ошибки; таблица relations хранит только успешные RoomRelationValues, в отличие от elements/classifications. Не утверждать, что все helpers сохраняют SDK error. AddSurfaceFinishPiece/BuildRoomSurfaceFinishes находятся в Roombook.cpp, не RoomSurfaceFinish.cpp.
- Проверено без findings в этой границе: GUID-кэши локальны RoomProcessingContext одного RoomBook; явные writers идут после cached readers; error semantics IsFinish и Class_IsElementFinClass различаются и сохранены; value-поля wallPart/beamPart/grouped GUIDs скопированы до disposal (DevKit25 APIdefs_Elements.h:4904–4932 + LightRAG); nullptr-path имеет guards; Favorite/Attribute чтения остались прямыми.
- Validation: MCP force_refresh вернул 0 diagnostics для RoomSurfaces.cpp, Roombook.cpp, RoomSurfaceCandidates.cpp; это не runtime-проверка. Новая сборка/тесты/A-B в review не выполнялись; предыдущая canonical AC25 Debug сборка в этой беседе успешна. Регрессии новых четырёх helpers в TestRoombookGeometry.cpp поиском не найдены; отсутствие всех тестов во всём проекте не утверждается.
- Next Step: по отдельной команде исправить RC1–RC4 и добавить targeted regression cache/no-cache, errors/empty, shared source между комнатами, новый run и final counters; затем canonical build и host-приёмка. Last Checkpoint: нового нет, commit/push/Docs/generated не выполнялись, прочие задачи/правки сохранены.

### #272 — предыдущая запись реализации (уточнения review выше имеют приоритет)
**Добавлено/изменено:**
- RoomSurfaces.hpp/.cpp: общие читалки ReadElement/ReadClassificationItems/ReadZoneRelations/GetRelationsElement через RoomSurfaceCache; ElementRead/ClassificationRead/RoomRelationValues хранят точный SDK-результат, включая ошибки и пустые ответы; все обращения к таблицам защищены `cache != nullptr` (nullptr = прямое чтение без кэша, диагностический путь).
- Roombook.hpp/cpp: Otd_GetOtd_ByZone, Class_IsElementFinClass, OtdWall_Create_FromWall получили `RoomSurfaceCache*` (default nullptr); все production-callers передают `&context.surfaceCache`; CollectRoomInfo читает relations зоны через ReadZoneRelations (RelationGuard удалён — дублирование с ReadRoomSurfaceCandidates устранено общим value-снимком); отчёт «RoomBook source route» расширен счётчиками element/classification/relation/connected (reads/hits).
- RoomSurfaceCandidates.cpp: ReadBase и IsFinish — через ReadElement/ReadClassificationItems; ReadRoomSurfaceCandidates читает relations зоны через ReadZoneRelations; beamParts копируются из снимка до disposal.
- RoomSurfaceFinish.cpp: AddSurfaceFinishPiece и BuildRoomSurfaceFinishes (балки) — через ReadElement. Floor_FindInOneRoom — ACAPI_Element_Get через ReadElement.
**На что обратить внимание при review:**
1. Ошибки/пустые ответы кэшируются на весь прогон — неудачный ACAPI_Element_Get внутри одного run не повторяется; если какой-то путь полагался на повторную попытку, это изменение поведения.
2. RoomRelationValues — value-снимок (вектора скопированы), разделяется CollectRoomInfo и ReadRoomSurfaceCandidates; мутация через const& невозможна, но сверить, что нигде не снимают неконстантную ссылку.
3. ACAPI_Element_GetConnectedElements с фильтром типа (Roombook.cpp ~479/595) НЕ кэширован: таблица connectedElements ключируется только по GUID, а повторы только на стенах, граничащих с несколькими комнатами — нужен составной ключ (guid,type); отложено, не регрессия.
4. Не кэшировано намеренно: однократные чтения (зона в CollectRoomInfo, Opening_Create_One), ACAPI_Element_GetMemo (владеет handle'ами — нельзя хранить сырые), dead code (OtdWall_Create_FromColumn, Floor_Create_All/One), write/draw-стадия.
5. ACAPI_Favorite_Get и ACAPI_Attribute_Get не трогались — по команде владельца.
6. Формат отчёта «RoomBook source route» изменился (новые counters) — если отчёт парсится внешним скриптом, обновить парсер.
7. Диагностический caller TestRoombookGeometry.cpp:2345 использует default nullptr — компилируется и читает без кэша (проверено: обращения в RoomSurfaces.cpp защищены `cache != nullptr`, RoomSurfaceCandidates.cpp только пробрасывает указатель).
8. Runtime НЕ выполнялся: поведение в Archicad не проверено; canonical BuildAddOn.py AC25 Debug success, LSP 0 по 4 файлам, clang-format PASS. Скорость не измерялась (A/B — следующий шаг владельца).

#272 AC25 Windows, 2026-10-14: canonical BuildAddOn.py compile/link exit0; actual host selftests195/195 failed0, standalone analytic20/20 PASS. Исправлены четыре SDK FAIL объединением coplanar analytic cells в цельные contours с отверстиями; необоснованный TopLinkedWall fallback снят после live API/native проверки высоты3.3м и отдельной SDK-regression. Фактический маршрут: analyticWall569/slab24/faces3694, nativeFallback2, wall3DReads0/slab3DReads1, snapshotReads64/hits1, boundsReads0; coordinate calls1522/hits3220/refused0. Два штатных RoomBook:114.963с и120.374с для63комнат; API.GetAllElements4209→4209, повтор удалил2657GUID и создал2657. Это подтверждает native запись/повтор, НЕ независимое покрытие/идентичность каждого тела и НЕ контролируемый A/B benchmark. Последняя MCP LSP проверка Roombook/TestRoombookGeometry0диагностик; format/diff-check PASS. Production-прогоны выше относятся к предыдущему APX SHA2568e8dd987e9cf060cd1d75bd4412fafd9065a91467e430bde1fb23c6b74cd93f3; saved repo/build PLN hashes unchanged, результат повтора оставлен в VS Run без сохранения; четыре исходные breakpoint восстановлены. Evidence:scratch/272-parent-host/final-runtime.json, selftests-toplinked.json, all-first.json/all-repeat.json, build-toplinked.log. Диагностический consumer исправлен локально: ReadZoneSurfacePartition гидратирует PendingPreparedGeometry native-снимками, явно выдаёт geometryRoute=DiagnosticNative3D; production-discovery не менялся. Адресный RED PartitionSourceSnapshot→GREEN: зонаB69820A5 partitionComplete=true,12pieces,ambiguity0,errors0,4sources Extracted. Последняя canonical AC25 сборка build-diagnostic.log exit0, APX SHA256c7df1f97b15ec872800ad160710b5c56b2292469cc799ba128926910b7ea882d; её host195/195 PASS и штатный RoomBook115.6987875с для63комнат, actual wall3DReads0/slab3DReads1, coordinate calls1522/hits3220. Native inventory1608→4209,2657added/56removed; saved PLN unchanged, исходные4breakpoint восстановлены. Новый artifact:scratch/272-parent-host/diagnostic-final-runtime.json. Next Step: независимый native geometry/link audit и контролируемый A/B при необходимости; native-диагностику не считать доказательством analytic production equality (SEO/trims намеренно игнорируются). #272 OPEN; commit/push/Docs/generated не выполнялись по Scope; общего завершения задачи не заявлять.

#272 coordinate-cache реализован на обоих discovery-маршрутах, общий run-local helper сохраняет exact XY/Z/type/filter, ошибки/пустые ответы; NaN/inf не кэшируются. Родитель прочитал helper/diff и заново скомпилировал standalone production-helper regression clang++ C++17 -Wall -Wextra -Werror: exit0, PASS calls29/hits10/refused18/newRun1. Только callback/key/result tests, не SDK/runtime/скорость. Fresh child clangd baseline/final:8прежних unused, новых0; parent исправил новые English комментарии helper/test на русский и clang-format/diff-check PASS. Сборка аддона/host-приёмка НЕ выполнялись. Native→analytic Otd route ещё выполняется отдельным этапом; IDEА/Scope/прочие задачи сохранены, commit/push нет.

Решение владельца #272 после уточнения: SEO и подрезки обычных стен/перекрытий ИГНОРИРОВАТЬ в аналитическом маршруте. Это сознательная разница с native телом, не ошибка matcher; её не маскировать под доказанную native equality. Для обычных представимых оснований — finite prepared OtdWall/OtdSlab; для профилей сохраняется native reader/cache. Зона остаётся native 3D, реальные GUID/проёмы/состав/политика полов/потолков сохраняются. Scope теперь включает минимальное изменение порядка подготовки/partition и явное analytic provenance; остальные API-кэши отложены. Next Step: реализация analytic route + exact coordinate cache, regression/static checks; canonical build/runtime после освобождения VS Run. Без commit/push/Docs/generated, связанные незавершённые задачи сохранить.

Коррекция владельца #272: для обычных стен/перекрытий не читать native 3D-плоскости оснований; сопоставлять 3D-грани зоны с заранее подготовленными конечными OtdWall/OtdSlab. Native 3D оставить для сложных профилей/непредставимых случаев после проверки границ. Подтверждено по source: ReadBase Wall/Slab безусловно читает snapshot, partition идёт ДО ProcessElementsForRoomData, BuildRoomSurfaceFinishes очищает промежуточные массивы. Поэтому требуется перенос подготовки base representations до partition, без использования конечной отделки как основания. Расширение 3D-cache приостановлено; exact coordinate-cache остаётся полезным, остальные API-кэши отложены. Next Step: read-only проверка минимального analytic маршрута/типовых gates/provenance/material/openings, затем реализация по уточнённому плану. SDK/build/runtime нового маршрута NOT VERIFIED; без commit/push/Docs/generated, остальная работа сохранена.

Read-only аудит дополнительных API-кэшей #272: подтверждены повторные Element_Get (кандидаты/discovery/legacy/каждый surface-piece), GetRelations дважды на зону, классификации между этапами/зонами, Favorite/Defaults на каждый writer, mapping building material→surface, connected windows/doors на каждый найденный wall-edge и повторный slab-list. Наибольший алгоритмический кандидат — coordinate-search внутри probe×slab-level; одинаковые XY/Z/type/filter запросы можно объединять, разные Z/фильтры не смешивать. Favorite_ReadComposite и ElementsRead уже имеют batch/guard, Floor_Create_All/One и Favorite_GetType live production callers не найдены. Runtime-стоимость/приоритеты NOT VERIFIED, код аудитом не изменён. Следующий шаг по отдельной команде — staged кэши с сохранением ошибки/ownership/UnhideUnlock, затем измерение. Подробности: skill lightrag/references/somestuff-room-api-cache-audit.md.

#272 — AC25 Windows: реализация кэша выполнена в RoomSurfaces/RoomSurfaceCandidates/Roombook cpp+hpp; общий geometry-GUID cache на этап чтения, включая ошибки/пустые; body-local component cache; bounds используют полный cached snapshot либо vertex-only world-Z без полигонов. Система координат float bbox по SDK/LightRAG NOT VERIFIED, bbox не используется. Parent review шести delta и hash manifest PASS; fresh clangd reader/candidates без compiler diagnostics, Roombook те же8unused baseline; tweak failures не объявлены clean exit0. Format/diff-check PASS. Артефакты: Hermes scratch/272-implementation. Compiled/Tested: not performed — VS по-прежнему Break в PartitionRoomSurfaces:334, отладчик не трогали. Следующий шаг: после разрешения владельца освободить debug-сессию, canonical build, cache/uncached проверки и одинаковый native сценарий с измерением времени; скорость пока NOT VERIFIED. Копии snapshots между комнатами пока сохраняются. #272 OPEN; checkpoint/commit/push/Docs/generated нет, карточка Roombook пока не описывает новый cache.

PartitionInvalidSourcePlane, AC25 Windows: по решению владельца PlaneTolerance=1e-4 м (0,1мм, равен Roombook::min_dim); точечный DistanceTolerance/AreaTolerance не менялись. Устранена собственная регрессия малых граней: вектор площади нельзя сравнивать с линейным допуском; согласованная stored-плоскость сохранена, fallback Ньюэлла считается относительно origin. Canonical BuildAddOn compile/link exit0, format/diff-check и MCP LSP0 PASS. Live selftests156/156 failed0; зона0AB1934B-44F0-444A-93C5-F1A6424C2345: partitionComplete=true,17pieces,errors0 (раньше source4/body1/polygon94 имел20мкмнеплоскость). RoomBook returned96.61с, но Отладка подтвердил следующий blocker Unsupported beam geometry51037AF1-1476-4A3A-BB18-4201022519C0: создание всей отделки НЕ подтверждено. Next Step: отдельно диагностировать эту балку; не считать returned доказательством создания. По разрешению владельца debug-процесс закрыт без сохранения и перезапущен; свободный orphan build-copy lock обратимо перенесён в scratch/partition-lock-78512947-f35d-4ca0-8f7c-36b5e5825b49.lck после exclusive-check. VS оставлен Run, пять исходных breakpoint восстановлены. Evidence: scratch/plane-tolerance-{selftests,live,roombook}.json и spillover/call_988feb13a9c04e32b4bcf39f23d2b47c.txt. #252 OPEN; Docs/generated/commit/push по Scope не выполнялись; прочие задачи сохранены.

Текущий шаг Z9 (2026-10-13): read-only favoriteName exporter добавлен; BuildAddOn AC25 success, APX SHA256131478ba92212656591bafcac746f8641b104a253efb664aced7a60184af6699. VS Run/PID41460/test_25, breakpoint нет. favoriteName=«плитка» пока отвечает API4001 «ongoing user input»: следующий шаг — завершить диалог/ввод в Archicad и прочитать реальный libpart/контракт. Обе сохранённые PLN SHA256a12f5aed6de0d356bba5d279cef49ead94a69ac1409844638aaf841913977795 совпадают; PLN не сохранялся. Новый RoomBook не выполнялся; Object-writer и профильный зазор не исправлены, Z9 не завершён; commit/push нет.

Принять проверенный ортогональный вариант либо отдельно согласовать точное представление двух диагональных граней20.498776м² и прежних четырёх трапеций. Полного охвата всех контактов не заявлять. При разрешении обновить Docs/modules/Roombook.md и scoped generated: BuildSurfaceWalls теперь создаёт оболочку с проёмами, GroupSurfaceWallPieces объединяет до проверки представимости, добавлен LinkSurfaceOpenings. Z9 direction завершён:24адресных RED→GREEN после удаления повторного разворота Wall-fallback,два запуска335→335проверены. Следующий шаг общего Z9 по команде владельца «Заканчивай починку всех откосов» — полная матрица всех исходных окон/дверей и трёх откосов, классификация каждого отсутствующего/нестандартного контакта и проверка параметрического/material-rule контракта. Два независимых исследования уточнили:четыре верхних native-результата отсутствуют,пятый существует с40ммзазором на профильной стене;ещё два окна потерял прежний checker,полосы1мм закрываются торцами верхов. Live VS подтвердил у отсутствующих верхов draw_type=API_BeamID/favorite.type=API_ObjectID:они доходят до пустого OtdBeam_Draw_Object. Владелец выбрал сохранить Object-избранное и реализовать горизонтальный аксессуар,без Wall-fallback;контракт существующих object/slab writers исследуется отдельно до правки. Live избранное у Object-ветки:«плитка»,API_ObjectID,libInd516. Чтение char-массивов через VS evaluate не показало имена параметров;для установления точного libpart/контракта добавляется read-only TESTING-экспорт Object/Window library identity и scalar GDL параметров,не создание нового GDL. Важная поправка аудита:SDK25 type6=API_ObjectID,а не Beam;прежнее название40Beam в отчётах неверно. Для окна1A6C live Param_SetToWindows:frame=.05,sill=-.05,reveal_from_sill=false,base_th=.25;причина профильного reference/frame зазора ещё не установлена. SDK25 содержит top/ends overrides Beam,но writer их не назначает — минимальная исправляемая ошибка. Общий Z9 IN_PROGRESS;исправленная ошибка знака остаётся GREEN. Archicad оставлен в VS Run с повторным результатом, сохранение PLN не выполнялось; полный TestKit, Undo/Redo, принудительный отказ SDK и другие версии not verified.

## Last Checkpoint

b55d12f [278] IDEA.md: состояние задачи отписки наблюдателей Refs: #278
(код отписки: 0dd7ba3; правки вылетов Undo.dll: 9d68f16 [#277]; чистка мёртвого кода Roombook: be55b78).

## Plan

- [x] Записан план C1–C9 очистки и укрупнения Roombook как части текущего рефакторинга; исходные границы/сообщения сверены по source, документальная вставка проверена.

- [ ] C1–C9: выполнить план «Читаемость и минимальная структура» параллельно завершению функциональных участков; начальный шаг — актуальная карта удалений/зависимостей и baseline. Очистка и переносы пока не выполнены.

- [x] Зона230 / memo-only перекрытия, AC25 Windows: PrepareSource читает polygon memo без generic-opening/native fallback для slab, включая отказ memo; отдельные opening frame/transform exports удалены. Canonical AC25 build memo-build.log exit0; фактический source-counter slab3DReads0. Сохраняются только отверстия polygon memo, отдельные API_OpeningID не восстанавливаются. Проверка потолка после изменения владельцем группы пересечения слоя на0 и повторного RoomBook: одно тело, верх/низ по31вершине, без holes, XY совпадает с зоной (max drift0), контакт6.9м. Повтор удалил все2664GUID первого запуска. Evidence: scratch/267-zone230/memo-validation.json. Общий selftest НЕ GREEN:246PASS/1FAIL (ProfileWall routeReason, отдельный scope #272); последняя APX на диске отличается от исходного memo-manifest, точная привязка повторного прогона к исходному APX not verified. Сохранённые PLN hashes unchanged. Нового checkpoint/commit/push/Docs/generated нет.

- [/] #272 оставшиеся API-кэши (команда владельца 2026-10-14, AC25 Windows): в RoomSurfaceCache добавлены ElementRead/ClassificationRead/RoomRelationValues/connectedElements. Кэшируются: ACAPI_Element_Get (кандидаты RoomSurfaceCandidates.cpp:95, подготовка :691, AddSurfaceFinishPiece :1146, балки :1278, OtdWall_Create_FromWall :2424, Floor_FindInOneRoom :2854), ACAPI_Element_GetClassificationItems (Class_IsElementFinClass + IsFinish, сырые пары и ошибка), ACAPI_Element_GetRelations зоны (value-снимок разделяют CollectRoomInfo и ReadRoomSurfaceCandidates), GetRelationsElement на ребро стены (:2316, по GUID стены). Исключено владельцем: ACAPI_Favorite_Get, ACAPI_Attribute_Get. Не актуально: GetElemList перекрытий (Floor_FindAllFromWalls удалён, вызов единственный :2789). Не трогаем dead code: OtdWall_Create_FromColumn :2696, Floor_Create_All/One. Реализация завершена 2026-10-14: все перечисленные сайты переведены на ReadElement/ReadClassificationItems/ReadZoneRelations; value-снимок GetRelations разделяют CollectRoomInfo и ReadRoomSurfaceCandidates (RelationGuard удалён); подписи hpp/cpp синхронизированы (Otd_GetOtd_ByZone, Class_IsElementFinClass, OtdWall_Create_FromWall); отчёт «RoomBook source route» расширен счётчиками element/classification/relation/connected reads/hits. LSP 0 диагностик, clang-format PASS, canonical BuildAddOn.py AC25 Debug compile/link success (SomeStuff.apx). Runtime/приёмка и ABB не выполнялись; commit/push/Docs по Scope нет.

- [/] Упрощение обычных оснований без SEO (#272 (закрыто #267)), AC25 Windows: локальный slab discovery исправлен, стены используют ранее подключённый prepared analytic маршрут. Новых3D-кэшей нет. Canonical build/214host checks/27standalone и native floor+ceiling зоны101/повтор PASS; whole-project geometry и performance A/B not verified. VS Run/PID45788 оставлен с результатом без сохранения, две исходные breakpoint восстановлены. Last Checkpoint без нового коммита; Docs/generated/commit/push не выполнялись по Scope.

- [/] #272 analytic preparedWalls/preparedSlabs и exact coordinate-cache реализованы; canonical AC25 build, standalone20/20 и host195/195, actual wall3DReads0/slab3DReads1, два RoomBook и native GUID replacement проверены (актуальная запись выше). Обычные top-linked стены используют уже разрешённую API-высоту; SEO/trims намеренно игнорируются. Диагностический partition consumer исправлен и проверен отдельным native-маршрутом без изменения production; независимые native geometry/links и контролируемый A/B пока not verified. VS Run оставлен с результатом, сохранение PLN/Docs/generated/commit/push не выполнялись; остальные задачи и API-кэши сохранены.

- [/] Приоритет владельца #272: сначала 3D-тела и coordinate-search, AC25 Windows. Довести существующий run-local snapshot/component/bounds cache и исключить повторные поиски с ТОЧНО одинаковыми XY/Z/type/filter, включая ошибки/пустые; проверить семантику выбранного тела SDK, ключи, lifetime и counters. Остальные API-кэши отложены до этого этапа. Без изменения matcher/material/writers/плоскостных допусков и параллельных API. Сейчас VS Run; сессию не останавливать без разрешения, build/runtime пока blocked. Next Step: targeted implementation и regression/static validation; затем canonical build и измерение после освобождения отладчика. Docs/generated/commit/push не выполнять.

- [/] BLOCKED_INPUT: Unsupported beam geometry51037AF1-1476-4A3A-BB18-4201022519C0 (#252,AC25Windows). Live чтение указанного GUID вернуло headerError-2130313115; snapshot отсутствует. ЗонаA3FBEBE6 read-only partitionComplete=true/37pieces,beamFinish/beamErrors пусты. Новый RoomBook на текущем проекте завершил расчёт63комнат за135.985409с, ветка Unsupported beam geometry не достигнута; создание не заявляется. Новый код балки не менялся,build не запускался. Scope/старые tolerance-правки сохранены. Next Step: получить модель/запуск с этой балкой или актуальный отказ; не ослаблять предикаты без RED. Своя точкаRoombook:936 удалена, четыре существующие восстановлены, VS Run. Evidence:scratch/beam51037_{beam,zone,partition,results}.json и beam-fix-current-roombook.json. Docs/generated/commit/push нет.

- [/] #272 — ускорение 3D RoomBook, AC25 Windows: https://github.com/kuvbur/AddOn_SomeStuff/issues/272. Scope: общий кэш снимков по geometry GUID на один read-only этап запуска (ошибки/пустые также), переиспользование между зонами/discovery/slab; кэш компонентов внутри тела; лёгкие поисковые bbox-границы с разрешённой владельцем float-точностью после проверки координат. Точные контуры/материалы/writers/модель не менять, API не распараллеливать. План: SDK → реализация → format/LSP → canonical build → счётчики/геометрия/время на одинаковой модели. Сейчас VS Break в PartitionRoomSurfaces: сборку/перезапуск не выполнять и сессию не останавливать. Next Step: реализация и статическая проверка; runtime/build после освобождения отладчика. Last Checkpoint: нового нет; commit/push/Docs/generated без отдельной команды не выполнять. Остальные незавершённые задачи сохранены.

- [/] Z9 #252 diagnostic: минимальный read-only RoomGeometryDiagnostic по optional favoriteName; прямой ACAPI_Favorite_Get с memo, экспорт типа/library identity и scalar numeric GDL params/размерностей массивов. Без изменения модели/defaults, fallback и UnhideUnlock; source/clang-format/LSP проверить отдельно, build/runtime выполняет родитель.

- [ ] По отдельному решению владельца исследовать точное создание двух целых диагональных контактов в зоне41BE148A и отсутствующего отделочного проёма4F6A150D. Прямоугольный writer их отвергает без приближённого заполнения и без случайной части стены; проверенная замена дробления для ортогональных контуров перенесена в Archive/«Одна стена с проёмами».

- [/] Завершить Z9 по команде владельца «Закончи починку построения откосов»: отдельные openingWalls передаются только в подготовку откосов, surface-вырезы не создают повторных откосов. Блокировка сборки/перезапуска устарела: текущие исходники, APX и обе PLN совпали с manifest финальной canonical AC25 сборки Archive/«Одна стена с проёмами»; свежее read-only чтение подтвердило live inventory456, native-геометрию всех335 результатов и links, совпадающие с повтором. Артефакт scratch/z9-reveal-final/live-readback.json. Адресный native-аудит подтвердил RED: на24окнах верхние стены Wall-favorite fallback направлены противоположно боковым; depth350мм сохраняется, знак неверен. Evidence: scratch/z9-reveal-final/upper-direction.json/report.json. Направление исправлено: дополнительный разворот OtdWall_Draw_Wall удалён;canonical AC25 build,24RED→GREEN в двух RoomBook,75/75selftests и335→335без дублей PASS (Archive/Z9 direction). Остаётся отдельно классифицировать5строк без верхнего контакта,нестандартную глубину и параметрические/материальные пробелы. #252: https://github.com/kuvbur/AddOn_SomeStuff/issues/252#issuecomment-6130725060. Материальные правила и полная параметрическая глубина пока not verified; Z9 не закрыт. В этом ходе VS был Design и Archicad отсутствовал;после сборки запущен один VS debug экземпляр с тестовой PLN,результат повтора оставлен открытым без сохранения. Четыре трапеции и диагональные грани вне scope;commit/push не запрошены.

- [x] Reader, снимок OtdRoom, read-only matcher/partition и beamPart проверены на AC25; завершённые reader-факты в Archive.

- [x] Production OtdWall/OtdSlab подключены к существующим правилам материалов, избранного и writers; площади учитывают уклон, высотные полосы остаются на плоскости, проёмы повторно не вычитаются.

- [x] clang-format, fresh direct LSP, canonical AC25 build; реальный RoomBook, независимое чтение тел/свойств/слоя и повтор без сохранения PLN выполнены.

- [x] Исправить три крупные пропущенные стеновые грани: разложение вырезов/отверстий, canonical AC25 build, self-tests 50/50, оба запуска и независимое покрытие относительно исходных контактов проверены.

- [x] Исправить отделку балки у профильной стены: грани пересекаются с фактическим сечением 3D-зоны вместо нижнего beamPart-контура. AC25 canonical build success; self-tests 52/52, новые оба ok в Отладка; runtime/повтор 63→63, все шесть контактных плоскостей двух балок прошли независимые missing/excess проверки, прежние три стеновые грани также PASS. Подробности в Archive.

- [ ] Три торцевых участка многослойных стен: определить контактные слои и назначение материала без произвольного base_flipped; полный охват стен пока не достигнут.

- [x] Пункт 3: same-source coplanar union перед прежним sweep; AC25 canonical build, self-tests60/60 (8 новых), native runtime63→63 и независимое bodywise-сравнение всех ориентированных контуров при1e-6 м PASS, покрытие стен/балок PASS. Подробности в Archive.

- [/] Пункт 2: все28 in-scope пропусков классифицированы; синхронные source-снимки отвергли ложный owner60/71. Четыре трапеции2.375999498м² требуют согласованного расширения прежнего wall writer; две микрополосы ниже неизменного порога. Неизвестным основаниям GUID не назначать. Три торца вне scope; полное закрытие пункта2 не заявляется.

- [ ] Остатки SDK-error/deleted/multiple-body и остальных неподдержанных геометрий/избранного — отдельным этапом.

- [x] По команде владельца добавлены русские комментарии, доказан comment-only характер правок, актуализированы и независимо проверены scoped Docs/generated, выполнены final AC25 build/load и свежие live self-tests60/60. Результат — текущий checkpoint Z7, подробности в Archive; без ReNum.cpp, push и закрытия #252.

## Decisions

- #271 https://github.com/kuvbur/AddOn_SomeStuff/issues/271 — OPEN, тело/метки прочитаны обратно; поиск Mesh под стенами и многослойный наклонный пол, Roof рекомендована для прототипа. Issue создана без production-правок/build/runtime; следующий шаг — прототип по отдельной команде. 2026-10-13 Mesh→наклонный пол, AC25 Windows: владелец согласовал несколько элементов, по одному плоскому элементу на плоскость сетки, и предложил рассмотреть Roof вместо Shell. SDK25 подтверждает coordinate search для API_MeshID, composite Shell/Roof и API_PlaneRoofID с XY-полигоном, angle/baseLine/posSign. Рекомендован прототип односкатной Roof как более прямое представление плоской грани; окончательная замена Shell и production-реализация ещё не выполнялись. Native Mesh reader, нижняя контактная поверхность Roof/порядок слоёв/стыки/отверстия/учёт и повтор пока not verified; build/runtime не выполнялись. Next Step: native-прототип односкатной многослойной Roof по реальной mesh-грани после согласования запуска; issue до production-правок. Подробности: skill lightrag/references/somestuff-mesh-shell-ac25.md. Production/модель/активные задачи/Scope сохранены; нового checkpoint/commit/push нет.

- Доведение только контура пола до ядра/первого отделочного слоя: создана https://github.com/kuvbur/AddOn_SomeStuff/issues/268, CLOSED; краткое описание, схема реализации и критерии проверки прочитаны обратно через gh. Отделочные стены используются только как источник данных, их геометрия не меняется. Дубли по запросам «пол ядро», «пол многоугольник», «ядро», «смещение пола» не найдены. Следующее действие по #268 — реализация и проверка AC25 Windows; в текущем запросе только оформление issue, C++/модель/сборка не затронуты, checkpoint не создан, Z9 остаётся активным.

- Исследование доведения пола до ядра (AC25 Windows, production не изменён): использовать уже читаемый полный состав основания при Param_SetToBase/Param_SetComposite и сохранить отдельное смещение в OtdWall до фильтрации по l[1:/h[0] и добавления состава избранного. Сумма толщин от стороны комнаты ДО первого Core, без толщины самого Core; при отсутствии Core — глубина первого Finish. Конечный base_composite для этого непригоден: он содержит только выбранные материальными правилами слои. Корректировать только совпавшие со стеной участки границы пола на его отметке, не общий offset всего контура; потолки/исходную зону/основания не менять. ProcessSlabFinishes сейчас идёт до ProcessWallFinishes, поэтому коррекция требует отдельного этапа после подготовки стен; OtdWall_Delim_One должен сохранять смещение. Next action этого отдельного расширения: issue/dedup, минимальная реализация и геометрические regressions; сдвиг рёбер и runtime пока not verified. Текущий Z9 и Last Checkpoint не изменены.

- Native AC25 Inside/flipped=false строит тело слева от линии. Адаптер ориентирует левый бок внутрь зоны; slantAlpha=atan2(1,-dot(riseXY,rightXY)). Обе семантики подтверждены полным чтением созданных тел, а не численным тестом угла.

- Точечный допуск 1e-6 м не менялся; проверка площади представимого параллелограмма учитывает погрешность точек вдоль периметра. Исправлен нестабильный отказ одной грани при повторе 51→52; финальные запуски оба 52.

- Избранное и правила назначения материала сохранены; толщина slab-отделки может быть толщиной избранного (в фикстуре 0.131 м), не обязательно 1 мм. Нового writer/Morph/GDL нет.

- Переиспользуемые геометрические уроки: skill lightrag, references/somestuff-surface-finish-ac25.md. Scoped Docs/generated обновлены перед текущим checkpoint по явной команде владельца.

- Плинтусы оформлены в #270: https://github.com/kuvbur/AddOn_SomeStuff/issues/270 (OPEN, тело/метки прочитаны обратно). Краткое описание обоих видов, реальные стыки только созданной отделки, схема Draw_Elements → ReadRoomSurfaces → Polygon2D::CutElem, погонные количества и проверки; AC25 Windows. Дубли по «плинтус», «skirting», «карниз», «стык отделка» не найдены. Issue создана до реализации; следующее действие по отдельной команде — прототип контактов и выбор типа/избранного плинтуса. C++/модель/build/runtime не затронуты, checkpoint/commit/push нет.

- Расширение требования плинтуса: создавать также потолочный плинтус только по реальному стыку нижней обращённой в помещение поверхности СОЗДАННОГО аддоном потолка с созданной отделочной стеной. Нет потолка в зоне/на участке либо нет 3D-контакта — нет потолочного плинтуса; верх зоны/стены не fallback. Напольный и потолочный маршруты используют общий алгоритм конечных контактов с отверстиями, но отдельные результаты/погонные количества и настройки. Scope текущего обсуждения — исследование, реализация не начата; следующее действие — read-only прототип обоих видов контактов, build/runtime not verified. Активная задача и checkpoint не изменены.

- Исследование плинтуса AC25 Windows: окончательное требование владельца — только реальный стык СОЗДАННОГО аддоном пола с созданной отделочной стеной; нет пола в зоне/на участке или нет 3D-контакта — нет плинтуса, без fallback на контур зоны/низ стены. XY отсекать Polygon2D::CutElem(Sector, Boundary_Closed) с отверстиями; Z определяется общим 3D-стыком, не произвольным OtdWall.zBottom. SDK25 CorePolygon2D.hpp:3011 подтверждает InsideOrOn; :3406 IntersectElemBorderOnly ищет совпадающие граничные части, не внутренние линии. Лучшее место окончательного построения — отдельный этап Draw_Elements после успешного создания полов, стен и проёмов в существующей undo-команде; предварительный расчёт после ProcessWallFinishes возможен, но не доказывает создание/контакт. Погонную длину не смешивать с площадями. Подробности: C:/Users/da-rogojin/AppData/Local/hermes/cache/scratch/skirting-polygon2d-ac25-research.md; reusable reference в skill lightrag. Scope — исследование, C++/модель не менялись; build/runtime/скорость not verified. Следующее действие по отдельной команде — read-only прототип реальных стыков созданных элементов, затем issue перед реализацией; активный Z9 и checkpoint18d05b7 не изменены, commit/push нет.

## Archive

Завершённые задачи перенесены в `IDEA_ARCHIVE.md`:
- #278 — отписка наблюдателей (DetachObserver в Helpers/ReservationChangeHandler/APINotify)
- #277 — вылеты Undo.dll при переключении окон и в ElementEventHandlerProc
- #276 — неверная высота потолка-объекта (level от уровня этажа, ac_ref_height=0)
- #275 — совместимость SDK AC26–30
- #274 — очистка коллинеарных вершин полигонов полов и потолков
- #273 — очистка старых preferences проекта
- #269 — сигнатура AdvanceProcessPhase в CommonFunction
- #268 — доведение контура пола до ядра базовой стены
- #267 — поиск черновых полов и потолков по низу и верху стен
- #192 — регрессия Object-избранного пола/потолка
- Локальная приёмка углов стен (API_WallPart), срез документации Roombook 2026-10-15, расследования падений Undo.dll, этапы Z7–Z9.

## Грабли

- **ID контрола в `'GDLG'` = позиция строки, а не число в комментарии.**
  `/* [ n] */` — пояснение для человека, компилятор его не читает: кнопка,
  объявленная раньше списка, получает ID 3, а список — ID 4. Симптомы
  неочевидые: список пуст (нет заголовков/строк), а подпись из `DGSetItemText`
  уходит «в никуда», и кнопка показывает дефолтный ресурсный текст.
  Компилятор ресурсов такое НЕ диагностирует — молчит. Всегда держать
  `[n]` по возрастанию в порядке строк; при добавлении кнопки в середину —
  дописывать её последней строкой. То же для `'DLGH'`.
- **Выделение в 3D-окне требует СНАЧАЛА перехода в это окно.**
  `ACAPI_Selection_Select` / `ACAPI_Element_Select` выделяют элементы только
  ТЕКУЩЕЙ БД: до перехода в 3D элементы чужой базы выделить нельзя
  (документирован `APIERR_BADDATABASE`). Рабочий порядок для показа элементов
  из другой базы: `ShowAllIn3D` → выделение → подсветка → `ZoomToElements` →
  redraw. Подсветка цветом, в отличие от выделения, работает и для элементов
  чужой базы (контракт — «in the 2D … and 3D window»), поэтому она и нужна
  поверх выделения. Отказ выделения/зума не должен прерывать остальные шаги.
  Ошибка воспроизведения: «выделить GUID'ы, потом уйти в 3D» — выделение
  срабатывает в плане, но в 3D элементы не выделены; «сначала перенести окно»
  — элементы становятся недоступны вовсе.
- **Собранный `.apx` может содержать устаревший ресурс.** `AddOn.grc`
  генерируется из `Tools/AddOn.grc.in` на этапе конфигурации CMake, поэтому
  правка шаблона без пересборки не попадает в аддон; при отладке GUI всегда
  сверять mtime `.apx` и `ResourceObjects/AddOn.grc.rc2` с текущим временем.
  Побочный эффект: временная подмена шаблола ради проверки гипотезы
  «сбивает» сгенерированный `.grc` — восстановить нужно и то, и другое.
- **HTML вшит в ресурс** (`'DATA' ID_ADDON_HTML`) — правки `Interface_ru.html`
  требуют пересборки; после каждой правки `Tools/test_html.ps1`.
- **Дубли строк классификации**: C++ кладёт словарь и под `systemname`, и под
  `systemname_full` → записи ×2 (JS-дедуп `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить
  ко всем» при единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — настройки только в локальный
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile`
  отвергает чужую `PreferencesVersion` → новый массив в настройках требует bump
  версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов;
  док DevKit требует одну область на действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase +
  BRACEEND) — имя без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**; аргументы
  `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>`
  ронял ArchiCAD. Ответ моста — строка JSON: вложенные `DG::JSObject`/`JSArray`
  при передаче в JS теряются.
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID`
  триггерят SelectionChangeHandler → `static suppressSelectionRefresh`.
  `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`,
  сброс = вызов без par1, Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, значения — по
  элементам отдельно; в кэше `property` — только определения.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`;
  Reviews/ в .gitignore (BOM+LF, править через python `utf-8-sig`).
- **Постоянный MSBuild-узел VS держит `.tlog`** (`nodeReuse:true`,
  `MSB4018 … read.1.tlog`) — процесс живёт часами и при этом **не** означает
  идущую сборку: проверить `Get-CimInstance Win32_Process` по `CommandLine` и
  просто повторить `BuildAddOn.py`, а не ждать и не убивать узел.
- **Править `_generated/*.json` можно только вручную**: генератор обнуляет
  `callgraph.json`, а `json.dump(..., indent=2)` из Windows-скрипта даёт CRLF —
  стиль HEAD: LF, без BOM, **без завершающего LF**.
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`;
  пересоздание элементов отделки в Roombook; `Sync.cpp:419-428`
  накопительный `epm`.
- **MSVC не выдаёт часть диагнозов clang — локальная зелёная сборка Windows
  не означает, что mac-сборка в CI зелёная.** mac собирает с `-Werror` и
  расширенным набором `-W…`, поэтому диагностики, молчащие в MSVC, роняют
  mac-job'ы. Поймано два диагноза подряд: `-Wunused-lambda-capture`
  (`SpecPlanning.cpp:283`) и неоднозначный `operator=` при `GS::Array = {}`
  (`CommonFunction.cpp:2769`). Windows при этом зелёная во всех пяти версиях —
  и это выглядит как проблема кода под macOS, хотя дело в инструменте.
  **`GS::Array` очищать только `Clear ()`, не `= {};`:** у него есть шаблонный
  `operator=` через `ConversionEnumerator`, и пустой список инициализации
  неоднозначен. `= {};` допустимо для POD-структур и `time_point`.
- **Имитация mac-сборки через clang-cl не работает — не тратить на неё время.**
  Проверено: у `clang-cl` нет части флагов mac (`-std=gnu++20` → `/std:c++20`),
  а локальные заглушки `Sources/AddOn/api_headers/` (только под `_MSC_VER`, в
  mac-сборку не входят) дают диагностики `-Wreserved-macro-identifier`/
  `-Wc++98-compat`/`-Wundef`, которых в mac-логе нет. Отличить свои артефакты
  от дефектов не вышло. Такие классы ошибок надёжнее закрывать по логу CI.
  Если проверка всё-таки нужна, `-W…` нельзя давать после исходника (clang-cl
  считает их файлами), `/Zs` несовместим с `-c`, PCH из БД на диске
  отсутствует, `-D вида -DADDON_NAME=\"X\"` нельзя переразбирать `shlex`
  (кавычки теряются → `missing terminating '"'`), нужны `-DACExtension`,
  `-DAC_25`, `/EHsc` (иначе падает `exprtk.h`).
