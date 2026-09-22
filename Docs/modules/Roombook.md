# Roombook — Спецификация отделки

> Хеш коммита: 493caf5 (2026-09-22).

## Назначение
Спецификация отделки: генерация ведомостей отделочных материалов и работ из модели. [по коду; самый большой модуль (~5600 строк), пофункциональные карточки не приведены — см. symbols.json (1076 символов)]

## Файлы
- `Roombook.cpp`

## Точка входа
- `RoomBook` — `Sources/AddOn/Roombook.cpp:57`; вызывается из `MenuCommandHandler` (SomeStuff_Main.cpp:457). [из callgraph.json]
- Внутренние функции Roombook (1076 символов) — пофункционально не разобраны (см. symbols.json).

## Зависимости
- `CommonFunction.hpp`, `Helpers.hpp`, `Propertycache.hpp` [по include]

## Точка входа
- `RoomBook` — `Sources/AddOn/Roombook.cpp:57`; вызывается из `MenuCommandHandler` (SomeStuff_Main.cpp:457). [из callgraph.json]
- Внутренние функции Roombook (1076 символов) — пофункционально не разобраны (см. symbols.json).

## Зависимости (используется в)
`SomeStuff_Main` [по коду]

## Инварианты и подводные камни
- Утечка memo при ошибке GetMemo (:2028-2034) — **исправлена** (FIX 2026-09-12: Dispose перед continue) [из комментария + проверено 2026-09-22]
- AGENTS.md §16: recreation of finish elements — out of scope [из AGENTS.md]
