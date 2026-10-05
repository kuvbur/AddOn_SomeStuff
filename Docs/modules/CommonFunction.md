# CommonFunction — Утилиты

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep; выборочные).

## Назначение
Общие вспомогательные структуры и функции: чтение/запись свойств, этажи, форматирование, отладочный вывод, QR, строки. [из комментария, CommonFunction.hpp:47-48]

## Файлы
- `CommonFunction.cpp/hpp` (hpp 513 строк)

## Ключевые типы [из комментария]
`Story`/`Stories` — этажи; `FormatString` — параметры округления/формата; `ParamValueData` — унифицированное значение; `ParamValueComposite`/`ParamComposite` — слои конструкции; `ParamValue` — полное описание параметра с ~20 флагами источника (fromProperty, fromGDLparam, fromMaterial и др.); `ProcessWindowGuard` — RAII окна прогресса (InitProcessWindow/CloseProcessWindow, AC27+ ACAPI_ProcessWindow_*); `SuspendGroupsGuard` — RAII режима «приостановить группировку» (AC27+ ACAPI_View_IsSuspendGroupOn/ACAPI_Grouping_Tool, ниже ACAPI_Environment+ACAPI_Element_Tool). [из комментария, CommonFunction.hpp:53-260]

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
| `DoubleToInt32` / `DoubleToInt32RoundUp` | 955 / 991 | Приведение `double` к `Int32` с насыщением на границах диапазона (NaN → 0, вне диапазона → `INT32_MIN/MAX`), сообщение через `msg_rep` [CommonFunction.cpp, проверено grep] |

## Карточки

### `SafeWriteReport(GS::UniString text, bool withDial) -> void` (#264, рабочее дерево)
- Определение: `Sources/AddOn/CommonFunction.cpp:410`; объявление: `CommonFunction.hpp:339`. [по коду после clang-format]
- Принимает готовый текст по значению; удаляет все `%` через `ReplaceAll` из копии и передаёт её в `ACAPI_WriteReport`. Значение `withDial` сохраняется; исходные правила не меняются. Не является вариадической форматной функцией. [по коду]
- `msg_rep` использует обёртку; обе перегрузки `DBprnt` очищают копии строк также перед отладочным форматным выводом. В Roombook/Sync/MEPv1 заменены только авторские вызовы отчёта; api_headers/APICommon не мигрировались по указанию владельца. [по коду]
- Компиляция/runtime не выполнялись по решению владельца; LSP/generated not verified: MCP вернул дерево CommonFunction без новой функции. Падение после исправления не воспроизводилось.

### `UnhideUnlockElementLayer(const API_Guid &elemGuid)`
- Расположение: `Sources/AddOn/CommonFunction.cpp:2448`
- Назначение: если слой элемента скрыт или заблокирован — делает видимым/разблокированным (по GUID → заголовок → индекс слоя). [из комментария, CommonFunction.hpp:467-476]
- Контракт: 3 перегрузки; по SDK ACAPI_Attribute_Get/Set сбрасывает flags & 1 (hidden) и & 2 (locked). [из комментария]
- Побочные эффекты: **меняет атрибут слоя** (видимость/блокировка) — влияет на весь слой. [из комментария]

### `EvalExpression(GS::UniString &unistring_expression) -> bool`
- Расположение: `Sources/AddOn/CommonFunction.cpp:1336`
- Назначение: вычисление выражений в `< >`; что не может вычислить — заменит на пустоту. [из комментария]
- Побочные эффекты: мутирует входную строку. [из комментария]

### `DoubleToInt32(double value, const GS::UniString &modulename, const GS::UniString &info = EMPTYSTRING, const API_Guid &elemGuid = APINULLGuid) -> Int32` (#221)
- Расположение: `Sources/AddOn/CommonFunction.cpp:955`, объявление — `CommonFunction.hpp:428`
- Назначение: приведение `double` к `Int32` с насыщением на границах диапазона. [по коду]
- Контракт: `NaN` → `0`; `value > INT32_MAX` → `INT32_MAX`; `value < INT32_MIN` → `INT32_MIN`; внутри диапазона — усечение к нулю, как при обычном C-касте. Каждый выход за границы (включая `NaN`) выводится через `msg_rep` с `APIERR_BADVALUE`; тип результата не меняется. [по коду]
- Побочные эффекты: запись в отчёт ArchiCAD (`msg_rep`); `elemGuid` добавляется в текст сообщения при ненулевом значении. [по коду]

### `DoubleToInt32RoundUp(double value, ...) -> Int32` (#221)
- Расположение: `Sources/AddOn/CommonFunction.cpp:990`, объявление — `CommonFunction.hpp:437`
- Назначение: то же, что `DoubleToInt32`, но с округлением дробной части вверх — для мест, где прежде каст сопровождался `if (intValue / 1 < doubleValue) intValue += 1`. [по коду]
- Контракт: инкремент выполняется только если результат строго меньше значения **и** меньше `INT32_MAX` — без второй проверки насыщение до `INT32_MAX` переполнило бы переменную. [по коду]
- Побочные эффекты: те же, что у `DoubleToInt32`. [по коду]

Вызывается из: `Helpers.cpp` (`ReadID`, `NumToString`, `ConvertToParamValue` ×3, `ConvertToParamValue(API_Property)` ×3, `ConvertStringToParamValue`, `ConvertDoubleToParamValue`, `ConvertToParamValue(API_IFCProperty)` ×2, `ConvertByFormatString`), `Dimensions.cpp::DimParse`, `Roombook.cpp` ×3, `spec/Spec.cpp` ×3, а также внутри `CommonFunction.cpp` — `ceil_mod_classic` (:1079), `DoubleM2IntMM` (:1093), `DelimTextLine` (:1244), `API_AttributeIndexFindByName` (:2564). [проверено grep по `Sources/AddOn`]


## Зависимости
- `api_headers/*` (APICommon22-29), `Constants.hpp`, `third_party/alphanum.h`, `third_party/exprtk.h`, `DG.h`, `Point2D.hpp` [по include]

## Зависимости (используется в)
Практически все модули [по include]

## Инварианты
- `ProcessWindowGuard` — окно прогресса закрывается автоматически даже при исключении [из комментария, CommonFunction.hpp:214-232]
- `SuspendGroupsGuard` — если режим «приостановить группировку» уже включён (`suspGrp == true`, группировка отключена), guard ничего не делает. Если группировка активна (`suspGrp == false`) — включает режим на время своей жизни и выключает в деструкторе. Если переключение не удалось — восстанавливать нечего, деструктор молчит. `APITool_SuspendGroups` — переключатель On/Off, массив GUID игнорируется. Копирование запрещено. AC22 — заглушка, `APIEnv_IsSuspendGroupOnID` не существует. [из комментария, CommonFunction.hpp:211-253]
