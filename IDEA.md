# Current Task

## Task

UI вкладки «Монитор» — остаток работ:

1. **1.5 `GetFilterPresets`** — реальный список преднастроенных фильтров отображения свойств
   (взамен мока `ACBridge.getFilterPresets`, HTML:276)
2. **1.6 `ResetPropertyToDefault`** — сброс свойства к значению по умолчанию для выделенных
   (взамен мока `ACBridge.resetPropertyToDefault(propertyId)`, HTML:421)
3. **1.7.1–1.7.3** — кэш распарсенных правил SomeStuff (Sync/Spec) + фильтр списка свойств по наличию
   правила; цветовая маркировка неактивных флагов (`flagfindspec=false`); Pin свойств

**ТЗ по интерфейсу** — `Sources/AddOnResources/RFIX/HTML/ТЗ интерфейс.md` (обязательное). Правки
`Interface_ru.html` / `index.html` — только по ТЗ; после каждой правки HTML валидация
`powershell -File Tools/test_html.ps1`.

Целевая версия — **AC25** (в репозитории только DevKit-25).

## Status

IN_PROGRESS — код Этапа 1.7 не начат. Открыт остаток ревью 2026-09-12 и runtime-проверки R2–R10.
Новый пользовательский реквест 2026-09-13 (UI классификации) — выполнен, считается отдельной задачей.

## Last Completed

- **2026-09-13 — UI классификации (вкладка «Монитор»)**, `Interface_ru.html`:
  (1) выбор классификации + кнопка «Применить ко всем» теперь видны всегда (в т.ч. при
  единой классификации и одном выделенном элементе — раньше ранний `return` по `data.common` прятал их);
  (2) устранены дубли строк в списке — JS-дедуп `uniqueClassificationOptions` (причина: C++
  кладёт один и тот же словарь системы в `systemdict` и под `systemname`, и под
  `systemname_full`, GetFullName даёт одинаковые полные имена → каждая запись ×2);
  (3) выпадающий список переделан в дерево как в Archicad — разворачиваемые узлы (▸/▾),
  живой поиск по названию с пересчётом вариантов по мере набора и подсветкой совпавших цепочек.
  Валидация `Tools/test_html.ps1`: HTMLHint PASSED + verify.js PASSED. Логика (дедуп/дерево/
  фильтр) проверена ad-hoc запуском в node. Runtime НЕ тестировался (нужен живой ArchiCAD).
- Ревью 2026-09-12 закрыто: `11add7a` + `13ba469` (60 находок), `2a48348` (ревью 2026-09-12b, 87 пунктов),
  `072f975` (вторая проверка), `1625cf1` (Sync.cpp-7: Name2Rawname — регистронезависимость + каноническая форма)
- `f0909f0` — настройки аддона вынесены из prefs проекта в локальный файл
  `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`; автоматические записи (Show/Hide палитры, observer, Initialize) убраны
- Runtime AC25 (2026-09-12, `restart_archicad_for_test.ps1`, exit 0): 741 ok, 0 `=== ERROR IN TEST ===`
  в диапазоне `TEST : start`..`TEST : end` (включая Name2Rawname и DBtest SetToMax `numpos == 10`);
  AC25 Build succeeded
- Подсветка + зум по клику ×N, пользовательское выделение сохраняется — **Verified 2026-08-24**
  (подтверждено Дмитрием, runtime AC25)

## Next Step

Пункты 1.5/1.6 — замена моков `ACBridge.getFilterPresets` / `resetPropertyToDefault` на инлайн-функции
моста (быстрый вход). Далее 1.7.1: baseline времени `GetPropertiesList` на выделении из 10 элементов
(`clock()` вокруг мост-функций + `DBprnt` → `test_results.txt`) → кэш правил Sync/Spec + фильтр списка
свойств по наличию правила.

## Plan

- [ ] 1.5 `GetFilterPresets` — преднастроенные фильтры отображения свойств: вход нет →
      `{presets: [{label, query}]}` (`query` — подстрока или regex `/шаблон/`).
      Хранилище — существующий `SyncSettings.dat` (поле-массив пресетов в `SyncSettings`,
      `SyncSettings.hpp`); набор по умолчанию — из ТЗ §5.3 (Все свойства / Только IFC / Только геометрия /
      Незаполненные, совпадает с моком HTML:276-285). Учесть: `ReadSyncSettingsFromFile`
      (`SyncSettings.cpp:188`) отвергает файл с другой версией → нужен bump `PreferencesVersion = 5`
      (`SyncSettings.cpp:17`) и дефолтный набор для старых файлов.
      Заменяет мок `ACBridge.getFilterPresets` (HTML:276), потребители — кнопки фильтра по имени (HTML:953)
      и по значению (HTML:979), ТЗ §5.3
- [ ] 1.6 `ResetPropertyToDefault` — сброс свойства к значению по умолчанию для всех выделенных:
      `{propertyId}` → `{success}`; заменяет мок `ACBridge.resetPropertyToDefault` (HTML:421), кнопка сброса
      в строке свойства (HTML:1144), ТЗ §5.11 (в т.ч. «Сброс свойства реально изменяет Archicad») —
      запись внутри `ACAPI_CallUndoableCommand`
- [ ] 1.7.1 Кэш правил SomeStuff + фильтр (правила читаются из описаний свойств через кэш; выход — мост-данные для UI + ускорение повторных рендеров)
- [ ] 1.7.2 Цветовая маркировка неактивных флагов — HTML+CSS по данным моста, без новых вызовов API
- [ ] 1.7.3 Pin свойств — `STATE.pinnedProperties` (JS), порядок сортировки в `renderPropertyList`
- [ ] Остаток ревью: `Sync.cpp-8` (каноническая ветка возвращает текст без нормализации регистра —
      `Sync.cpp:1008` всё ещё `rawname = name;`, нужно `loweredName`), `ResetProperty.cpp-4`
      (одна undo-область на всю операцию вместо области на каждый элемент/инструмент —
      `ResetProperty.cpp:277, 411, 415`; правка уровня функции)
- [ ] Runtime-проверки R2–R10 по `Reviews/open-2026-09-12.tracker.csv` — в т.ч. R8 (Teamwork Send/Receive
      несколько циклов с включённым мониторингом), R9 (сборки AC22–24/26–29)

Методика 1.7.x: тесты до (GREEN-регрессии текущего поведения) → реализация → тесты после → замер
`clock()` → оптимизация при деградации >10%.

## Decisions

- Пресеты фильтров (пункт 1.5) хранятся в существующем `SyncSettings.dat` — решение Дмитрия 2026-09-12
- JSON-команды — снято: удалены коммитом `b7a996b`; вся UI-функциональность делается инлайн-функциями
  JS-моста `BrowserPalette` (спецификации в формате `*Command` не актуальны)
- 2026-09-12: настройки — локальный файл `…/GRAPHISOFT/SomeStuff/SyncSettings.dat`, в prefs проекта не пишем
- 2026-09-12: проверка платформенной нейтральности `SyncSettings.dat` (интернет + DevKit-25): путь
  через `APIEnv_GetSpecFolderID` кроссплатформенный (mac `~/Library/Preferences/Graphisoft`, win
  `%APPDATA%\Graphisoft`); формат файла формально не межплатформенный (GS::OChannel = нативный
  порядок байт, для переноса есть `IO::SetPlatformOProtocol`), но файл локальный и обе целевые
  платформы little-endian. Legacy-миграция читает `ACAPI_GetPreferences` без platformSign — оставлено
  как есть (вне текущего scope)
- 2026-09-12: цель сборки и тестов — AC25
- 2026-09-12: снято автором (не фиксить): `Dimensions.cpp:158` `pen_original`; пересоздание элементов отделки
  в Roombook; `Sync.cpp:419-428` накопительный `epm` в SyncElement
- 2026-08-24: мост — push-pattern (`browser.ExecuteJS()`), аргументы `JSFunction` парсить через
  `DynamicCast<JSValue>` (детали — скилл `archicad-cef-pitfalls`)

## Scope

Этап 1.7: `Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp`,
`Sources/AddOnResources/RFIX/HTML/Interface_ru.html`, `Sources/AddOn/TestFunc.cpp/.hpp`, `IDEA.md`.
Остаток ревью: `Sources/AddOn/Sync.cpp`, `Sources/AddOn/pk/ResetProperty.cpp`,
`Reviews/open-2026-09-12.tracker.csv`. Всё остальное — отдельная задача.

---

## Справочное для Этапа 1.7

### Кэш и хелперы для повторного использования

| Компонент | Файл | Назначение |
|-----------|------|------------|
| `PropertyCache::property` | Propertycache.hpp:107 | Словарь всех свойств (ParamDictValue) — самый быстрый доступ |
| `PropertyCache::propertygroups` | Propertycache.hpp:115 | Группы свойств (Guid → API_PropertyGroup) |
| `PropertyCache::systemdict` | Propertycache.hpp:113 | Системы классификации |
| `GetSelectedElements2()` | CommonFunction.hpp:292 | GUID выделенных элементов (оптимизированная) |
| `ParamHelpers::GetParamValueFromCache()` | Propertycache.hpp:46 | Чтение значения из кэша по rawname |
| `ParamHelpers::GetAllPropertyDefinitionToParamDict()` | Propertycache.hpp:92 | Все определения свойств в ParamDictValue |
| `ParamHelpers::GetGroupFromCache()` | Propertycache.hpp:55 | Имя группы свойств по Guid |
| `ParamHelpers::isPropertyDefinitionRead()` | Propertycache.hpp:51 | Проверка готовности кэша |
| `ClassificationFunc::GetAllClassification()` | ClassificationFunction.hpp:26 | Загрузка всех классификаций |
| `ClassificationFunc::ReadSystemDict()` | ClassificationFunction.hpp:64 | Словарь систем/классов |
| `RegisterACAPIJavaScriptObject()` | dialogs/BrowserPalette.cpp:100 | JS-мост в BrowserPalette |

Принцип: данные вкладки «Монитор» берутся из уже загруженного `PROPERTYCACHE()` — без дополнительных
`ACAPI_Property_GetPropertyValue` на каждый элемент.

---

## Last Checkpoint: ad7e8f5 [UI классификации] dropdown-дерево как в Archicad + дедуп + всегда видимое «Применить ко всем»
## Next Step: runtime-проверка классификации (древовидный выбор + поиск) в живом ArchiCAD; далее 1.5/1.6 (замена моков getFilterPresets/resetPropertyToDefault), 1.7.1
## Scope: Sources/AddOn/dialogs/BrowserPalette.cpp/.hpp, Sources/AddOnResources/RFIX/HTML/Interface_ru.html, Sources/AddOn/TestFunc.cpp/.hpp, IDEA.md
## Verified 2026-08-24: подсветка+зум по клику ×N работает, выделение сохраняется (подтверждено Дмитрием, runtime AC25)
