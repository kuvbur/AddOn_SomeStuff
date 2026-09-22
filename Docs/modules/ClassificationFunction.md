# ClassificationFunction — Авто-классификация

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Загрузка систем классификации, поиск классов, назначение автокласса по описанию свойства (`some_stuff_class`). [из комментария, ClassificationFunction.hpp:22-25, 58-61]

## Файлы
- `ClassificationFunction.cpp/hpp`

## Ключевые типы [из комментария, ClassificationFunction.hpp:11-20]
`ClassificationValues` (system, item, parentname, itemname) → `ClassificationDict` (классы системы) → `SystemDict` (системы).

## Публичный API

| Функция | Назначение |
|---------|------------|
| `GetAllClassification` | Загружает все классы из систем в словарь [из комментария]; определение ClassificationFunction.cpp:21, вызывается из Propertycache.hpp:613 (ReadClassification) [по коду, grep] |
| `GatherAllDescendantOfClassification` | Перебирает потомков класса [из комментария, :29] |
| `AddClassificationItem` | Добавляет элемент классификации в словарь [из комментария, :34] |
| `GetFullName` | Полное имя класса с иерархией [из комментария, :42] |
| `FindClass` (2 перегрузки) | Поиск класса по имени/GUID [из комментария, :49, :55] |
| `GetSystemName` | Имя системы по GUID [из комментария, :52] |
| `SetAutoclass` | Назначает автокласс элементу, если нет классификации (описание `some_stuff_class`) [из комментария, :61] — карточка |
| `ReadSystemDict` | Читает словарь из кэша или проекта [из комментария, :64] |

## Карточки

### `ClassificationFunc::SetAutoclass(const API_Guid elemGuid)`
- Расположение: `Sources/AddOn/ClassificationFunction.cpp` (строка не проверена)
- Назначение: назначает элементу автокласс, если у него ещё нет классификации; класс по описанию с `some_stuff_class`. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **меняет классификацию элемента** (ACAPI-запись). [по коду; не проверено detail]

## Зависимости
- `Constants.hpp` [по include]

## Зависимости (используется в)
`Propertycache` (systemdict), `BrowserPalette`, `Helpers` [по коду]

## Инварианты
- `itemname` — ключ словаря, должен быть уникальным в системе [из комментария, ClassificationFunction.hpp:15]
