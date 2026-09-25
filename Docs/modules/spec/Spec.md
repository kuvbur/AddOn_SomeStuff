# spec/Spec — Движок спецификаций

> Хеш состояния: незакоммиченная правка #207 (base `07b97e0`, 2026-09-24). Номера строк — определения в `.cpp` (1-based, проверены source; функции внутри namespace Spec).

## Назначение
Генерация спецификаций по правилам: разбор описаний, выбор элементов, группировка, создание/обновление элементов. [из комментария, Spec.hpp:8-9]

## Файлы
- `spec/Spec.cpp/hpp`
- `json_commands/SpecCommand.cpp/hpp` — JSON-точка входа AC25–29 [по коду]

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

### `Spec::SpecAll(const SyncSettings &syncSettings, const GS::Array<GS::UniString> *ruleNames = nullptr, const Point2D *placementPoint = nullptr, SpecRunResult *runResult = nullptr) -> GSErrCode`
- Расположение: `Sources/AddOn/spec/Spec.cpp:140`
- Назначение: создаёт спецификацию из текущего выбора, всех видимых элементов или правил по умолчанию. [из комментария]
- Контракт: если выделение пусто и `GetRuleFromDefaultElem` обнаружил включённые элементы (`has_elementspec`), передаёт заполненные `rules` в `SpecArray` даже при пустом `guidArray`; возвращает `NoError` только когда нет ни выделения, ни включённых элементов default-правила. Для non-interactive запуска `placementPoint` задаёт начальную точку без диалога и окна прогресса; при `ruleNames == nullptr` обрабатываются все валидные правила. `runResult->elementsToCreate` после размещения равен приросту `paramOut` (число реально созданных элементов, без уже запланированных изменений); остальные счётчики отражают сформированные списки. [по коду; AC25 Debug #207 и runtime #208/#209]
- Если требуемое значение свойства строительного материала не прочитано, строка не формируется: пустое наименование не подставляется вместо исходных данных. `ParamHelpers::GetAttributeValues` для отсутствующего у исходного элемента свойства с `fromPropertyDefinition` пробует получить значение у строительного материала. [по коду; AC25 runtime #209]
- Побочные эффекты: **создаёт/обновляет/удаляет элементы спецификации** (PlaceElements-цепочка); читает выделение и свойства. [по коду]
- Вызывает: `GetRuleFromDefaultElem` (:165), `SpecFilter` (:157/176), `SpecArray` (:184), `GetSelectedElements` (Helpers.cpp:695). [по коду]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:441) и `SomeStuffCommand.Spec` (`json_commands/SpecCommand.cpp`; JSON API AC25–29). Menu-path сохраняет `SpecDG` и `ClickAPoint`; JSON-команда требует `placementPoint {x, y}`, необязательно принимает `ruleNames` и возвращает `status`, `resultCode`, `elementsToCreate`, `elementsToModify`, `elementsToDelete`, `elapsedSeconds`. AC25 runtime: после вызова с точкой число объектов выросло с 548 до 582, в последних 34 объектах свойство «Спецификации материалов/Наименование в объект» заполнено. [по коду и AC25 runtime #208/#209]

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
