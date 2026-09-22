# spec/Spec — Движок спецификаций

## Назначение
Генерация спецификаций по правилам: разбор описаний, выбор элементов, группировка, создание/обновление элементов по правилам SomeStuff.

## Файлы
- `spec/Spec.cpp/hpp` — движок спецификаций (~1000+ строк)

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `GroupSpec` | Описание группы внутри правила: unic_paramrawname, out_paramrawname, sum_paramrawname, flag_paramrawname |
| `SpecRule` | Полное правило: rule_name, groups, out_paramrawname, subguid_*, elements, exsist_elements, rule_definitions, flags (isKM, isKZH, delete_old, stop_on_error, only_visible) |
| `Element` | Временный контейнер: out_param, out_sum_param, subguid_*, elements, exs_guid |
| `ElementDict` | HashTable<string, Element> — словарь по уникальным параметрам |
| `SpecRuleDict` | HashTable<string, SpecRule> — словарь правил |

## Публичный API

### Основные
| Функция | Назначение |
|---------|------------|
| `SpecAll` | Создание спецификации из выбора/видимых/правил |
| `SpecArray` | Обработка массива элементов по правилам |
| `SpecFilter` | Исключение неподходящих типов/БД |
| `GetRuleFromDefaultElem` | Правила из свойств по умолчанию |
| `GetRuleFromElement` | Правила из выбранного элемента |
| `AddRule` | Добавление правила из описания |
| `GetRuleFromDescription` | Парсинг описания в SpecRule |

### Вспомогательные
| Функция | Назначение |
|---------|------------|
| `GetElementForPlaceProperties` | Свойства для размещения |
| `GetParamValue` | Чтение значения параметра |
| `GetElementsForRule` | Элементы для одного правила |
| `GetParamToReadFromRule` | Параметры для чтения |
| `GetElementForPlace` | Создание/настройка элемента |
| `GetSizePlaceElement` | Размер по сетке |
| `PlaceElements` | Размещение сформированных элементов |

## Зависимости
- `Helpers.hpp` — ParamHelpers, GetSelectedElements
- `Propertycache.hpp` — PROPERTYCACHE()
- `CommonFunction.hpp`

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — команда спецификации
- `TableRenderer` — данные из Spec для таблиц

## Инварианты
- `SpecRule` содержит пары GUID: elements (обрабатываемые) и exsist_elements (существующие для перезаписи)
- `stop_on_error = true` — при ошибке прекращает обработку правила
- `only_visible = true` — обрабатывать только видимые элементы
- `isKM`/`isKZH` — специальные правила для КМ/КЖ (техничка/ведомость расхода стали)
