# TestFunc — Локальное тестирование

> Хеш коммита: 493caf5 (2026-09-22). Пофункциональные записи не приведены (682 символа — см. symbols.json); помечено неполным покрытием в _progress.md.

## Назначение
Вспомогательные функции для локального тестирования и отладки. Активен только под `TESTING`. [из комментария, TestFunc.hpp:16]

## Файлы
- `Sources/AddOn/TestFunc.cpp/hpp` (hpp целиком под `#ifdef TESTING`)

## Публичный API (namespace TestFunc, void-функции) [из комментариев, TestFunc.hpp:17-60]
`Test` — запуск набора проверок helpers; `TestGetTextLineLength` — длина текстовой строки; `TestCalc` — арифметика; `TestFormula` — формулы; `TestFormatString`/`TestFormatStringFormula` — форматирование; `TestConvert*` (ToParamValue/Attribute/Property/Definition) — преобразования; `TestSetParamValueSourseByName`, `TestSetrawNameFromProperty`, `TestCheckIgnoreVal`, `TestReadProperty` — прочее (далее по hpp до :126).

## Зависимости
- `api_headers/APICommon25/26/27.h` [по include]

## Примечания
- Прод-код в тестовых задачах read-only; вывод DBprnt/DBtest, ошибки — grep "ERROR IN TEST" test_results.txt (AGENTS.md §10)
