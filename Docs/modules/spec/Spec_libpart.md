# spec/Spec_libpart — Спецификация библиотечных элементов

## Назначение
Разбор и хранения данных спецификаций/списков для библиотечных элементов (металлопрокат, арматура, материалы). Используется для чтения информации из элементов (COMPONENT, GDL lists).

## Файлы
- `spec/Spec_libpart.cpp/hpp` — разбор данных

## Ключевые типы (namespace ListData)

| Тип | Описание |
|-----|----------|
| `ArmUch` | Участок гнутого стержня (l, dop, ang) |
| `Arm` | Арматурное изделие (pos, klass, diam, qty, ves, isGnut, isPm, naen, unit, key) |
| `Prokat` | Металлопрокат/профиль (pos, tip_konstr, obozn_mater, mater, obozn, tip_profile, qty, ves) |
| `Mat` | Строительный материал (pos, tip_konstr, obozn, naen, qty, ves, unit, key) |
| `Subpos` | Сборочная позиция (prokat, mat, arm hash-таблицы) |
| `LibElement` | Библиотечный элемент (subpos, pos, obozn, naen, qty, ves, keys) |
| `LibElements` | HashTable<API_Guid, LibElement> — по GUID элемента |

## Публичный API

| Функция | Назначение |
|---------|------------|
| `AddProkat` | Добавление проката в структуру |
| `GetAllKeys` | Все пары ключ-значение LibElement |
| `Add` | Добавление материала/элемента |
| `AddLibdataToParamValueDict` | Запись сметных данных в словарь |

## Структура данных
```
LibElement
├── subpos (HashTable<string, Subpos>)
│   ├── prokat (HashTable<string, Prokat>)
│   ├── mat (HashTable<string, Mat>)
│   └── arm (HashTable<string, Arm>)
├── pos, obozn, naen, qty, ves, unit, key
└── keys (Array<Pair<string, string>>) — для поиска в библиотеке
```

## Зависимости
- `CommonFunction.hpp`

## Зависимости (используется в)
- `Spec` (GetParamValue, GetElementsForRule)
- `Helpers` (чтение компонентов)
