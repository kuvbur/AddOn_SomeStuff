# Summ — Суммирование свойств

## Назначение
Запускает суммирование значений свойств элементов и записывает результат в свойство или информацию проекта.

## Файлы
- `Summ.cpp/hpp` — основная логика суммирования

## Публичный API

| Функция | Сигнатура | Назначение |
|---------|-----------|------------|
| `SumSelected` | `(SyncSettings&)` | Запуск суммирования для выбранных элементов |
| `GetSumValuesOfElements` | `(GS::Array<API_Guid>&, ParamDictElement&)` | Сбор значений свойств из массива элементов |
| `Sum_GetElement` | `(const GS::Array<API_Guid>&, const GS::HashTable<...>&, ParamDictElement&, SumRules&)` | Распределение элементов по правилам суммирования |
| `Sum_Rule` | `(const API_PropertyDefinition&, SumRule&)` | Разбор описания свойства в правило суммирования |
| `Sum_OneRule` | `(SumRule&, ParamDictElement&, ParamDictElement&)` | Суммирование по одному правилу |

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `SumRule` | Правило суммирования: position (куда писать), value (что суммировать), criteria (группировка), sum_type (NUM_SUM/TEXT_SUM/MIN_SUM/MAX_SUM), write_to (элемент/проект) |
| `SumRules` | `GS::HashTable<API_Guid, SumRule>` — словарь правил |

## Зависимости
- `Helpers.hpp` — ParamHelpers, GetSelectedElements
- `DG.h` — диалоговые элементы
- `SyncSettings` — настройки синхронизации

## Зависимости (используется в)
- `SomeStuff_Main.cpp` — через MenuCommandHandler (команда суммирования)
