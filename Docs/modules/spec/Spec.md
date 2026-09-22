# spec/Spec — Движок спецификаций

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep; функции внутри namespace Spec).

## Назначение
Генерация спецификаций по правилам: разбор описаний, выбор элементов, группировка, создание/обновление элементов. [из комментария, Spec.hpp:8-9]

## Файлы
- `spec/Spec.cpp/hpp`

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `GroupSpec` | unic_paramrawname, out_paramrawname, sum_paramrawname, flag_paramrawname, fromMaterial, fromLibData, n_layer [из комментария, Spec.hpp:13-23] |
| `SpecRule` | rule_name, groups, out_paramrawname, subguid_paramrawname/rulename/rulevalue, elements, exsist_elements, rule_definitions, favorite_name, flags (isKM, isKZH, delete_old, stop_on_error, only_visible) [из комментария, Spec.hpp:27-47] |
| `Element` / `ElementDict` | Временный контейнер создаваемого элемента / словарь по сцепке уникальных параметров [из комментария] |
| `SpecRuleDict` | HashTable<string, SpecRule> [из комментария] |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `SpecAll` | 140 | Создание спецификации из выбора/видимых/правил по умолчанию [из комментария] — карточка |
| `SpecFilter` | ~346 | Исключение неподходящих типов/БД [из комментария; строка не проверена] |
| `GetRuleFromDefaultElem` | 36 | Правила из свойств элемента по умолчанию [из комментария] |
| `GetRuleFromElement` | — | Правила из выбранного элемента [из комментария; строка не проверена] |
| `AddRule` | 1114 | Разбор описания и добавление правила в словарь [из комментария] |
| `GetRuleFromDescription` | 1833 | Разбор строки описания в SpecRule [из комментария] |
| `GetParamValue` | — | Чтение одного значения параметра [из комментария; строка не проверена] |
| `GetElementsForRule` | 1463 | Формирование элементов для одного правила [из комментария] |
| `GetParamToReadFromRule` | — | Параметры для предварительного чтения [из комментария; строка не проверена] |
| `GetElementForPlace` | 2297 | Создание/настройка элемента для размещения [из комментария] |
| `GetSizePlaceElement` | — | Размер элемента по сетке [из комментария; строка не проверена] |
| `PlaceElements` | 2427 | Размещение сформированных элементов и заполнение параметров [из комментария] — карточка |

## Карточки

### `Spec::SpecAll(const SyncSettings &syncSettings) -> GSErrCode`
- Расположение: `Sources/AddOn/spec/Spec.cpp:140`
- Назначение: создаёт спецификацию из текущего выбора, всех видимых элементов или правил по умолчанию. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **создаёт/обновляет/удаляет элементы спецификации** (PlaceElements-цепочка); читает выделение и свойства. [по коду]
- Вызывает: `GetRuleFromDefaultElem` (:165), `SpecFilter` (:157/176), `SpecArray` (:185), `GetSelectedElements` (Helpers.cpp:695). [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:441). [из callgraph.json]

### `Spec::PlaceElements(GS::Array<ElementDict> &elementstocreate, ParamDictValue &paramToWrite, ParamDictElement &paramOut, Point2D &startpos) -> GSErrCode`
- Расположение: `Sources/AddOn/spec/Spec.cpp:2427`
- Назначение: размещает сформированные элементы в модели и заполняет их параметры. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **создание элементов в проекте** (из избранного `favorite_name`), запись параметров/GUID (`subguid`); изменение сетки размещения (startpos). [по коду]
- Вызывает: `GetElementForPlace` (:2461), `UnhideUnlockElementLayer` (CommonFunction.cpp:2472), `StringUnic` (CommonFunction.cpp:1473); создание элементов — `ACAPI_Element_Create` в `ACAPI_CallUndoableCommand` (:2449). [из callgraph.json]
- Вызывается из: `SpecArray` (Spec.cpp:950). [из callgraph.json]

## Зависимости
- `Helpers.hpp`, `Propertycache.hpp`, `CommonFunction.hpp` [по include]

## Зависимости (используется в)
- `SomeStuff_Main.cpp` [по коду]

## Инварианты
- `stop_on_error = true` — остановка обработки правила при ошибке; `only_visible = true` — только видимые [из комментария, Spec.hpp:43-44]
- `isKM`/`isKZH` — правила для КМ/КЖ [из комментария, Spec.hpp:45-46]
