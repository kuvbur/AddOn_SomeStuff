# ClassificationFunction — Авто-классификация

## Назначение
Загрузка систем классификации, поиск классов по имени, назначение автокласса элементам на основе описания свойства (`some_stuff_class`).

## Файлы
- `ClassificationFunction.cpp/hpp` — авто-классификация

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `ClassificationValues` | Один класс: system, item, parentname, itemname |
| `ClassificationDict` | `HashTable<string, ClassificationValues>` — классы в системе |
| `SystemDict` | `HashTable<string, ClassificationDict>` — системы с вложенными классами |

## Публичный API

| Функция | Назначение |
|---------|------------|
| `GetAllClassification` | Загрузка всех классов из систем в словарь |
| `GatherAllDescendantOfClassification` | Сбор всех потомков заданного класса |
| `AddClassificationItem` | Добавление одного класса в словарь |
| `GetFullName` | Полное имя класса с иерархией |
| `FindClass` | Поиск класса по имени → GUID |
| `GetSystemName` | Имя системы по GUID |
| `FindClass` | Поиск по паре (system GUID, class GUID) |
| `SetAutoclass` | Назначение автокласса элементу (по `some_stuff_class`) |
| `ReadSystemDict` | Чтение словаря систем из кэша |

## Зависимости
- `Constants.hpp`

## Зависимости (используется в)
- `Propertycache` (ReadClassification → systemdict)
- `BrowserPalette` (показ классификаций)
- `Helpers` (при выборе элементов)

## Инварианты
- `ClassificationDict` использует `itemname` как ключ — он должен быть уникальным в пределах системы
- `reversesystemdict` в Propertycache — обратное отображение (system GUID + class GUID → имя класса)
