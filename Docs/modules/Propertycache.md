# Propertycache — Кэш свойств и справочных данных

## Назначение
Кэш свойств, атрибутов, классификаций, геолокации, форматирования и данных MEP для чтения без повторных запросов к API. Главный центральный хранилище данных add-on.

## Файлы
- `Propertycache.cpp/hpp` — кэш

## Ключевые типы

| Тип | Назначение |
|-----|------------|
| `PropertyCache` | Основной класс-кэш со всеми полями |
| `CachedLayer` | Данные одного слоя (buildingMaterial, fillThick, flagBits) |
| `PropertyRuleFlag` | Кэш правил SomeStuff в описаниях свойств (#158) |
| `MEPDict` / `MEPDicts` | Словари MEP (AC2900) |

## Поля PropertyCache

### Кэши данных
- `property` — свойства, `info` — информация о проекте, `attrib` — атрибуты, `glob` — глобальные переменные
- `file` — прочитанные файлы, `filedata` — данные из файлов
- `systemdict` — системы классификации, `reversesystemdict` — обратное отображение
- `propertygroups` — группы свойств, `dimrules` — правила размеров
- `compositeCache` — кэш состава конструкций

### Состояние (bool флаги)
Каждый кэш имеет пару флагов: `isX_OK` (успешно) и `isXRead` (запрошен). Примеры: `isGetGeoLocation_OK/Read`, `isClassification_OK/Read`, `isMEP_OK/Read`, `isPropertyDefinition_OK/Read`.

### AC2900
- `mepdict` — MEP данные, `isMEP_OK/Read`

## Методы PropertyCache

### Обновление
- `Update()` — полное обновление всех кэшей (глоб, geo, классификация, свойства, атрибуты, файлы, MEP)
- Читает данные из Propertycache.cpp Update() → читает из API, кэширует

### Чтение (отдельные кэши)
- `ReadClassification()` — системы классификации
- `ReadPropertyDefinition()` — свойства
- `ReadAttribute()` — атрибуты
- `ReadInfo()` — информация о проекте
- `ReadGetGeoLocation()` / `ReadSurveyPointTransformation()` / `ReadPlaceSets()` / `ReadLocOrigin()` — геоданные
- `ReadFileFromDefinition()` — файлы из описаний свойств
- `ReadGroupProperty()` — группы свойств
- `ReadFormatStringForMeasureType()` — форматирование
- `ReadMEP()` (AC2900) — MEP данные

## Публичные функции (вне класса)

| Функция | Назначение |
|---------|------------|
| `GetCache()` | Получение единственного экземпляра |
| `GetPropertyRuleFlag` | Кэш правил SomeStuff (#158) |
| `GetPropertyNameByGUID` | Имя свойства по GUID |
| `GetPropertyFullName` | Полное имя свойства (с группой) |
| `isEng` | Проверка языка ArchiCAD |
| `PROPERTYCACHE()` | Глобальный accessor (function pointer) |

## Зависимости
- `Helpers.hpp` — ParamHelpers
- `dialogs/CommandHelpers.hpp` — SyncRuleInfo, ParsePropertyResult
- `CommonFunction.hpp` — DBprnt, msg_rep
- `ClassificationFunction.hpp` — SystemDict
- `ACAPI/MEP*` (AC2900)

## Зависимости (используется в)
Почти все модули обращаются через `PROPERTYCACHE()`:
- `Sync` — при чтении свойств
- `Spec` — правила из свойств
- `Summ` — суммирование
- `ResetProperty` — для определений свойств
- `Helpers` — GetAllAttributeToParamDict и др.

## Инварианты
- Ключи кэша всегда в нижнем регистре (`ToLowerCase` + `BRACEEND`)
- Значения свойств берутся только из `PROPERTYCACHE()`, НИКОГДА из `ACAPI_Property_GetPropertyValue`
- Кэш перечитывается при смене проекта (Update())
- `selectionPropertiesCache` — mutable, для GetPropertiesListCommand
- `propertyRuleFlags` — кэш правил (#158), инвалидируется при перечитывании определений
- FIX (ревью 2026-09-12): `AddFile` и `ReadClassification` содержат оптимизации ContainsKey+Get → GetPtr
