# ReNum — Перенумерация

## Назначение
Перенумерация позиций чертежей по правилам, заданным в свойствах. Поддерживает числовые и текстовые позиции, нулевое заполнение, группировку по критериям и разделителям.

## Файлы
- `ReNum.cpp/hpp` — перенумерация

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `RenumPos` | Позиция: GUID, текст, числовая часть, префикс, суффикс, char code |
| `RenumElem` | Элемент правила: массив позиций + mostFrequentPos |
| `RenumRule` | Правило: state, oldalgoritm, flag, position, criteria, delimetr, nulltype, nullcount |
| `Values` | `std::map<string, RenumElem>` (с alphanum сортировкой) |
| `TypeValues` | `unordered_map<RenumMode, Values>` — по типу нумерации |
| `Delimetr` | `unordered_map<string, TypeValues>` — по разделителю |
| `Rules` | `GS::HashTable<API_Guid, RenumRule>` — правила по GUID |

## Публичный API

| Функция | Назначение |
|---------|------------|
| `ReNumSelected` | Запуск перенумерации выбранных элементов |
| `RenumDG` | Диалог выбора правил |
| `GetRenumElements` | Сбор элементов для перенумерации |
| `ReNumHasFlag` | Проверка переключателя флага |
| `ReNumGetFlag` | Состояние флага перенумерации |
| `ReNum_GetElement` | Обработка одного элемента |
| `GetMostFrequentPos` | Наиболее частая позиция |
| `GetPos` | Позиция по критерию и разделителю |
| `ElementsSeparation` | Разделение элементов по группам |
| `ReNumOneRule` | Применение одного правила |

## Зависимости
- `Helpers.hpp`
- `third_party/alphanum.h` (натуральная сортировка)

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — через MenuCommandHandler
- `CommonFunction` (GetCharCode, alphanum_less)

## Инварианты
- `RenumPos` поддерживает как числовые (1, 2, 3), так и текстовые (А, Б, В) позиции
- `oldalgoritm` флаг в `RenumRule` определяет старый vs новый алгоритм расчёта
- Сортировка `Values` использует `doj::alphanum_less` — натуральная сортировка (A1, A2, ..., A10, не A1, A10, A2)
