# CommonFunction — Утилиты

## Назначение
Общие вспомогательные структуры и функции: чтение/запись свойств, работа с этажами, форматирование значений, базовые преобразования, отладочный вывод (DBprnt/DBtest), QR-коды, обработка строк.

## Файлы
- `CommonFunction.cpp/hpp` — основные утилиты (~513 строк в hpp)

## Ключевые структуры

| Структура | Назначение |
|-----------|------------|
| `Story` | Этаж проекта (index, level) |
| `Stories` | `GS::Array<Story>` — массив этажей |
| `FormatString` | Параметры форматирования чисел (н_zero, stringformat, needRound, krat, koeff и др.) |
| `ParamValueData` | Унифицированное значение параметра (тип, строка, int, double, formula и др.) |
| `ParamValueComposite` | Данные одного слоя конструкции |
| `ParamComposite` | Состав конструкции целиком |
| `ParamValue` | Полное описание параметра с флагами происхождения (fromProperty, fromGDLparam, fromMaterial и 20+ флагов) |
| `ProcessWindowGuard` | RAII-обёртка для окна прогресса Archicad |

## Утилиты (выборка)

| Функция | Назначение |
|---------|------------|
| `DBprnt` / `DBtest` | Отладочный вывод в окно отчёта |
| `msg_rep` | Сообщение об ошибке с модулем |
| `GetSelectedElements2` | Выбор элементов без настроек синхронизации |
| `CallOnSelectedElem2` | Вызов функции для каждого выбранного элемента |
| `GetStories` | Информация об этажах |
| `GetFloorIndexAndOffset` / `GetzPos` | Преобразование координат Z ↔ этаж |
| `TextToQRCode` | Генерация QR-кода |
| `UniStringToDouble`, `round_nzero`, `is_equal` | Преобразование и сравнение |
| `StringSplt`, `StringUnic` | Работа со строками |
| `EvalExpression` | Вычисление выражений в `< >` |
| `UnhideUnlockElementLayer` | Снятие скрытия/блокировки слоя |

## Зависимости
- `ACAPinc.h`, `api_headers/APIEnvir.h`
- `api_headers/APICommon*.h` (условно по версии)
- `Constants.hpp`
- `third_party/alphanum.h` (сортировка)
- `third_party/exprtk.h` (вычисления)
- `DG.h`, `Point2D.hpp`, `Polygon2DData.h`

## Зависимости (используется в)
Практически во всех модулях (Helpers, Sync, Propertycache, Spec, ResetProperty, MEPv1 и др.)

## Инварианты
- `ParamValue` содержит ~20 boolean-флагов происхождения данных — каждый должен устанавливаться явно при чтении из конкретного источника
- `ProcessWindowGuard` корректно работает только один экземпляр за раз (вложенные вызовы проблематичны)
