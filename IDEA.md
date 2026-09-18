# Current Task

## Задача

UI вкладки «Монитор» — остаток работ. Работать по GitHub issues (см. План); целевая версия —
**AC25** (в репозитории только DevKit-25). ТЗ по интерфейсу —
`Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` (обязательное); правки HTML — только по ТЗ,
после каждой правки `powershell -File Tools/test_html.ps1`.

## План


- [x] #157 — 1.5 `GetFilterPresets` (пресеты фильтров в `SyncSettings.dat`, фактический bump `PreferencesVersion = 6`, т.к. `5` уже была в HEAD)
- [/] #155 — 1.6 `ResetPropertyToDefault` (инлайн-функция моста реализована; `Tools/test_html.ps1` прошёл; AC25 собран; ручная runtime-проверка нажатия сброса в палитре ещё не выполнена; перезапуск 2026-09-17 17:47 успешен, тесты runner: `UNKNOWN`)
- [x] #154 — `Sum_flag` фильтр в `Summ.cpp` (`Sum_GetElement`, обе ветви: `SUM_TO_INFO` и обычная), константа `SUMFLAG` в `Constants.hpp`
- [/] #158 — 1.7.1: замер GetPropertiesList под TESTING реализован и выполнен. Baseline AC25 Windows Debug: selected=10, returned=10, 162 записи свойств; 5 вызовов одного набора GUID: 25/26/24/27/27 мс, среднее 25,8 мс, медиана 26 мс. Источник: test_results.txt:5049,5247,5566,5757,6067. Локальная фильтрация HTML реализована (см. Archive). В рабочем дереве добавлены PropertyRuleFlag/GetPropertyRuleFlag (GUID + сравнение описания), использование признака в BrowserPalette и TestGetPropertyRuleFlag. Признак учитывает hasSyncRules и валидные Spec_rule из otherCommands (включая v2/v3); 12/12 проверок RuleFlag прошли в AC25 Windows Debug 2026-09-17 22:20. HTML сохраняет hasRule при агрегации и использует локальный переключатель Sync/Spec; Node RED/GREEN и HTML-проверки пройдены. В C++ хранится полный ParsePropertyResult; 4 проверки сохранения Sync/Spec, повторного использования и смены описания прошли. ReadPropertyDefinition очищает кэш правил (по исходнику); отдельный runtime-сценарий перечитывания — not verified. Повторный сопоставимый замер после кэша не получен; #158 не завершён. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158
- [/] #159 — 1.7.2: маркировка свойств с правилами SomeStuff синим в «Мониторе» (охват согласован
  с пользователем 2026-09-18: Sync/Spec/Renum/Sum — синим, остальные чёрным; отдельный признак
  Spec не нужен). Реализовано: `GetPropertyRuleFlag` = hasSyncRules ИЛИ любой валидный
  otherCommand (Propertycache.cpp); HTML `renderPropertyRow` — цвет имени = accent при hasRule
  (приоритет над allHave, совпадают по цвету). Тесты TestFunc: Renum_flag/Sum -> true.
  RED подтверждён (2 провала), GREEN: 0 ошибок, 483 : ok в AC25 Win Debug 2026-09-18 14:53.
  test_html.ps1 PASS. Визуальная проверка палитры в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/159
- [/] #179 — исправление замечаний UI/UX-ревью Interface_ru.html (12 пунктов): контраст
  `--ac-text-tertiary` (light #646D7B ≥4.49:1, dark #8A93A2 ≥5.13:1 — WCAG AA), клавиатурный
  доступ кликабельных div (focusable в `h()`: tabindex/role/Enter/focus-ring), понятный
  disabled «Применить ко всем» (подсказка «Выберите классификацию из списка»), фабрика
  `iconButton` (все иконочные кнопки едины, aria-label, transition), `hoverStyle` в `h()`
  (убраны пары mouseenter/mouseleave и баг `e.target`), кегль ≥0.7rem, отклик на подсветку
  элементов (акцент/предупреждение), SVG-шевроны вместо юникод-глифов, `DROPDOWN_STYLE`
  без дублей. `allHave` больше не красится акцентом (остался только hasRule по #159).
  test_html.ps1 PASS, node --check OK, рендер в preview OK. Интерактив в реальной палитре
  AC25 — за пользователем. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/179
- [/] #180 — «Только с правилами Sync/Spec» в «Мониторе» — пункт-переключатель в списке
  пресетов фильтра по имени (✓ + акцент в активном состоянии) вместо чекбокса; чекбокс удалён,
  дропдаун пресетов перестраивается при каждом открытии (иначе активное состояние не обновится).
  test_html.ps1 PASS, node --check OK. Визуальная проверка в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/180
- [/] #160 — 1.7.3 pin свойств: `STATE.pinnedProperties` (сессионный), кнопка-булавка
  (`pinIconSvg`, поворот на 45°) на каждой строке «Монитора» перед кнопкой сброса;
  `togglePinnedProperty` перерисовывает список; в `renderLoadedPropertyList` закреплённые
  (прошедшие фильтры) выносятся в первую группу «Закреплённые», из исходных групп убираются.
  test_html.ps1 PASS; node-харнесс (DOM-заглушка, Temp/hermes-pin-test.js): 10/10 — порядок,
  pin/unpin, отсутствие дублей, фильтр скрывает закреплённое, выбор строки. Визуальная
  проверка в AC25 — за пользователем. Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/160
- [ ] #161 — Sync.cpp-8: регистр в канонической ветке Name2Rawname (`rawname = name` → `loweredName`)
- [ ] #162 — ResetProperty.cpp-4: одна undo-область на всю операцию
- [ ] #163–#171 — runtime R2–R10 (в т.ч. R8 Teamwork, R9 сборки AC22–24/26–29)
- [ ] #186 — неприоритетное UI: адаптивный горизонтальный вид палитры (вкладки внизу),
  когда ширина панели больше высоты в заданном соотношении (порог определить тестами).
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/186
- [ ] #187 — вкладка «Синхронизация»: вверху выбор свойства, описание которого
  редактируется; под окном DSL — кнопка «Автоформат» (перенос строк и отступы в правилах
  аддона). Реализация: новый мост (чтение/запись описания определения), форматтер в JS.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/187
- [x] #173 — защита `restart_archicad_for_test.ps1`: не закрывать Archicad, если открыто несколько процессов или в имени файла нет `test`
- [x] #174 — AI-readable вывод `restart_archicad_for_test.ps1`: этапы, причины отказа и финальный `AI_RESULT`
- [x] #175 — **выполнено, ручная runtime-проверка подтверждена пользователем 2026-09-17: «всё работает»**. Кнопка свёртывания окна палитры до колонки вкладок: HTML прячет рабочую область, C++-мост `SetPaletteCollapsed` ужимает окно; пристыкованную палитру снимаем с дока → меняем ширину → возвращаем в док (иначе док не даёт менять ширину). Добавлены поправки для правого дока и ручного resize перед сворачиванием: `SetClientWidth` якорится к `TopRight` на правой половине экрана, минимум dock-слота ослабляется до `UnDock`, ширина повторно задаётся после `Dock`. После краша при сворачивании удалён опасный путь `UnDock/Dock` из `PanelResized`; ручное уменьшение свёрнутой панели принимается как новый компактный размер, ручное увеличение только логируется и исправляется следующим явным сворачиванием. Колонка вкладок HTML возвращена к ширине 2.2rem; развёрнутая палитра в шаблоне RINT-ресурса уменьшена с 450 до 383. 2026-09-17 17:47 runner распознал `test_25`, выполнил сборку AC25 и перезапуск; прежний отказ не воспроизведён. UI-проверки остаются ручными.
- [/] #176 — AC25 каркас ведомостей в Navigator по `Docs/Tables_Navigator_AC25.md`. Готово и проверено runtime в `test_25`: корневой раздел (`EnsureNavigatorRoot`, локализованное имя через ресурс `SomeStuffSchedulesNameID`, пустой `displayId` у root — иначе имя дублировалось), создание узла каталога (`NewItem`, `SS001…`, открытие диалога ID/Name) и удаление (`DeleteItem`). Визуальная проверка диалога — за пользователем. Рендеринг таблицы и окно вынесены в #177.
- [/] #177 — общий класс отрисовки таблиц `Sources/AddOn/table/TableRenderer.hpp/.cpp` (проверенная редакция ТЗ — `Docs/TableRenderer_AC25.md`). AC25-формы подтверждены SDK25: рисование = `APIDb_StartDrawingDataID` → `ACAPI_Element_Create` (2D-элементы) → `APIDb_StopDrawingDataID` (в ТЗ были AC29-имена `ACAPI_Drawing_*`); измерение текста — `APIAny_GetTextLineLengthID` в мм (в проекте уже есть `GetTextWidth`/`GetFontIndex` из `CommonFunction`, они переиспользованы). Реализованы layout (ширины/высоты/перенос/объединения/заливки) и отрисовка. Self-test под `TESTING` проходит в AC25 (Draw err 0, layout точен). Обвязка: `OpenView` открывает окно MyDraw и создаёт содержимое в том же вызове, `CreateIDFStore` → IDF, `GetElemsForDrawingCheck` больше не рисует. Код перенесён в `Sources/AddOn/table/` вместе с `TablesNavigator`. Открыто: кириллица в окне выводилась чужими глифами (исправлены порядок «layout до сброса БД окна» и `charCode` из гарнитуры — результат ждёт визуальной проверки) и габарит drawing data раздувается текстовым слоем (сетка/заливки совпадают с расчётом; причина not verified). Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/177
- [/] #178 — сузить интерфейс и панель на 40% (383 → 230 px). Окно палитры в `Tools/AddOn.grc.in`
  (GDLG 32580 + контрол Browser) 383 → 230; HTML `min-width` на body 250 → 150 px; ТЗ §2 и порог
  проверки 12 в `Tools/verify.js` приведены в соответствие (250 → 150 px). Побочный эффект ширины
  выявлен замером: при 230 px строка «Выделено элементов … ≤ <лимит> ⟳ » не помещалась в рабочую
  область 195 px (clientWidth 195 / scrollWidth 219) → добавлен `flex-wrap:wrap` + `row-gap`. На
  383 и 230 px замер даёт 0 обрезанных элементов и docScrollWidth == viewport. AC25 Windows Debug
  собран, `test_html.ps1` PASS. Визуальная проверка палитры в AC25 — за пользователем.
  Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/178
- [x] #182–#185 — исправления по ручной проверке палитры 2026-09-18; **подтверждено пользователем 2026-09-18: «теперь всё работает»** (фильтр, синяя маркировка, дропдаун классификации, поведение строк при наведении). Временная диагностика из моста убрана, финальная сборка 22:19, автотесты 487 `: ok` / 0 ошибок. #182: дропдаун классификации
  обрезался скролл-фреймом блока (замер DOM: видно 29 px из 74) — на время открытия панель снимает
  `max-height`/`overflow` блока (`setScrollFrameExpanded`; предок ищется в момент клика — виджет
  собирается раньше вставки в блок, `closest()` при сборке возвращал null). #183 + сообщение
  пользователя «строки меняют размер при наведении»: (а) раскрытие ветви — только кликом по «+»,
  клик по строке-ветви больше не разворачивает; (б) корень симптома — `h()` при hover перезаписывал
  весь `cssText` значением, сохранённым до добавления отступов, и строки «прыгали»; теперь hover
  меняет только свойства из `hoverStyle`. #184/#185: мост отдаёт `hasRule=false` для правил
  материалов — `SyncString` вызывался из `ParsePropertyDescription`/`ParsePropertyDescriptionToRules`
  с `elementType = API_ObjectID` и отбраковывал `Sync_from{Material:Layers; ...}` на проверке
  применимости; добавлен параметр `checkElementType` (по умолчанию true — поведение синхронизации
  не изменено), в разборе описаний передаётся false. Диагностика `TestPropertyRuleFlagOnProjectElements`
  (RuleFlagProj в test_results.txt): описания из обоих источников совпадают, после фикса rule=1
  у 5+4 свойств; «Флаг» (Sync_flag) корректно остался rule=0 (флаг — не правило). RED (1 провал)
  → GREEN (487 `: ok`, 0 ошибок). `test_html.ps1` PASS. Ручная проверка в AC25 — за пользователем.
  Issues: #182, #183, #184, #185.

Методика 1.7.x: тесты до (GREEN-регрессии) → реализация → тесты после → замер `clock()` →
оптимизация при деградации >10%.

## Грабли

- **Моки `ACBridge` в HTML** (`getFilterPresets` HTML:276, `resetPropertyToDefault` HTML:421) —
  «рабочий» UI до реализации моста; потребители: кнопки фильтра HTML:953/979, сброс HTML:1144.
- **Дубли строк классификации**: C++ кладёт словарь системы и под `systemname`, и под
  `systemname_full`, GetFullName даёт одинаковые полные имена → каждая запись ×2 (JS-дедуп
  `uniqueClassificationOptions`).
- **Ранний `return` по `data.common`** прятал выбор классификации и «Применить ко всем» при
  единой классификации/одном элементе.
- **prefs проекта ломают Teamwork** — `ACAPI_SetPreferences` пишется во все файлы проекта
  (DevKit-25: Preferences_Save). Настройки — только в локальный
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; `ReadSyncSettingsFromFile` отвергает файл с другой
  `PreferencesVersion` → любой новый массив в настройках требует bump версии.
- **Undo-области на каждый элемент** (ResetProperty.cpp) — сотни undo-шагов; док DevKit
  требует одну область на пользовательское действие.
- **Ключи кэша параметров всегда в нижнем регистре** (prefix + ToLowerCase + BRACEEND) —
  возврат имени без нормализации регистра = правило молча не срабатывает.
- **Мост = инлайн-функции `RegisterACAPIJavaScriptObject`**, JSON-команды сняты (b7a996b);
  аргументы `JSFunction` парсить через `DynamicCast<JSValue>` — `DynamicCast<JSArray>` ронял
  ArchiCAD (латентный краш R1–R2 из ревью палитры).
- **Подсветка**: `APIIo_HighlightElementsID` + `APIDo_ZoomToElementsID` триггерят
  SelectionChangeHandler → палитра сбрасывала выделение; фикс — `static suppressSelectionRefresh`.
  `SetElementHighlight` только AC26+/27+; AC25 — прямой `ACAPI_Interface`, сброс = вызов без par1,
  Clear перед Set.
- **Данные «Монитора» — только из `PROPERTYCACHE()`**, без `ACAPI_Property_GetPropertyValue`
  на каждый элемент. В кэше `property` — ТОЛЬКО определения; значения — по элементам отдельно.
- **`gh` CLI не в PATH** — `export PATH="$PATH:/c/Program Files/GitHub CLI"`; Reviews/ в
  .gitignore — трекер локальный (BOM+LF, править через python `utf-8-sig`).
- **Снято автором (не фиксить)**: `Dimensions.cpp:158` `pen_original`; пересоздание элементов
  отделки в Roombook; `Sync.cpp:419-428` накопительный `epm`.

## Archive

### 2026-09-17 — #158: хранение результата разбора
- Реализовано поле parsed и заполнение результатом ParsePropertyDescriptionToRules; изменение описания заменяет запись. ReadPropertyDefinition очищает записи правил; отдельная runtime-проверка этого пути остаётся открыта.
- Baseline: 12 RuleFlag проходят. Первый расширенный эксперимент завершился exit 50 до test_results.txt; причина not verified, Windows Application не содержит события сбоя. Выполнен точечный откат; baseline и отдельное добавление поля снова прошли.
- Упрощённые проверки без локального PropertyCache/ReadPropertyDefinition: RED (Sync/Spec) → заполнение кэша. Исправлено ошибочное ожидание теста fullCommand == исходное описание: парсер формирует команду заново (Sync.cpp:2843). Финал: 16/16 RuleFlag/RuleCache, 483 строки : ok, 0 ошибок проверок.
- Журналы: C:/Users/da-rogojin/AppData/Local/Temp/hermes-cache-{baseline,red,rollback,field,content-red,green,final}.log. Сбой exit 50 не объявлен исправленным.
- Коммит не создавался. Issue #158 остаётся открытым.

### 2026-09-17 — #158: признак Spec_rule
- Исправлена ошибка реализации: GetPropertyRuleFlag игнорировал otherCommands. Теперь учитывает только валидные Spec_rule, не Renum/Sum; Sync.cpp не изменён.
- RED/GREEN исполнены в AC25: 4 ожидаемых провала → 0; все 12 RuleFlag проходят. Сборка и clangd успешны; полный #158 остаётся открытым.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158#issuecomment-5719904847
- Checkpoint не создавался: пользователь не запрашивал коммит.

### 2026-09-17 — локальная фильтрация «Монитора», подшаг #158 / ТЗ §5.3
- Загрузка отделена от renderLoadedPropertyList: фильтры имени/значения, пресеты и группы не вызывают GetPropertiesList повторно. Полный список не мутируется, новые запросы и нулевое выделение инвалидируют запоздавшие ответы.
- Node с реальными обработчиками HTML и подменённым native-мостом: исходный HEAD воспроизводит 2 вызова вместо 1; после изменения проходят имя/значение/regex/снятие фильтра/группы/пресеты/обновление/порядок ответов/нулевое выделение.
- test_html.ps1 PASS (20:58); ESLint извлечённого inline JS: 0 ошибок, 6 предупреждений. npm validate:js не работает на HTML (Unexpected token <); конфигурация не изменена.
- Scope: только Interface_ru.html и состояние IDEA.md. Новая C++ сборка не запускалась; CEF runtime — not verified. Кэш Sync/Spec и фильтр наличия правила остаются в активной задаче #158.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/158#issuecomment-5718930348
- Checkpoint: 69bad72 — только Interface_ru.html. Коммит создан агентом без явного запроса пользователя; историю не переписывали.

### 2026-09-17 — проверка документа о ведомостях под AC25
- Status: COMPLETED (документационная проверка, не реализация).
- Scope: `Docs/Tables_Navigator_AC25.md` и план обсуждения `.hermes/plans/2026-09-17_181500-tables-navigator-discussion.md`; исходное вложение и production-код не изменены.
- Результат: контракты Navigator/MyDraw/IDF, хранения, фильтров и событий уточнены по LightRAG и DevKit-25; все 52 явно указанных SDK-файла существуют. Сборка и runtime не выполнялись; непроверенные сценарии перечислены в §10 документа.
- Next Step: согласовать первую ведомость, смысл строки и хранение; реализация — отдельная задача. Активная задача «Монитор» не заменялась.
- Last Checkpoint: не создавался; коммит пользователь не запрашивал.

### 2026-09-17 — диагностика runner #173 (AC25 / Windows)
- [x] Добавлен и исполнен PROJECT_CHECK; исходная проверка принимает test_25.
- [x] Штатный перезапуск и сборка AC25 завершились; git diff --check прошёл.
- Прежний отказ не воспроизведён, исправление причины не заявляется. Scope: runner и IDEA.md; UI/C++ не редактировались в этой задаче.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/173#issuecomment-5716365617

## Status: IN_PROGRESS — #160 (pin свойств) реализован и проверен харнессом; визуальная проверка в AC25 — за пользователем. #178/#182–#185 подтверждены.
## Last Completed: 2026-09-18 22:40 — #160: pin свойств «Монитора» (STATE.pinnedProperties, кнопка-булавка, группа «Закреплённые»); test_html.ps1 PASS, node-харнесс 10/10.
## Next Step: по приоритетам 2026-09-18 — следующая сессия: ручная проверка готовых UI-хвостов в AC25 (#178, #179, #180, #160, #159, #155) и закрытие после подтверждения. #158 остаётся незавершённой до момента, когда повторный замер станет критичен. #186 — неприоритет (подтверждено). #187 — реализация отдельной сессией.
## Last Checkpoint: 9496a6c `[#182-#185] Правки палитры по ручной проверке: дропдаун классификации, hover-строки, hasRule правил материалов` (8 файлов; подтверждено пользователем, issues #182–#185 закрыты). Для #160 чекпоинт не создавался.
## Scope: #160 — только Sources/AddOnResources/RFIX/HTML/Interface_ru.html и IDEA.md. C++ не менялся.
