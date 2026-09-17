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
- [ ] #158 — 1.7.1 кэш правил SomeStuff + фильтр списка свойств (baseline `clock()` → `DBprnt` до реализации)
- [ ] #159 — 1.7.2 цветовая маркировка неактивных флагов
- [ ] #160 — 1.7.3 pin свойств
- [ ] #161 — Sync.cpp-8: регистр в канонической ветке Name2Rawname (`rawname = name` → `loweredName`)
- [ ] #162 — ResetProperty.cpp-4: одна undo-область на всю операцию
- [ ] #163–#171 — runtime R2–R10 (в т.ч. R8 Teamwork, R9 сборки AC22–24/26–29)
- [x] #173 — защита `restart_archicad_for_test.ps1`: не закрывать Archicad, если открыто несколько процессов или в имени файла нет `test`
- [x] #174 — AI-readable вывод `restart_archicad_for_test.ps1`: этапы, причины отказа и финальный `AI_RESULT`
- [/] #175 — кнопка свёртывания окна палитры до колонки вкладок: HTML прячет рабочую область, C++-мост `SetPaletteCollapsed` ужимает окно; пристыкованную палитру снимаем с дока → меняем ширину → возвращаем в док (иначе док не даёт менять ширину). Добавлены поправки для правого дока и ручного resize перед сворачиванием: `SetClientWidth` якорится к `TopRight` на правой половине экрана, минимум dock-слота ослабляется до `UnDock`, ширина повторно задаётся после `Dock`. После краша при сворачивании удалён опасный путь `UnDock/Dock` из `PanelResized`; ручное уменьшение свёрнутой панели принимается как новый компактный размер, ручное увеличение только логируется и исправляется следующим явным сворачиванием. Колонка вкладок HTML возвращена к ширине 2.2rem; развёрнутая палитра в шаблоне RINT-ресурса уменьшена с 450 до 383. 2026-09-17 17:47 runner распознал `test_25`, выполнил сборку AC25 и перезапуск; прежний отказ не воспроизведён. UI-проверки остаются ручными.

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

### 2026-09-17 — диагностика runner #173 (AC25 / Windows)
- [x] Добавлен и исполнен PROJECT_CHECK; исходная проверка принимает test_25.
- [x] Штатный перезапуск и сборка AC25 завершились; git diff --check прошёл.
- Прежний отказ не воспроизведён, исправление причины не заявляется. Scope: runner и IDEA.md; UI/C++ не редактировались в этой задаче.
- Issue: https://github.com/kuvbur/AddOn_SomeStuff/issues/173#issuecomment-5716365617

## Status: WAITING_FOR_TEST
## Last Completed: #173 — добавлен PROJECT_CHECK с фактически проверяемой строкой и результатом regex. Штатный runner 2026-09-17 17:47: exit 0, build=True (AC25), tests=UNKNOWN, JSON tests skipped; test_25 открыт вновь (PID 43156). Предыдущий ложный отказ не воспроизведён; алгоритм защиты не изменён.
## Next Step: вручную проверить палитру «Монитор» (#175) и сброс свойства (#155) в открытом test_25. При повторном отказе runner сохранить PROJECT_CHECK и SAFETY_BLOCK; причина прежнего отказа — not verified.
## Last Checkpoint: aa4d833 `[#157] real filter presets bridge`; в диагностике runner новых коммитов нет.
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html, Tools/AddOn.grc.in, Sources/AddOnResources/RINT/AddOn.grc, Sources/AddOn/TestFunc.cpp/.hpp, Tools/restart_archicad_for_test.ps1, IDEA.md
