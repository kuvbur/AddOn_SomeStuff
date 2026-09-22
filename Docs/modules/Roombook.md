# Roombook — Спецификация отделки

## Назначение
Спецификация отделки: генерация ведомостей отделочных материалов и работ из модели ArchiCAD.

## Файлы
- `Roombook.cpp` — основная логика (~5686 строк — самый большой модуль по namespace)
- `Roombook.hpp` — объявления

## Статус документирования
См. `docs/modules/roombook.md` для полной документации (отдельный файл).

## Зависимости
- `CommonFunction.hpp` — GetStories, FormatString, DBprnt
- `Helpers.hpp` — GetSelectedElements, ParamHelpers
- `Propertycache.hpp` — PROPERTYCACHE()
- `TableRenderer` — отображение в виде таблицы

## Зависимости (используется в)
- `SomeStuff_Main` — через MenuCommandHandler
- `TableRenderer` — данные для отображения

## Примечания
- См. AGENTS.md §16: Roombook — recreation of finish elements — out of scope
- Самый большой модуль по размеру (~5686 строк в namespace)
