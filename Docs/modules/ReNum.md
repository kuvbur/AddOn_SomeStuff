# ReNum — Перенумерация

> Хеш коммита: 493caf5 (2026-09-22). Номера строк — определения в `.cpp` (1-based, проверены grep).
> Обновлено 2026-10-02 (#249): добавлена формульная разновидность `Renum{…}` — номера строк ниже пересчитаны по текущему дереву.

## Назначение
Перенумерация позиций по правилам из свойств: числовые и текстовые позиции, нулевое заполнение, группировка по критериям и разделителям. [по коду]

## Файлы
- `ReNum.cpp/hpp`

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `RenumPos` | Позиция: GUID, текст, число, префикс/суффикс, char code [из комментария, ReNum.hpp:11-138] |
| `RenumElem` | Массив позиций + mostFrequentPos [из комментария] |
| `RenumRule` | state, oldalgoritm, flag, position, criteria, delimetr, **criteria_formula, delimetr_formula**, nulltype, nullcount, elemts [ReNum.hpp:149-188] |
| `Rules` | `HashTable<API_Guid, RenumRule>` [из комментария] |
| `RenumPart` | Роль части правила в имени параметра-формулы (`Criteria` / `Delimetr`) [ReNum.hpp:223] |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `ReNumSelected` | 56 | Запуск перенумерации выбранных элементов [из комментария] — карточка |
| `RenumDG` | 131 | Диалог выбора правил; проверяет наличие правила для одного элемента [из комментария] |
| `GetRenumElements` | 186 | Сбор элементов для перенумерации [из комментария] |
| `GetFormulaTemplate` | 362 | Вырезает шаблон формулы из части правила по двойным кавычкам (#249) |
| `RenumFormulaName` | 382 | Имя параметра-формулы по роли части правила (#249) |
| `GetFormulaRawName` | 392 | Полное имя параметра-формулы: `FORMULANAMEPREFIX + имя + ';' + шаблон` (#249) |
| `AddFormulaToRead` | 405 | Кладёт `ParamValue` с `hasFormula` в `paramToRead` (#249) |
| `ReNum_GetElement` | 439 | Обработка одного элемента: применяет правила [из комментария] |
| `GetMostFrequentPos` | 692 | Наиболее частая позиция среди вариантов [из комментария] |
| `GetPos` | 720 | Позиция по критерию и разделителю [из комментария] |
| `ReNumOneRule` | 741 | Применение одного правила к набору элементов [из комментария] — карточка |
| `ElementsSeparation` | 930 | Разделение элементов правила по группам [из комментария] |
| `ReNumHasFlag` | 1053 | Проверка переключателя флага нумерации [из комментария] |
| `ReNumGetFlag` | 1080 | Состояние флага по данным параметров [из комментария] |

## Формула в критерии и разбивке (#249)

Детектор — двойные кавычки, всё между ними — шаблон (`Renum{"%Помещение%x<%Этаж%>"}`).
Непарные кавычки и пустые `""` трактуются как обычное свойство.

**Ключевые решения:**

- Шаблон вырезается **до** `ToLowerCase` правила (`ReNum_GetElement`), иначе
  литерал шаблона стал бы мелким. Имя свойства между `%…%` приводит уже
  `ReplaceProcToBrace`.
- Шаблон **не оборачивается** в `<…>`: `EvalExpression` вычисляет каждую пару
  `<…>` на месте, внешняя обёртка обрезала бы внутреннюю формулу по первой `>`.
  Так же поступает Sync для кавыченной формулы (`Sync.cpp:1774`).
- Зависимости `%имя%` собираются `ParseParamNameMaterial` — по умолчанию как
  **свойства** (`fromMaterial = true`), как в `BuildReadParamDict` (Spec.cpp).
- Имя параметра уникально в пределах правила: `renum_criteria` / `renum_delimetr`
  различаются, чтобы одинаковые шаблоны не получили один ключ в
  `ParamDictValue`.
- Проверка существования свойства для формульной части **не выполняется**;
  формульная часть не попадает и в `error_propertyname`.
- Вычисление идёт существующим путём: `ParamValue.hasFormula` → `Read` →
  `ReadFormula` → `ReplaceParamInExpression` → `EvalExpression`. `ElementsSeparation`
  не знает про формулы и получает уже вычисленную строку.
- **Частичная подстановка:** `ReplaceParamInExpression` подставляет только
  зависимости с `isValid == true`; остальные заменяются пустой строкой, а сам
  факт подстановки возвращается, когда сработала хотя бы одна. Поэтому формула с
  одной битым свойством даст результат вроде `Кx2-` вместо отказа — это
  существующее поведение движка (Sync/Spec), намеренно не менялось.
- Если формула не вычислилась или результат пуст, элемент пропускается с
  сообщением — так же, как при пустом значении критерия (отдельного кода отказа
  не вводилось).
- Ограничение: нулевое заполнение работает только для числовой позиции
  (`RenumPos::FormatToMax` выходит при `!isNum`) — решение владельца, поведение
  не менялось.


## Карточки

### `ReNumSelected(SyncSettings &syncSettings) -> GSErrCode`
- Расположение: `Sources/AddOn/ReNum.cpp:56`
- Назначение: запускает перенумерацию выбранных элементов по правилам в свойствах. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись позиций в свойства элементов** (ElementsWrite в undoable-команде, :134/:139); читает выделение. [по коду]
- Вызывает: `GetRenumElements` (:87), `GetRuleFromSelected` (Helpers.cpp:355), `GetSelectedElements` (Helpers.cpp:695), `ElementsWrite` (Helpers.cpp:4625), `SyncArray` (Sync.cpp:456). [из callgraph.json]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:427). [из callgraph.json]

### `ReNumOneRule(RenumRule &rule, ParamDictElement &paramToReadelem, ParamDictElement &paramToWriteelem, bool &has_error)`
- Расположение: `Sources/AddOn/ReNum.cpp:741`
- Назначение: применяет одно правило перенумерации к набору элементов. [из комментария]
- Контракт: не проверено.
- Побочные эффекты: **запись новых позиций** в параметры для записи (paramToWriteelem); `has_error` накапливается (FIX 2026-09-12: `has_error = has_error || …`, см. DISCREPANCIES/историю коммита 61153ca). [по коду]
- Вызывает: `ElementsSeparation`, `GetMostFrequentPos`, `GetPos`, `AddParamValue2ParamDictElement` (Helpers.cpp:1714); RenumPos::FormatToMax/SetToMax/ToParamValue. [вызовы по коду; номера строк callgraph.json устарели после #249]
- Вызывается из: `GetRenumElements` (ReNum.cpp:345). [из callgraph.json]

### `ReNum_GetElement(elemGuid, paramToRead, rules, error_propertyname, definitions) -> bool`
- Расположение: `Sources/AddOn/ReNum.cpp:439`
- Назначение: разбирает `Renum_flag{…}` и связанное `Renum{…}`, создаёт/дополняет `RenumRule`, регистрирует значения для чтения. [по коду]
- Контракт: для формульной части правила (`Renum{"…"}`) existence-проверка свойства не выполняется; непарные кавычки трактуются как обычное свойство; шаблон вырезается до `ToLowerCase`. [по коду #249]
- Побочные эффекты: пишет в `rules` и `paramToReadelem`; при отсутствующих свойствах — в `error_propertyname` (формульные части туда не попадают). [по коду]
- Вызывает: `GetFormulaTemplate`, `RenumFormulaName`, `GetFormulaRawName`, `AddFormulaToRead`, `GetParamValueFromCache`, `AddParamValue2ParamDictElement`. [по коду]
- Вызывается из: `GetRenumElements` (ReNum.cpp:249). [по коду]

## Зависимости
- `Helpers.hpp`, `third_party/alphanum.h` [по include]

## Зависимости (используется в)
- `SomeStuff_Main.cpp` [по коду]
## Версии (AC29)
- AC29 — первый SDK, который собирает аддон как **C++20** (`<LanguageStandard>stdcpp20</LanguageStandard>` в сгенерированном vcxproj; в AC26–28 — `stdcpp17`). Следствие для сравнений: у неконстантного `bool RenumPos::operator== (const RenumPos&)` компилятор добавляет «перевёрнутый» кандидат `y == x`, и `pa == pb` становится неоднозначным (MSVC C2666 в `TestFunc.cpp`). Оператор сделан `const` — поведение не меняется, в C++17 правка нейтральна. [по vcxproj AC29 + лог сборки]

