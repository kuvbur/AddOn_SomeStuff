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

## Методы PropertyCache
- `Update()` — полное обновление всех кэшей (glob, geo, survey, placeSets, locOrigin, classification, groups, definitions, attribute, info, files, MEP) [по коду, Propertycache.hpp:326-355]
- `Read*()` — перечитывание отдельных кэшей [по коду]
- `AddFile(fileName)` — чтение внешнего файла (FIX 2026-09-12: GetPtr вместо ContainsKey+Get) [из комментария, Propertycache.hpp:426-448]

## Публичные функции

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `GetPropertyRuleFlag` | 11 | Кэш правил SomeStuff (#158): парсит описание при промахе кэша [из комментария] — карточка |
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
- Побочные эффекты: мутирует кэш `propertyRuleFlags`. [по коду]

### `PropertyCache::Update()`
- Расположение: `Sources/AddOn/Propertycache.hpp:326` (в теле класса)
- Назначение: полное обновление всех кэшей с таймированием (clock, лог длительности). [по коду]
- Контракт: перечитывание определений очищает `propertyRuleFlags` (правила удалённых свойств). [из комментария, Propertycache.hpp:559]
- Побочные эффекты: **мутирует весь кэш**; вызовы множества ACAPI_* (GetPreferences, классификации, группы свойств и др.). [по коду]

## Зависимости
- `Helpers.hpp`, `dialogs/CommandHelpers.hpp`, `CommonFunction.hpp`, `ClassificationFunction.hpp` [по include]

## Зависимости (используется в)
Sync, Spec, Summ, ResetProperty, Helpers и др. через `PROPERTYCACHE()` [по коду]

## Инварианты
- Ключи кэша всегда в нижнем регистре (`ToLowerCase` + `BRACEEND`) — AGENTS.md §6 [из AGENTS.md]
- Значения свойств — только из PROPERTYCACHE(), не ACAPI_Property_GetPropertyValue [из AGENTS.md]
