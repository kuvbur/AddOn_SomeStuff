# Состояние работы — документирование кодовой базы

## Обновления по задачам
- Документация (2026-09-25, рабочее дерево `llm_test`, HEAD `13c1948`): самостоятельный проход по `Docs/`. (1) Новая карточка `Docs/modules/json_commands.md` для `json_commands/` (модуль AC25+ #205/#206/#207/#208, не был задокументирован): `CommandBase`/`ReadOnlyCommand`/`ModifyCommand`, `RegisterJsonCommands`, `RoomBookCommand`, `SpecCommand`, `HealthCommand` (шаблон, не регистрируется); все строки .cpp проверены grep. (2) `symbols.json` пересобран regex fallback — 8549 символов (было 8544; добавлен `dialogs/OtherDbDialog.cpp`, учтён `json_commands/`); `callgraph.json` сохранён (152→155 рёбер: добавлено `RegisterJsonCommands`→`RoomBookCommand`/`SpecCommand` и `Initialize`→`RegisterJsonCommands`, call_sites 1-based). (3) `REPOMAP.md` и `ARCHITECTURE.md`: добавлены `json_commands/` и `dialogs/OtherDbDialog.*` в структуру, слои и Mermaid-граф; исправлена статистика compile_commands.json (31 запись, 1464 `/I` — было «210/0 include»). (4) `SomeStuff_Main.md`: добавлены `RegisterInterface`/`Initialize`/`FreeData`, карточка `Initialize` (с `RegisterJsonCommands` :569), карта вызовов `MenuCommandHandler` обновлена под дерево `llm_test`; `Sync.md` — шапка (#203/#210) + include `OtherDbDialog.hpp`; `Helpers.md` — строка `GetAttributeValues` (определение .cpp:9577, hpp:748; фильтр `fromAttribDefinition`, #209 open). Не тронуты: code-файлы, `IDEA.md`, `DISCREPANCIES.md` (расхождений не найдено).
- #207/#208/#209 (2026-09-25, рабочее дерево `llm_test`, перед checkpoint): пустое выделение с default-правилом теперь доходит до `SpecArray`; JSON Spec требует `placementPoint`, не открывает интерфейс, возвращает код/счётчики; расширение фильтра `GetAttributeValues` на `fromPropertyDefinition` отменено (#209 — open, фильтр возвращён к `fromAttribDefinition`). Финальный runner AC25 собрал аддон и открыл `test_25.pln`; JSON-вызов создал 34 объекта (548→582), в последних 34/34 заполнено «Спецификации материалов/Наименование в объект». `symbols.json` пересобран regex fallback (8544 символа), `callgraph.json` сохранён. AC26–29 и смешанное создание/изменение — не проверены.
- #206 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): добавлена `SomeStuffCommand.Spec`, которая загружает текущие `SyncSettings` и вызывает `Spec::SpecAll` на главном потоке. `Docs/modules/spec/Spec.md` обновлён; clang-format, clangd 0 и финальный runner AC25 успешны. HTTP-вызов вернул `status="returned"`, `elapsedSeconds=318.3851546`; корректность созданной спецификации отдельно не проверялась.

- #205 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): восстановлена минимальная инфраструктура `API_AddOnCommand`, зарегистрирована `SomeStuffCommand.RoomBook`. Финальный runner AC25 успешен; два последовательных HTTP-вызова вернули `status="returned"` за 17.5131455 и 16.9828768 с неизменными итоговыми количествами элементов на втором расчёте.

- #193 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): исправлено направление словаря в `BuildOtdByParent`: внешний GUID из `SyncGetSubelement` остаётся базовым элементом, внутренний GUID отделки используется для определения `TypeOtd`. Синтетический тест AC25 показал 3 RED до исправления и 5 GREEN после, полный TESTING-набор — 0 `ERROR IN TEST`; финальный runner успешен. На свежем `test_25.pln` два последовательных JSON-запуска RoomBook дали стабильные количества Object=599, Wall=487, Slab=9, Beam=63; второй запуск стен удалил и создал по 430 GUID без роста количества. Обновлены `Roombook.md`/`TestFunc.md`; `symbols.json` пересобран regex fallback (8514 символов), `callgraph.json` сохранён байт-в-байт.

- #203 (2026-09-24, рабочее дерево `llm_test`, без checkpoint): `RunParam` после успешного parameter script окна/двери определяет тип через кросс-версионный `GetElemTypeID(element)` и запускает script marker из непустого `openingBase.markGuid`; GUID marker сначала разворачивается в `API_Elem_Head`. `Sync.md` обновлён; `symbols.json`/`callgraph.json` не менялись — сигнатуры и связи функций не изменены. LightRAG не дал контекст для `RunGDLParScript`/`markGuid` во всех режимах; fallback DevKit AC25/29 подтвердил поле и старый API contract. clangd 0, `BuildAddOn.py -v 25` success; runtime marker — не проверено.

- #199 (2026-09-24, рабочее дерево `llm_test`, без коммита): `SyncCalcRule` обходит внешний целевой GUID из `WriteDict`; синтетический TESTING-кейс AC25 показал RED→GREEN, тесты прошли по маркерам панели «Отладка». `BuildAddOn.py -v 25` и `restart_archicad_for_test.ps1` завершились успешно; фактическая запись на PLN — не проверено. Обновлены `Sync.md`/`TestFunc.md`; `symbols.json` пересобран только для этих двух файлов из regex fallback (8488 записей), сохранены записи Roombook и весь callgraph.

- #195 (2026-09-24, рабочее дерево `llm_test`, перед checkpoint): `RoomBook` разделён на контекст, сбор элементов, чтение параметров, обработку отделки, свод/запись материалов, сбор GUID на удаление и резервирование (см. `Docs/modules/Roombook.md`). `symbols.json`: regex fallback выполнен (8514 символов), `callgraph.json` сохранён байт-в-байт. clang-format, clangd 0, финальный AC25 runner и два последовательных runtime-вызова RoomBook успешны; второй расчёт сохранил количества Object/Wall/Slab/Beam без накопления элементов.

## Текущая ветка
Документация `Docs/` ведётся на `docs/codebase-map` (AGENTS.md §17); рабочий код — `llm_test`.

## Хеш коммита
`72ec74a` (2026-09-22) — фазы 0-2 задокументированы и закоммичены; HEAD `llm_test` — `13c1948` (2026-09-25)

## Список модулей

| Модуль | Каталог | .cpp файлов | .h/.hpp файлов | Примерная сложность |
|--------|---------|-------------|----------------|---------------------|
| Core/Helpers | Sources/AddOn/ (Helpers.cpp/hpp) | 1 | 1 | Большой |
| Propertycache | Sources/AddOn/Propertycache.cpp/hpp | 1 | 0 | Средний |
| Sync | Sources/AddOn/Sync.cpp/hpp | 1 | 1 | Большой |
| SomeStuff_Main | Sources/AddOn/SomeStuff_Main.cpp/hpp | 1 | 1 | Маленький |
| CommonFunction | Sources/AddOn/CommonFunction.cpp/hpp | 1 | 1 | Большой |
| Dimensions | Sources/AddOn/Dimensions.cpp/hpp | 1 | 1 | Маленький |
| MEPv1 | Sources/AddOn/MEPv1.cpp/hpp | 1 | 1 | Большой |
| ReNum | Sources/AddOn/ReNum.cpp/hpp | 1 | 1 | Средний |
| Roombook | Sources/AddOn/Roombook.cpp/hpp | 1 | 1 | Огромный |
| Summ | Sources/AddOn/Summ.cpp/hpp | 1 | 1 | Маленький |
| ClassificationFunction | Sources/AddOn/ClassificationFunction.cpp/hpp | 1 | 1 | Маленький |
| Constants | Sources/AddOn/Constants.hpp | 0 | 1 | Справочник |
| TestFunc | Sources/AddOn/TestFunc.cpp/hpp | 1 | 1 | Большой |
| dialogs/BrowserPalette | Sources/AddOn/dialogs/BrowserPalette.cpp/hpp | 1 | 1 | Большой |
| dialogs/CommandHelpers | Sources/AddOn/dialogs/CommandHelpers.cpp/hpp | 1 | 1 | Маленький |
| dialogs/DG4rule | Sources/AddOn/dialogs/DG4rule.cpp/hpp | 1 | 1 | Маленький |
| dialogs/SyncSettings | Sources/AddOn/dialogs/SyncSettings.cpp/hpp | 1 | 1 | Большой |
| spec/Spec | Sources/AddOn/spec/Spec.cpp/hpp | 1 | 1 | Огромный |
| spec/Spec_libpart | Sources/AddOn/spec/Spec_libpart.cpp/hpp | 1 | 1 | Маленький |
| table/TableRenderer | Sources/AddOn/table/TableRenderer.cpp/hpp | 1 | 1 | Средний |
| table/TablesNavigator | Sources/AddOn/table/TablesNavigator.cpp/hpp | 1 | 1 | Большой |
| pk/AutomateFunction | Sources/AddOn/pk/AutomateFunction.cpp/hpp | 1 | 1 | Средний |
| pk/ResetProperty | Sources/AddOn/pk/ResetProperty.cpp/hpp | 1 | 1 | Маленький |
| pk/Revision | Sources/AddOn/pk/Revision.cpp/hpp | 1 | 1 | Средний |
| third_party/qrcodegen | Sources/AddOn/third_party/qrcodegen.cpp/hpp | 1 | 1 | Большой |
| json_commands | Sources/AddOn/json_commands/*.cpp/hpp | 5 | 5 | Маленький (AC25+) |

## Статус фаз

- [x] Фаза 0: Разведка (clangd MCP работает, compile_commands.json найден, модули выделены)
- [x] Фаза 1: Символы собраны через скрипт (docs/tools/generate_symbols.py → _generated/symbols.json)
- [x] Фаза 2: Пилот — модуль pk/ задокументирован в docs/modules/pk.md
- [ ] Фаза 3: Остальные модули
- [ ] Фаза 4: Обзор
- [ ] Фаза 5: Сверка

## Порядок документирования (Фаза 3)
1. pk/ — наибольший входящий спрос (вызывается из Many)
2. dialogs/BrowserPalette — точка входа UI
3. dialogs/SyncSettings — настройки
4. spec/Spec — правила
5. table/TableRenderer — таблицы
6. table/TablesNavigator — навигатор
7. dialogs/CommandHelpers
8. dialogs/DG4rule
9. spec/Spec_libpart
10. ReNum
11. Summ
12. CommonFunction
13. MEPv1
14. ClassificationFunction
15. Roombook
16. SomeStuff_Main
17. TestFunc
18. Propertycache
19. Sync
20. Dimensions
21. third_party/qrcodegen
22. Constants (справочник)

## Замечания
- compile_commands.json покрывает 25 .cpp файлов (не .h/.hpp)
- Связь между модулями: Helpers → Propertycache → Sync → Core
- BrowserPalette использует JS bridge для HTML-интерфейса
- Roombook — самый большой модуль (5686 строк в Namespace)

## Инфраструктура (FIX: compile_commands.json + callgraph)

### Диагностика (Step 1)

**compile_commands.json СОДЕРЖИТ include-пути:**
- Всего записей: 25 (1 cmake_pch + 24 source)
- С полем `command` (string): 25/25
- С полем `arguments` (array): 0/25
- Всего `/I` флагов: 1464
- Уникальных путей: 61
- **Отсутствующих путей: 0** (все существуют на диске)

**clangd диагностика Helpers.cpp:**
- Errors: 20 (все "Expression result unused" / "Unused variable" — стандартные warning'ы компилятора)
- Warnings: 3 (unused-includes)
- **"file not found" errors: 0** — clangd корректно резолвит все заголовки

### Причина пустого callgraph.json

Не в compile_commands.json (он корректен). Причина: `docs/tools/generate_symbols.py` НЕ реализует сбор callgraph — всегда записывает `[]` в `callgraph.json`. Необходимо добавить `textDocument/callHierarchy` запросы.

### Корень проблемы: CMake
- `CMakeLists.txt` line 23: `set(CMAKE_EXPORT_COMPILE_COMMANDS ON)` — включено
- `Tools/BuildAddOn.py` имеет `--lsp` флаг: генерирует compile_commands.json через CMake и копирует в корень (BuildAddOn.py lines 56-88)
- Текущий `compile_commands.json` был сгенерирован CMake в `Build/LspCompileCommands/25/` и скопирован в корень
- Сгенерированный файл использует `command` (string), не `arguments` (array)

### Версия
- AC25 (DevKit-25): `D:/SomeStuff_addon/Build/DevKit/APIDevKit-25/`

### Что исправлено / как починено (Steps 2–4)

**compile_commands.json: исправление НЕ потребовалось** — premise задачи (0 include-путей) не подтвердилась. Файл сгенерирован штатно: `BuildAddOn.py --lsp` → CMake (`CMAKE_EXPORT_COMPILE_COMMANDS ON`, CMakeLists.txt:23) → копирование из `Build/LspCompileCommands/25/` в корень. Проверка clangd-диагностикой Helpers.cpp: 0 ошибок "file not found" (20 ошибок — только -Wunused-value/-Wunused-variable, которые clangd трактует как errors, MSVC прощает).

**callgraph.json пустой по другой причине**: `generate_symbols.py` никогда не реализовывал сбор callHierarchy — всегда писал `[]`. Дополнительно скрипт не может общаться с clangd через subprocess.PIPE на Windows (clangd 22.1.8: нет `--port`, pipe даёт пустой greeting / Errno 22; проверено 3 способами). Сбор выполнен напрямую через clangd MCP (`get_call_hierarchy`) на позициях определений из symbols.json.

**Регенерация callgraph** (для будущих запусков): `get_call_hierarchy` на позиции имени функции в определении (колонка имени, не тела); SDK-рёбра фильтровать по пути `Sources/AddOn/`.

### Проверка (Step 3)
- Helpers.cpp: 0 "file not found" ✓
- Все 1464 `/I` путей из 24 source-записей существуют на диске ✓
- callHierarchy работает (clangd MCP): 7 из 9 запрошенных функций pk/ резолвятся

### Список неполных записей
- `ResetProperty` (ResetProperty.cpp:14), `Revision::SetRevision` (Revision.cpp:14) — clangd: "No call hierarchy available at this position"; не блокирует остальное
- SDK-рёбра (DevKit/MSVC/Windows Kits) не развёрнуты в edges — только счётчики outgoing_count

### Новая статистика
- `callgraph.json`: 29 рёбер внутри проекта (было 0), 9 функций опрошено, 7 резолвится
- `symbols.json`: 8421 символов, 17 модулей (не менялось)
- `pk.md`: добавлены поля «Вызывает»/«Вызывается из» для 9 функций AutomateFunction (не тронуты остальные разделы)

## Фаза 5: Сверка (2026-09-22, коммит f8f599c)

- [x] Фаза 3: все 23 модуля имеют Docs/modules/*.md; закоммичено (f8f599c + недостающие Dimensions/TestFunc/third_party/Helpers)
- [x] Фаза 5: 10 записей сверены с кодом — см. Docs/DISCREPANCIES.md
- Итоговый отчёт: см. ниже

### Итоговый отчёт (фаза 5)

- Модулей задокументировано: 23 файла в Docs/modules/ (включая вложенные dialogs/, spec/, table/, third_party/)
- Покрытие symbols.json (8421 символ, 17 модулей): пофункциональные записи приведены для pk/ (шаблон с «Вызывает»/«Вызывается из»), таблицы API — для остальных
- **Неполное покрытие** (честная пометка): Helpers (2145 символов) и TestFunc (682) — только ключевые типы и таблица API из заголовочных комментариев, без пофункциональных записей; third_party — внешняя библиотека, не документируется пофункционально
- `[не проверено]`/not verified записи: ResetProperty (callHierarchy), Revision::SetRevision (callHierarchy), Helpers.cpp:1761-мемо (ссылка на ревью, статус «исправлено» подтверждён grep'ом)
- Файлы вне compile_commands.json (не индексируются clangd): заголовки без единицы трансляции (все .hpp/.h, Constants.hpp) — индексируются как заголовки через включения; отдельной проблемы не зафиксировано
- Расхождения: 2 активных косметических (TestFunc.hpp:14, Sync.hpp:11), 5 исторических подтверждены исправленными — Docs/DISCREPANCIES.md

### Статус фаз (обновление)
- [x] Фаза 3: модули задокументированы и закоммичены
- [x] Фаза 4: ARCHITECTURE.md, REPOMAP.md созданы; Mermaid-граф зависимостей — см. ARCHITECTURE.md; сценарии — список предложен, ожидает согласования
- [x] Фаза 5: сверка выполнена, DISCREPANCIES.md заполнен

## Доработка пилотного pk.md по замечаниям (2026-09-22)

- Проблема 1: все описания в таблицах и карточках pk.md помечены [из комментария]/[по коду]/[не проверено]; `not verified` заменено на `не проверено`
- Проблема 2: добавлены 16 карточек нетривиальных функций (5 AutoFunc с побочными эффектами, 6 ResetProperty-семейства, 5 Revision); в карточке ResetProperty зафиксирован баг AC27+ (return false) как актуальный
- Проблема 3 (проверено): строки в прежней таблице были строками .hpp; реальные определения в .cpp (1-based): GetNear 15, GetCuplane 34, Get3DProjectionInfo 149, Get3DDocument 208, GetSectLine 260, DoSect 406, PlaceDocSect 489, ProfileByLine 556, AlignOneDrawingsByPoints 771, GetDrawingsSort 892, AlignDrawingsByPoints 926 — подтверждены grep'ом по AutomateFunction.cpp и согласуются с clangd callHierarchy (MCP отдаёт 0-based)
- ResetProperty/SetRevision: clangd callHierarchy не резолвится (2 столбца каждый) → помечено `не проверено` в карточках

## Применение исправленного шаблона ко всем модулям (2026-09-22, после приёмки пилота)

- Все 23 модуля Docs/modules/*.md переписаны по принятому шаблону пилота:
  пометки источника [из комментария]/[по коду]/[не проверено] на каждом описании;
  номера строк — определения в .cpp (проверены grep для Summ, Sync, ReNum, Spec, SomeStuff_Main, CommonFunction, Propertycache; для остальных строка помечена «не проверена» вместо догадки);
  карточки нетривиальных функций с побочными эффектами (2-5 на модуль: SyncAndMonAll/SyncData/SyncElement, SumSelected/Sum_OneRule, ReNumSelected/ReNumOneRule, SpecAll/PlaceElements, Update/GetPropertyRuleFlag, UnhideUnlockElementLayer/EvalExpression, SetAutoclass, SelectionChangeHandler/Show, GetSyncSettingsCache, TableRenderer::Draw/ComputeLayout, DimParse/DimAutoRound)
- Helpers и TestFunc — честно помечены как неполное покрытие (2145/682 символов) в шапке и здесь
- Чего не хватает: callHierarchy для модулей кроме pk (сбор через clangd MCP не выполнялся) — в карточках соответствующие поля помечены [не проверено]

## Grep-fallback для callHierarchy (2026-09-22)

- clangd callHierarchy/find_references на позициях определения ResetProperty.cpp:14 и Revision.cpp:14 не резолвятся (callHierarchy: "No call hierarchy"; find_references: либо пусто, либо 719 чужих ссылок из DevKit-заголовков — неверный символ, по 2 попытки на каждый)
- Fallback: grep по Sources/AddOn (по предложению пользователя):
  - `ResetProperty()` вызывается из `SyncAndMonAll` (Sync.cpp:174, при успехе сброса — ранний выход)
  - `Revision::SetRevision()` вызывается из `MenuCommandHandler` (SomeStuff_Main.cpp:449, case SetRevision_CommandID)
  - ChangeMarkerText/ChangeMarkerTextOnLayout/ChangeLayoutProperty внешних вызовов не имеют (только внутри pk/Revision.cpp)
- Обновлены карточки ResetProperty и SetRevision в pk.md

## Финальный отчёт (2026-09-22, задача документирования завершена)

- Grep-карта вызовов входных функций (все проверены по коду):
  SumSelected ← Main:433; SyncAndMonAll ← Main:405; SyncSelected ← Main:410 (+Sync.cpp:565/2246); RunParamSelected ← Main:437; SyncShowSubelement ← Main:445; ResetProperty ← Sync.cpp:174; SetRevision ← Main:449; SyncSetSubelement ← Main:453; SpecAll ← Main:441; ReNumSelected ← Main:427; ProfileByLine ← Main:462; AlignDrawingsByPoints ← Main:465; MonAll ← Main:400/555; DimRoundAll ← Main:128/165/489; GetAllClassification ← Propertycache.hpp:613; ReadMEP ← Helpers.cpp:5747 (+Propertycache.cpp:149)
- Исправлены номера строк SomeStuff_Main: MenuCommandHandler=366, ProfileByLine=462, AlignDrawingsByPoints=465 (pk.md + callgraph.json)
- Критерии готовности: модулей 23/23, описания с пометками, DISCREPANCIES.md заполнен, вне Docs/ и AGENTS.md изменений нет
- Открытые пункты (не блокируют): список ключевых сценариев в ARCHITECTURE.md ожидает согласования для sequence-диаграмм; Helpers/TestFunc — неполное покрытие; callHierarchy для остальных модулей не собирался (clangd mis-resolve на определениях — обход grep'ом)

## Решение по приоритетам: callgraph точечно (2026-09-22)

- Callgraph собран только для pk/ (29 рёбер, 9 функций). Для остальных 22 модулей «Вызывает»/«Вызывается из» помечены [не проверено] — осознанно.
- Решение: расширять callgraph НЕ на все модули, а точечно под модуль, где профилированием всплыла горячая функция (полный проход ~ тот же объём, что ручной сбор pk; профилирование скорее всего будет по отдельным горячим модулям).
- Процедура точечного сбора — Docs/tools/UPDATE_PROCEDURE.md §2.
- generate_symbols.py callgraph не автоматизирует (subprocess к clangd не работает на Windows) — подтверждено, это не недоделка, а ограничение окружения.

## Точечный callgraph Sync (2026-09-22, коммит de8e94b) — первый прогон политики

- Sync выбран как первый горячий модуль (все команды меню и события изменения элементов проходят через него)
- clangd callHierarchy: все 6 функций резолвятся (MonAll, SyncAndMonAll, SyncByType, SyncElement, SyncData, ParseSyncString). Важно: MCP ждёт 0-based строки — значения из grep (1-based) давать как line-1; col = позиция имени функции
- 49 новых рёбер; итого 78 в callgraph.json
- Открытия: MonAll вызывается не только из меню, но и из Initialize (SomeStuff_Main.cpp:555); ParseSyncString активно тестируется (TestParseSyncStringIndependent, 7 вызовов); SyncAndMonAll сначала зовёт ResetProperty и при успехе делает ранний выход
- Синхронизация координат: в callgraph.json — 0-based (clangd), в modules/*.md — 1-based (grep); правило записано в meta.lines

## Полный проход callgraph по оставшимся модулям (2026-09-22)

- Решение «точечно» перевыполнено командой пользователя: прогнан callHierarchy по всем модулям
- Опрошено 30 функций, резолвится 24; callgraph.json — 152 ребра (было 78)
- Не резолвятся (по 2 попытки, стоп): ReadMEP (MEPv1.cpp:177), RoomBook (Roombook.cpp:56), GetSyncSettingsCache (SyncSettings.cpp:417), ElementEventHandlerProc (Main:141), ResetProperty/SetRevision (известно ранее) — вызывающие известны из callHierarchy MenuCommandHandler и grep, поля в карточках заполнены с пометкой источника
- Открытия: DimRoundAll дёргается из ТРЁХ обработчиков (меню :489, ElementEvent :165, ProjectEvent :128); GetPropertyRuleFlag — только тестовые вызовы (DISCREPANCIES #9); RoomBook entry — Roombook.cpp:57; LoadSyncSettingsFromPreferences — clangd даёт SyncSettings.cpp:518, grep 429 (два совпадения)
- Обновлены: Summ, ReNum, Spec, Dimensions, ClassificationFunction, TableRenderer, Propertycache, SomeStuff_Main, SyncSettings, BrowserPalette, Roombook, MEPv1

## Пункты 2-3 закрыты (2026-09-22)

- П.2 (неполное покрытие): TestFunc — все 33 функции из hpp; Helpers — полный API по объявлениям hpp (FormatStringFunc, топ-уровень, ParamHelpers чтение/запись/конвертации, операторы); Roombook — все ~70 функций из documentSymbols по подсистемам. Остаток «неполного покрытия» снят.
- П.3 (нерезолвящиеся): тела 6 функций разобраны body-scan (Docs/tools/_body_scan.py, brace-matching + whitelist имён проекта) → Docs/_generated/body_scan.json; «Вызывает» заполнены в pk.md (ResetProperty, SetRevision), MEPv1 (ReadMEP: до AC28 false, AC28+ GetMEPData), Roombook (RoomBook — 96 вызовов), SyncSettings (GetSyncSettingsCache → ReadSyncSettings), SomeStuff_Main (ElementEventHandlerProc — диспетчер всей функциональности).
- Находки: ConvertToProperty с TODO «переписать под ParamValue» (Helpers.hpp:419); дубликат объявления CompareParamDictValue (Helpers.hpp:427/432); у ResetProperty body-scan обрезался по внешней скобке — ResetPropertyElement2Defult подтверждён чтением (:34).

## Пункт 1 закрыт: sequence-диаграммы (2026-09-22)

- Набор 10 сценариев подтверждён пользователем; диаграммы — Docs/SEQUENCES.md (Mermaid, все сообщения из callgraph.json/body_scan.json)
- ARCHITECTURE.md ссылается на SEQUENCES.md
- Задача документирования: все фазы и все открытые пункты закрыты (кроме мелких пометок [не проверено] на SDK-вызовах внутри карточек)

## Пункт 1: сценарии согласованы, sequence-диаграммы построены (2026-09-22, коммит 397acd6)

- Все 10 сценариев подтверждены пользователем; sequenceDiagram (Mermaid) добавлены в ARCHITECTURE.md — построены из callgraph.json + body_scan.json, номера строк 1-based
- Задача документирования полностью завершена: 23/23 модулей с полным покрытием, 152 ребра callgraph + body_scan для 6 функций, DISCREPANCIES (9 записей), 10 sequence-диаграмм, регламент обновления (UPDATE_PROCEDURE.md)
