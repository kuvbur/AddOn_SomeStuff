# Current Task

## Задача

UI вкладки «Монитор» — остаток работ. Работать по GitHub issues (см. План); целевая версия —
**AC25** (в репозитории только DevKit-25). ТЗ по интерфейсу —
`Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` (обязательное); правки HTML — только по ТЗ,
после каждой правки `powershell -File Tools/test_html.ps1`.

## План

- [ ] #157 — 1.5 `GetFilterPresets` (пресеты фильтров в `SyncSettings.dat`, bump `PreferencesVersion = 5`)
- [ ] #155 — 1.6 `ResetPropertyToDefault` (существующий issue, инлин-функция моста, `ACAPI_CallUndoableCommand`)
- [x] #154 — `Sum_flag` фильтр в `Summ.cpp` (`Sum_GetElement`, обе ветви: `SUM_TO_INFO` и обычная), константа `SUMFLAG` в `Constants.hpp`
- [ ] #158 — 1.7.1 кэш правил SomeStuff + фильтр списка свойств (baseline `clock()` → `DBprnt` до реализации)
- [ ] #159 — 1.7.2 цветовая маркировка неактивных флагов
- [ ] #160 — 1.7.3 pin свойств
- [ ] #161 — Sync.cpp-8: регистр в канонической ветке Name2Rawname (`rawname = name` → `loweredName`)
- [ ] #162 — ResetProperty.cpp-4: одна undo-область на всю операцию
- [ ] #163–#171 — runtime R2–R10 (в т.ч. R8 Teamwork, R9 сборки AC22–24/26–29)

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

## Last Checkpoint: 2a08586 backlog в issues #157-#171; правило issue-first (AGENTS.md 11.1)
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html, Sources/AddOn/TestFunc.cpp/.hpp, IDEA.md
