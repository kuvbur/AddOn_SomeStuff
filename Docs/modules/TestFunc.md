# TestFunc — Локальное тестирование

> Хеш коммита: f8f599c (2026-09-22)

## Назначение модуля
Вспомогательные функции для локального тестирования и отладки add-on. Активен только под `TESTING`. [из комментария, TestFunc.hpp:16]

## Файлы модуля
- `Sources/AddOn/TestFunc.cpp/hpp` (hpp целиком под `#ifdef TESTING`)

## Публичный API (namespace TestFunc)

Все функции — тестовые проверки, возвращают void [из комментариев]:

| Функция | Назначение |
|---------|------------|
| `Test` | Запускает набор локальных проверок основных helpers |
| `TestGetTextLineLength` | Вычисление длины текстовой строки в нестандартных случаях |
| `TestCalc` | Арифметические и логические операции внутренних функций |
| `TestFormula` | Формульный парсинг и вычисление |
| `TestFormatString` | Форматирование строк по правилам add-on |
| `TestFormatStringFormula` | Форматирование строк на основе формул |
| `TestConvertToParamValue` | Преобразование значений в ParamValue |
| `TestConvertAttributeToParamValue` | Преобразование атрибутов в ParamValue |
| `TestConvertPropertyToParamValue` | Преобразование свойств в ParamValue |
| `TestConvertPropertyDefinitionToParamValue` | Преобразование определений свойств в ParamValue |
| `TestSetParamValueSourseByName` | Определение источника параметра по raw-name |
| `TestSetrawNameFromProperty` | Извлечение raw-name из описания свойства |
| `TestCheckIgnoreVal` | Правила игнорирования отдельных значений |
| `TestReadProperty` | Чтение свойств элемента |

(далее по hpp до строки 126 — полный список см. `Docs/_generated/symbols.json`, модуль TestFunc, 682 символа)

## Зависимости
- `api_headers/APICommon25/26/27.h` (условно по версии)
- Тестируемые модули: Helpers, CommonFunction

## Примечания
- По правилам AGENTS.md §10: в задачах по тестам прод-код read-only; вывод через DBprnt/DBtest, ошибки — grep "ERROR IN TEST" test_results.txt
- Полные пофункциональные записи не приведены (только заголовочные комментарии); см. `Docs/_generated/symbols.json` — помечено как неполное покрытие в `_progress.md`