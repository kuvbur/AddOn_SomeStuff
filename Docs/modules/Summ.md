# Summ — Суммирование свойств

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep).

## Назначение
Запускает суммирование значений свойств элементов и записывает результат в свойство или информацию проекта. [из комментария, Summ.hpp:8]

## Файлы
- `Summ.cpp/hpp`

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SumRule` | position (куда писать), value (что суммировать), criteria (группировка), delimetr ("; "), ignore_val, sum_type, write_to, elemts, rule_name, state, n_ignore, n_write [из комментария, Summ.hpp:9-34] |
| `SumRules` | `GS::HashTable<API_Guid, SumRule>` [по коду] |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `SumSelected` | 29 | Запуск суммирования для выбранных элементов [из комментария] |
| `GetSumValuesOfElements` | 148 | Сбор значений свойств из массива элементов [из комментария] |
| `Sum_GetElement` | 221 | Распределяет элемент в таблицу с правилами суммирования [из комментария] |
| `Sum_Rule` | 362 | Разбирает описание свойства в правило суммирования [из комментария] |
| `Sum_OneRule` | 453 | Суммирование по одному правилу и запись результата [из комментария] |

## Карточки

### `SumSelected(SyncSettings &syncSettings) -> GSErrCode`
- Расположение: `Sources/AddOn/Summ.cpp:29`
- Назначение: запускает суммирование значений свойств для выбранных элементов. [из комментария]
- Контракт: не проверено (тело вне этой сессии не читалось).
- Побочные эффекты: **меняет свойства элементов или информацию проекта** (через Sum_OneRule); читает выделение. [по коду]
- Вызывает: `GetSumValuesOfElements` (:44), `GetSelectedElements` (Helpers.cpp:695), `ElementsWrite` (Helpers.cpp:4625), `WriteInfo` (Helpers.cpp:4726), `SyncArray` (Sync.cpp:456). [из callgraph.json]
- Контракт: обход правил в `ACAPI_CallUndoableCommand` (:58); итоги — в свойство/инфо проекта через WriteInfo. [по коду]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:433). [по коду, grep]

### `Sum_OneRule(SumRule &rule, ParamDictElement &paramToReadelem, ParamDictElement &paramToWriteelem)`
- Расположение: `Sources/AddOn/Summ.cpp:453`
- Назначение: выполняет суммирование по одному правилу и пишет результат в целевые свойства. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись результата в свойство элемента или информацию проекта** (write_to); счётчики n_ignore/n_write. [по коду]
- Вызывается из: `GetSumValuesOfElements` (Summ.cpp:213). [из callgraph.json]
- Вызывает: `AddParamValue2ParamDictElement` (Helpers.cpp:1714), `GetCharCode` (CommonFunction.cpp:1560), `StringUnic` (CommonFunction.cpp:1473), `ToString` (Helpers.cpp:8768). [из callgraph.json]

## Зависимости
- `Helpers.hpp`, `DG.h` [по include]

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — команда суммирования [по коду]
