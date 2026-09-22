# spec/Spec_libpart — Спецификация библиотечных элементов

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Разбор и хранение данных спецификаций/списков для библиотечных элементов (прокат, арматура, материалы). [из комментария, Spec_libpart.hpp:7]

## Ключевые типы (namespace ListData) [из комментария, Spec_libpart.hpp:13-159]
`ArmUch`, `Arm`, `Prokat`, `Mat`, `Subpos` (prokat/mat/arm + IsEmpty/Clear), `LibElement` (subpos + keys для поиска в библиотеке), `LibElements` (HashTable<API_Guid, LibElement>).

## Публичный API

| Функция | Назначение |
|---------|------------|
| `AddProkat` | Парсинг и добавление проката в сметную структуру [из комментария, :164] |
| `GetAllKeys` | Все пары «имя параметра–значение» LibElement [из комментария, :173] |
| `Add` | Добавление общего материала/элемента [из комментария, :178] |
| `AddLibdataToParamValueDict` | Запись распарсенных сметных данных в словарь параметров [из комментария, :183] — строки .cpp не проверены |

## Зависимости
- `CommonFunction.hpp` [по include]

## Зависимости (используется в)
Spec (GetParamValue, GetElementsForRule), Helpers [по коду]
