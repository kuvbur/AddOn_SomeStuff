# CommonFunction — Утилиты

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep; выборочные).

## Назначение
Общие вспомогательные структуры и функции: чтение/запись свойств, этажи, форматирование, отладочный вывод, QR, строки. [из комментария, CommonFunction.hpp:47-48]

## Файлы
- `CommonFunction.cpp/hpp` (hpp 513 строк)

## Ключевые типы [из комментария]
`Story`/`Stories` — этажи; `FormatString` — параметры округления/формата; `ParamValueData` — унифицированное значение; `ParamValueComposite`/`ParamComposite` — слои конструкции; `ParamValue` — полное описание параметра с ~20 флагами источника (fromProperty, fromGDLparam, fromMaterial и др.); `ProcessWindowGuard` — RAII окна прогресса (InitProcessWindow/CloseProcessWindow, AC27+ ACAPI_ProcessWindow_*). [из комментария, CommonFunction.hpp:53-232]

## Публичный API (выборка)

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `GetUnicGuid` | 29 | Оставляет в массиве только уникальные GUID с сохранением порядка [из комментария] |
| `GetStories` | 99 | Читает этажи проекта (индекс–уровень) [из комментария] |
| `TextToQRCode` | 169/231 | Генерация QR-кода из текста [из комментария] |
| `GetSelectedElements2` | 806 | Выбор элементов без чтения настроек (assertIfNoSel, onlyEditable) [из комментария] |
| `GetTypeByGUID` | 897 | Тип объекта по GUID [из комментария] |
| `IsTeamwork` | 1315 | Статус и ID пользователя Teamwork [из комментария] |
| `EvalExpression` | 1336 | Вычисление выражений в `< >`; невычислимое → пустота [из комментария] |
| `UnhideUnlockElementLayer` | 2448/2472/2496? | Снятие скрытия/блокировки слоя (3 перегрузки) [из комментария; строки 3-й не проверены] |
| `DBprnt` / `DBtest` / `msg_rep` | — | Отладочный вывод / тест / сообщение об ошибке [из комментария; строки не проверены] |
| `StringSplt`, `StringUnic`, `UniStringToDouble`, `round_nzero`, `is_equal`, `check_accuracy` | — | Строки и числа [из комментария; строки не проверены] |

## Карточки

### `UnhideUnlockElementLayer(const API_Guid &elemGuid)`
- Расположение: `Sources/AddOn/CommonFunction.cpp:2448`
- Назначение: если слой элемента скрыт или заблокирован — делает видимым/разблокированным (по GUID → заголовок → индекс слоя). [из комментария, CommonFunction.hpp:467-476]
- Контракт: 3 перегрузки; по SDK ACAPI_Attribute_Get/Set сбрасывает flags & 1 (hidden) и & 2 (locked). [из комментария]
- Побочные эффекты: **меняет атрибут слоя** (видимость/блокировка) — влияет на весь слой. [из комментария]

### `EvalExpression(GS::UniString &unistring_expression) -> bool`
- Расположение: `Sources/AddOn/CommonFunction.cpp:1336`
- Назначение: вычисление выражений в `< >`; что не может вычислить — заменит на пустоту. [из комментария]
- Побочные эффекты: мутирует входную строку. [из комментария]

## Зависимости
- `api_headers/*` (APICommon22-29), `Constants.hpp`, `third_party/alphanum.h`, `third_party/exprtk.h`, `DG.h`, `Point2D.hpp` [по include]

## Зависимости (используется в)
Практически все модули [по include]

## Инварианты
- `ProcessWindowGuard` — окно прогресса закрывается автоматически даже при исключении [из комментария, CommonFunction.hpp:214-232]
