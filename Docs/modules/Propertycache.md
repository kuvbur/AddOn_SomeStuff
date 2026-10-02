# Propertycache — Кэш свойств и справочных данных

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep).

## Назначение
Кэш свойств, атрибутов, классификаций, геолокации, форматирования и данных MEP для чтения без повторных запросов. [из комментария, Propertycache.hpp:18-19]

## Файлы
- `Propertycache.cpp/hpp`

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `PropertyCache` | Основной класс-кэш (property/info/attrib/glob/file/filedata/systemdict/dimrules/compositeCache и др.) [по коду, Propertycache.hpp:115-153] |
| `CachedLayer` | Данные слоя: buildingMaterial, fillThick, flagBits [по коду, Propertycache.hpp:101-105] |
| `PropertyRuleFlag` | Кэш правил SomeStuff в описаниях свойств (#158): description + parsed + hasRule [из комментария, Propertycache.hpp:107-113] |
| `FormulaCacheStats` | Счётчики кэшей формул `EvalExpression` (#217): calls/fullHits/exprHits/fullClears/exprClears. Только под `TESTING` [из комментария, Propertycache.hpp:115-129] |

## Методы PropertyCache
- `Update()` — полное обновление всех кэшей (glob, geo, survey, placeSets, locOrigin, classification, groups, definitions, attribute, info, files, MEP) [по коду, Propertycache.hpp:326-355]
- `Read*()` — перечитывание отдельных кэшей [по коду]
- `AddFile(fileName)` — чтение внешнего файла (FIX 2026-09-12: GetPtr вместо ContainsKey+Get) [из комментария, Propertycache.hpp:426-448]

## Публичные функции

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `GetPropertyRuleFlag` | 11 | Кэш правил SomeStuff (#158): парсит описание при промахе кэша [из комментария] — карточка; **вызывается только из TestFunc** (см. DISCREPANCIES #9) |
| `ReportFormulaCacheStats` | 47 | Печать накопленных счётчиков кэшей формул `EvalExpression` (#217) через `DBprnt`. Только под `TESTING` [из комментария, Propertycache.cpp:47-66] |
| `GetCache` | 34 | Единственный экземпляр кэша [по коду] |
| `isEng` | 878 | Язык ArchiCAD: для INT возвращает 1000 [из комментария] |
| `DimReadPref` | 1005 | Чтение правил размеров из информации о проекте (`Addon_Dimenstions`) [из комментария, Propertycache.hpp:23-29] |
| `DimParsePref` | 1046 | Разбор текста правила размеров [из комментария] |
| `GetPropertyNameByGUID` / `GetPropertyFullName` | — | Имя свойства по GUID / полное имя с группой [из комментария; строки не проверены] |

## Карточки

### `GetPropertyRuleFlag(const API_PropertyDefinition &definition) -> bool`
- Расположение: `Sources/AddOn/Propertycache.cpp:11`
- Назначение: кэшированный признак наличия правила SomeStuff в описании свойства (#158). [из комментария]
- Контракт: парсит описание только при промахе кэша или изменении описания (копия description — инвалидация). [из комментария, Propertycache.hpp:107-113, 729-732]
- Побочные эффекты: мутирует кэш `propertyRuleFlags`; парсит через `ParsePropertyDescriptionToRules` (CommandHelpers.cpp:200). [из callgraph.json]
- Вызывается из: **только тесты** — `TestGetPropertyRuleFlag` (TestFunc.cpp:2917, 16 вызовов), `TestPropertyRuleFlagOnProjectElements` (:3006, 2). [из callgraph.json]

### `PropertyCache::Update()`
- Расположение: `Sources/AddOn/Propertycache.hpp:326` (в теле класса)
- Назначение: полное обновление всех кэшей с таймированием (clock, лог длительности). [по коду]
- Контракт: перечитывание определений очищает `propertyRuleFlags` (правила удалённых свойств). [из комментария, Propertycache.hpp:559]
- Побочные эффекты: **мутирует весь кэш**; вызовы множества ACAPI_* (GetPreferences, классификации, группы свойств и др.). [по коду]

### `ReportFormulaCacheStats(const GS::UniString &reason)` (#217, только `TESTING`)
- Расположение: `Sources/AddOn/Propertycache.cpp:47`
- Назначение: печатает в `DBprnt` накопленные счётчики кэшей формул `EvalExpression` перед каждым `Clear()` по переполнению. [из комментария, Propertycache.cpp:47-49]
- Контракт: молчит при `calls == 0`; доли попаданий считаются от вызовов, дошедших до кэшей (вызовы, отсечённые пустой строкой или отсутствием разделителей, не учитываются). [по коду]
- Побочные эффекты: только вывод в `DBprnt`; счётчики **не** обнуляет. [по коду]
- Вызывается из: `EvalExpression` (`CommonFunction.cpp:1314` перед `exprResultFullCache.Clear`, `:1383` перед `exprResultCache.Clear`). [по коду]
- Инвариант: счётчики в `formulaCacheStats` намеренно не сбрасываются ни в конструкторе `PropertyCache`, ни в `Update()` — нужна картина за всю сессию. [из комментария, Propertycache.hpp:224-230]

## Зависимости
- `Helpers.hpp`, `dialogs/CommandHelpers.hpp`, `CommonFunction.hpp`, `ClassificationFunction.hpp` [по include]

## Зависимости (используется в)
Sync, Spec, Summ, ResetProperty, Helpers и др. через `PROPERTYCACHE()` [по коду]

## Инварианты
- `GetAllAttributeToParamDict` (`Propertycache.cpp:718`) кэширует имена и индексы Layer, CompWall и BuildingMaterial под `{@attrib:<тип>_name_<нижний регистр>}` и `{@attrib:<тип>_inx_<индекс>}`. Удалённые атрибуты пропускаются. Общий `ReadAttribute`/`Clear` обновляет и сбрасывает все три списка; кэш компонентов `compositeCache` — отдельный. [по коду, #235]
- Ключи кэша всегда в нижнем регистре (`ToLowerCase` + `BRACEEND`) — AGENTS.md §6 [из AGENTS.md]
- Значения свойств — только из PROPERTYCACHE(), не ACAPI_Property_GetPropertyValue [из AGENTS.md]
