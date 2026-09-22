# MEPv1 — Обработка данных MEP

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Чтение и обработка данных MEP для ArchiCAD 27+. [из комментария, MEPv1.hpp:81]

## Файлы
- `MEPv1.cpp/hpp`

## Публичный API

| Функция | Назначение |
|---------|------------|
| `GetSubElementOfRouting` | Собирает подэлементы маршрутизации для MEP-элемента [из комментария, MEPv1.hpp:102] |
| `GetSubElement` | Собирает все дочерние элементы MEP-объекта [из комментария, :106] |
| `ReadMEP` | Читает свойства MEP из заголовка в ParamDictValue [из комментария, :109] — карточка |
| `ClearRoutingSubelemCache` | Очищает кэш подэлементов [из комментария, :112] |
| `GetMEPData` и AC28+ Read*PreferenceTable | Чтение таблиц предпочтений MEP (duct/pipe/transition) [из комментария, :114-173] — строки .cpp не проверены |

## Карточки

### `MEPv1::ReadMEP(const API_Elem_Head &elem_head, ParamDictValue &paramByType) -> bool`
- Расположение: `Sources/AddOn/MEPv1.cpp` (строка не проверена)
- Назначение: читает свойства MEP из заголовка элемента в словарь параметров. [из комментария]
- Контракт: false — элемент не содержит MEP-данных. [по коду; не проверено detail]
- Побочные эффекты: заполняет paramByType; чтение через MEP-adapter (только чтение). [по коду; не проверено detail]

## Зависимости
- `ACAPI/MEP*` (27+/28+/29+), `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp` [по include]

## Инварианты
- rawname-константы `{@mep:...}` — 15 штук [из комментария, MEPv1.hpp:84-100]
- Ветки ServerMainVers_2700/2800/2900 — API MEP существенно меняется [по коду]
