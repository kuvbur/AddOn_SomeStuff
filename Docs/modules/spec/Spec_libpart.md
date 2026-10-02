# spec/Spec_libpart — Спецификация библиотечных элементов

> Хеш состояния: `2101967` (2026-10-02), строки — определения в `Spec_libpart.cpp`
> (1-based, собраны clangd и сверены с исходником). Карточка обновлена
> 2026-10-02: добавлены проверенные строки всех 14 функций и карта вызовов.

## Назначение
Разбор и хранение данных спецификаций/списков для библиотечных элементов
(прокат, арматура, материалы, позиции). [из комментария, Spec_libpart.hpp:7]

## Файлы
- `spec/Spec_libpart.cpp/hpp`

## Ключевые типы (namespace ListData) [из комментария, Spec_libpart.hpp:13-159]
`ArmUch`, `Arm`, `Prokat`, `Mat`, `Subpos` (prokat/mat/arm + IsEmpty/Clear),
`LibElement` (subpos + keys для поиска в библиотеке),
`LibElements` (HashTable<API_Guid, LibElement>).

## Публичный API

Все 14 функций модуля — строки проверены по исходнику. Прежние строки взяты из
`.hpp` и были неточны: `AddProkat` числился на :164 при определении на :322,
`GetAllKeys` на :173 при :383, `Add` на :178 при :441,
`AddLibdataToParamValueDict` на :183 при :482.

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `GetSubposKey` | 17 | Ключ позиции из координат, нижний регистр [по коду] |
| `GetKey (const LibElement &)` | 32 | Сводный ключ библиотечного элемента [по коду] |
| `GetKey (const Subpos &)` | 41 | Ключ позиции [по коду] |
| `GetKey (const Mat &)` | 51 | Ключ материала: имя + позиция [по коду] |
| `GetKey (const Arm &)` | 75 | Ключ арматуры: позиция + диаметр через `Printf` [по коду] |
| `GetKey (const Prokat &)` | 99 | Ключ проката: позиция + сорт через `Printf` [по коду] |
| `GetParam` | 126 | Значение параметра зоны, разобранное по фильтру [по коду] |
| `AddMat` | 153 | Разбор и добавление материала в сметную структуру [из комментария] |
| `AddArm` | 201 | Разбор и добавление арматуры (диаметр через `DoubleM2IntMM`) [из комментария] |
| `AddSubpos` | 263 | Разбор и добавление позиции [из комментария] |
| `AddProkat` | 322 | Разбор и добавление проката [из комментария] |
| `GetAllKeys` | 383 | Все пары «имя параметра–значение» `LibElement` [из комментария, hpp:173] |
| `Add` | 441 | Добавление общего материала/элемента, диспетчер по типу зоны [из комментария, hpp:178] |
| `AddLibdataToParamValueDict` | 482 | Запись распарсенных сметных данных в словарь параметров [из комментария, hpp:183] |

## Карта вызовов

Собрана `prepareCallHierarchy` (incoming/outgoing) по каждой функции, строки
1-based. SDK-вызовы и методы `GS::`-контейнеров не приводятся — они не часть
контракта модуля; полная карта в `Docs/_generated/callgraph.json`.

### `ListData::Add(LibElement &, GS::UniString &name, GS::UniString &unitcode, double &qty)`
- Расположение: `Spec_libpart.cpp:441`
- Назначение: диспетчер по типу зоны — вызывает один из четырёх разборщиков. [из комментария]
- Вызывает: `AddArm` (:457), `AddProkat` (:459), `AddMat` (:461, :463),
  `AddSubpos` (:465), `StringSplt` (CommonFunction.cpp:1657).
- Вызывается из: `ParamHelpers::ReadListData` (Helpers.cpp:6706).
- Побочные эффекты: наполняет `LibElement` — только словари в памяти, без
  обращения к модели. [по коду]

### `ListData::AddLibdataToParamValueDict(...) -> bool`
- Расположение: `Spec_libpart.cpp:482`
- Назначение: переносит распарсенные сметные данные в `ParamValue`. [из комментария]
- Вызывает: `AddDoubleValueToParamDictValue` (Helpers.cpp:1900),
  `AddLengthValueToParamDictValue` (:1885), `AddStringValueToParamDictValue` (:1914),
  `msg_rep` (CommonFunction.cpp:407).
- Вызывается из: `Spec::SpecValueReader::Read` (Spec.cpp:1693) — единственный
  путь, то есть чтение list-data невозможно без этой функции. [по коду]

### `ListData::GetAllKeys(const LibElement &el)`
- Расположение: `Spec_libpart.cpp:383`
- Назначение: все пары «имя параметра–значение» для поиска в библиотеке. [из комментария]
- Вызывает: `DBprnt` (CommonFunction.cpp:307) — диагностика.
- Вызывается из: `ParamHelpers::Read` (Helpers.cpp:5751).

### `ListData::GetSubposKey(const GS::UniString &subpos)`
- Расположение: `Spec_libpart.cpp:17`
- Вызывается из: `GetKey(LibElement)` (:33), `GetKey(Subpos)` (:42),
  `AddMat` (:176), `AddSubpos` (:291), `AddArm` (:236), `AddProkat` (:360).
- Контракт: результат в нижнем регистре (`SetToLowerCase`), разделители приведены
  к единому виду (`ReplaceAll`) — иначе одинаковые позиции дали бы разные ключи
  и строки не объединились бы. [по коду]

### Ключи `GetKey` — по одному на тип сметы
| Перегрузка | Строка | Вызывается из |
|---|---|---|
| `GetKey (const Mat &)` | 51 | `AddMat` (:181) |
| `GetKey (const Arm &)` | 75 | `AddArm` (:241) |
| `GetKey (const Prokat &)` | 99 | `AddProkat` (:365) |
| `GetKey (const LibElement &)` | 32 | внутри модуля не вызывается (публичный API для внешних потребителей) |
| `GetKey (const Subpos &)` | 41 | внутри модуля не вызывается (то же) |

## Границы
- Модуль **не обращается к модели**: только разбор строк и работа со
  словарями. `ACAPI_*` в нём нет — все исходящие рёбра уходят в
  `CommonFunction.cpp`, `Helpers.cpp` и GS-контейнеры. [по карте вызовов]
- `Add` — единственная точка входа извне; четыре разборщика (`AddMat`,
  `AddArm`, `AddSubpos`, `AddProkat`) вызываются только из неё. [по коду]

## Зависимости
- `CommonFunction.hpp` [по include]

## Зависимости (используется в)
- `spec/Spec.cpp` — `SpecValueReader::Read` вызывает `AddLibdataToParamValueDict`
- `Helpers.cpp` — `ParamHelpers::Read` → `GetAllKeys`,
  `ParamHelpers::ReadListData` → `Add` [по коду]

## Инварианты
- Ключи всегда в нижнем регистре и с нормализованными разделителями; иначе две
  одинаковые позиции не объединятся в одну строку спецификации. [по коду]
- `AddArm`/`AddProkat` приводят длину к миллиметрам через `DoubleM2IntMM`, а
  `AddMat`/`AddSubpos` читают число напрямую `UniStringToDouble` — разница
  обязательна: диаметры и сорта округляются, количества — нет. [по коду]