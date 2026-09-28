# spec/Spec — Движок спецификаций

> Хеш состояния: рабочее дерево ветки `spec_refactor` поверх `e22ddc8` (2026-09-28) — #227: дамп значений элементов (`includeParameters`). Последний закоммиченный хеш: `309606a`, раздел `GetParamValue` дополнен правкой #221. Номера строк — определения в `.cpp` (1-based, проверены source; функции внутри namespace Spec).

## Назначение
Генерация спецификаций по правилам: разбор описаний, выбор элементов, группировка, создание/обновление элементов. [из комментария, Spec.hpp:8-9]

## Файлы
- `spec/Spec.cpp/hpp`
- `json_commands/SpecCommand.cpp/hpp` — JSON-точка входа AC25–29 [по коду]

## Ключевые типы

| Тип | Описание |
|-----|----------|
| `GroupSpec` | unic_paramrawname, out_paramrawname, sum_paramrawname, flag_paramrawname, fromMaterial, fromLibData, n_layer [из комментария, Spec.hpp:13-23] |
| `SpecRule` | rule_name, groups, out_paramrawname, subguid_paramrawname (маркер), destinationParamGuidName (разрешённое свойство), subguid_rulename/rulevalue, elements, exsist_elements, rule_definitions, favorite_name, флаги политики [см. карточку ниже] |
| `Element` / `ElementDict` | Временный контейнер создаваемого элемента / словарь по сцепке уникальных параметров [из комментария] |
| `SpecRuleDict` | HashTable<string, SpecRule> [из комментария] |
| `SpecElementDump` | Дамп одного созданного/изменённого элемента: `guid`, `favorite_name`, `sourceElements`, `properties`, `gdlParameters` (#227, Spec.hpp) |

## Публичный API

| Функция | .cpp строка | Назначение |
|---------|-------------|------------|
| `SpecAll` | 150 | Создание спецификации из выбора/видимых/правил по умолчанию [из комментария] — карточка |
| `SpecFilter` | 222, 366 | Исключение неподходящих типов/БД (две перегрузки) [из комментария] |
| `GetRuleFromDefaultElem` | 42 | Правила из свойств элемента по умолчанию [из комментария] |
| `GetRuleFromElement` | 1140 | Правила из выбранного элемента [из комментария] |
| `AddRule` | 1207 | Разбор описания и добавление правила в словарь [из комментария] |
| `GetRuleFromDescription` | 1930 | Разбор строки описания в SpecRule [из комментария] |
| `GetParamValue` | 1416 | Чтение одного значения параметра [из комментария] |
| `GetElementsForRule` | 1559 | Формирование элементов для одного правила [из комментария] |
| `GetParamToReadFromRule` | 1294 | Параметры для предварительного чтения [из комментария] |
| `GetElementForPlace` | 2391 | Создание/настройка элемента для размещения [из комментария] |
| `GetSizePlaceElement` | 2436 | Размер элемента по сетке [из комментария] |
| `PlaceElements` | 2518 | Размещение сформированных элементов и заполнение параметров [из комментария] — карточка |
| `ParamValueToDumpString` / `FillDumpFromParamDict` / `FillDumpGDLParameter` | 2757, 2779, 2799 | Сборка дампа значений элемента для #227 [по коду] |

### R4.1 — нормализация описания выделена в проверяемую единицу (#228)
- `NormalizeRuleDescription (const GS::UniString &source)` — `Spec.cpp:1222`, объявление
  `Spec.hpp:147`. Прежде те же 33 замены стояли телом `AddRule` (`Spec.cpp:1271`),
  поэтому нормализация не имела собственного контракта и не тестировалась вовсе:
  все проверки парсера били по `GetRuleFromDescription` с уже нормализованной строкой.
- Закреплено в `TestSpecNormalize` (`TestFunc.cpp:532`, вызов на `:892`): 17 пар
  вход→выход плюс сквозной путь «нормализация → разбор», проверка неизменности
  исходной строки, пустая строка и строка из одних пробелов. Ожидания получены
  воспроизведением алгоритма, а не подгонкой под фактический вывод, и сверены
  с ключом `Fav;g@@u;p;f;q@@s@@x;y)`, уже закреплённым в `TestSpecAddRule`.
- **Установлено, что порядок замен критичен только между блоками, а не внутри
  них:** сперва убираются пробелы перед скобками, потом вызовы переписываются в
  маркеры. Внутри блока маркеров `g(` не входит в `gl(`/`gm(` (после `g` идёт
  `l`/`m`), поэтому перестановка `g(`/`gl(`/`gm(` результата не меняет —
  проверено перебором перестановок. Шесть проходов `"  " -> " "` схлопывают
  серию пробелов до одного для серий длиной до 64; более длинная остаётся свёрнутой
  не полностью. [по коду + перебор перестановок]
- **R4.3 (попутно, та же правка):** `GetRuleFromDescription` теперь принимает
  `const GS::UniString &` (`Spec.cpp:1963`, `Spec.hpp:156`) и правит локальную копию.
  Прежде парсер оставлял вход в обрезанном виде, и это держалось только тем, что
  вызывающий не читал строку после вызова. Тест «input consumed» заменён на
  «input intact» + «key reusable after parse». [по коду]

## Карточки

### R3 — `SpecRule`: разведение определения и состояния запуска (#228)
- Расположение: `Sources/AddOn/spec/Spec.hpp:25-68`, вызовы в `Spec.cpp`.
- Что изменилось: прежний единый `is_Valid` разнесён на три признака, каждый пишется своей стадией и не затирает остальные:
  - `parseValid` — описание правила разобрано (`GetRuleFromDescription`, `Spec.cpp:1986/1994/2032/2061/2264/2266/2268`); неизменно после разбора;
  - `selected` — правило выбрано: диалогом `SpecDG` (`:504`) или списком `ruleNames` (`:616`);
  - `destinationReady` — у избранного есть все выходные свойства и суммы (`:683`, `:690`).
- Совместимый адаптер `IsRunnableForRun ()` = `parseValid && selected && destinationReady` заменяет прежние чтения `is_Valid` в `SpecArray` (`:632`, `:830`, `:1040`). Внешнее поведение не изменилось: `SpecDG` по-прежнему вызывается на `:815`, уже после сверки с избранного, поэтому пользователю показывается меньше правил, чем в словаре — это свойство сохранено, а не исправлено. [по коду; A/B — см. ниже]
- `GroupSpec::is_Valid` НЕ переименован: это отдельный флаг с другим смыслом (`Spec.cpp:1308/1587/2229/2236/2242`). Одноимённость сохранена намеренно — переименование в этом шаге не требовалось контрактом. [по коду]
- **R3.3 — маркер и разрешённое свойство разведены.** Прежде `subguid_paramrawname` хранил и маркер из описания правила (`:1260`), и найденное имя свойства избранного (`:739` — перезапись), а `description.Contains (rule.subguid_paramrawname...)` на `:727` и сверка существующих объектов на `:1876` читали поле ПОСЛЕ перезаписи. Теперь маркер неизменяем, найденное свойство пишется в `destinationParamGuidName` (`:739`, `:783`, `:1710`, `:1876`). Инвариант «маркер == найденное имя» больше не подразумевается; раньше он держался только тем, что словарь правил создаётся заново на каждый запуск. [по коду]
- Побочные эффекты: не менялись — те же запросы к модели в том же порядке, тот же момент диалога, те же границы undo. [по коду]
- Проверка: clang-format, clangd 0 по `Spec.cpp`/`Spec.hpp`/`TestFunc.cpp` (ошибки в `TestFunc.cpp:877+` предсуществующие: clangd считает `-Wunused-` ошибкой, MSVC прощает), AC25 Debug — `Build succeeded!`, runner exit 0.
- **Тесты (AC25, реальный прогон, панель VS «Отладка»):** `SpecRegression values / read plan / grouping / reconcile / parser / add rule / sizes` — все `end`-маркеры на месте, `ERROR IN TEST` одна: `ConvertToParamValue(Property) : doubleValue (отрицательное)` — предсуществующая, из прошлой задачи.
- **A/B против эталона P0:** 3 прогона `SomeStuffCommand.Spec` на свежей модели (рестарт ArchiCAD) — каждый `completed`, C=2 / M=12 / D=2, 14 строк, `diff_rows` против `compare-p0-smoke.json` дал **0 расхождений** по значениям, gdl и числу источников. Первое же наблюдение дало регрессию в тесте (`Spec link copied`): тест сравнивал `Element::subguid_paramrawname` с маркером правила, что после R3.3 неверно по замыслу — фикстура разделена на `subguid_paramrawname` (маркер) и `destinationParamGuidName` (разрешённое), проверка переведена на второе.
- Не покрыто `not verified`: create-from-scratch, update, delete_old и сохранность значений в конечной модели (независимого read-back нет). A/B выполнен на повторяемом сценарии «свежая модель из рестарта», который воспроизводит 2/12/2. [по коду]

### `Spec::GetParamValue(...) -> bool` (#220, локальная правка)
- Расположение: `Sources/AddOn/spec/Spec.cpp:1389`.
- Контракт: читает обычное значение через `GetParamValueForElements`; материал выбирается по `pvalue.fromMaterial`, а не по аргументу `fromMaterial` (аргумент сохранён для совместимости). Для материалов и list-data отрицательный `n_layer` возвращает `false`. При отказе `pvalue.isValid == false`; остальные поля при отказе не являются результатом. [по коду]
- Отсутствующий элемент/ключ и пустой состав — ошибка. Положительный индекс за концом непустого состава — успешная пустая строка с нулевыми числовыми полями, `boolValue=false`, `canCalculate=false`, `isValid=true`. Для отсутствующих list-data или невычисленной формулы сохранён такой же успешный пустой результат. [по коду]
- Доступ к вложенным словарям — `GetPtr` с проверкой `nullptr`; результат формулы ищется после изменения локального словаря, указатель через его заполнение не удерживается. Входные словари не изменяются. [по коду]
- С 2026-09-28 (#221) `pvalue.val.intValue` для материала слоя заполняется через `CommonFunction::DoubleToInt32 (Spec.cpp:1472)`: значение вне диапазона Int32 (включая NaN) заменяется границей диапазона с сообщением `msg_rep`, тип поля и строковое значение не меняются. [по коду Spec.cpp:1466-1479]
- Вызывает: `hasLibData`, `ParamHelpers::ParseParamName`, `ListData::AddLibdataToParamValueDict`, `ParamHelpers::ReadFormula`, `ParamHelpers::GetParamValueForElements`, `UniStringToDouble`, `is_equal`, `DoubleToInt32`, диагностические `DBprnt`/`msg_rep`. Вызывается из `GetElementsForRule` и `TestFunc::TestSpecGetParamValue`. [по исходникам]
- Проверка: целевой runtime-набор AC25 в `TestFunc.cpp`; полный набор тестов и другие AC-версии этой проверкой не покрываются.

### `Spec::SpecAll(const SyncSettings &syncSettings, const GS::Array<GS::UniString> *ruleNames = nullptr, const Point2D *placementPoint = nullptr, SpecRunResult *runResult = nullptr) -> GSErrCode`
- Расположение: `Sources/AddOn/spec/Spec.cpp:140`
- Назначение: создаёт спецификацию из текущего выбора, всех видимых элементов или правил по умолчанию. [из комментария]
- Контракт: если выделение пусто и `GetRuleFromDefaultElem` обнаружил включённые элементы (`has_elementspec`), передаёт заполненные `rules` в `SpecArray` даже при пустом `guidArray`; возвращает `NoError` только когда нет ни выделения, ни включённых элементов default-правила. Для non-interactive запуска `placementPoint` задаёт начальную точку без диалога и окна прогресса; при `ruleNames == nullptr` обрабатываются все валидные правила. `runResult->elementsToCreate` после размещения равен приросту `paramOut` (число реально созданных элементов, без уже запланированных изменений); остальные счётчики отражают сформированные списки. [по коду; AC25 Debug #207 и runtime #208/#209]
- Если требуемое значение свойства строительного материала не прочитано, строка не формируется: пустое наименование не подставляется вместо исходных данных. `ParamHelpers::GetAttributeValues` для отсутствующего у исходного элемента свойства с `fromPropertyDefinition` пробует получить значение у строительного материала. [по коду; AC25 runtime #209]
- Побочные эффекты: **создаёт/обновляет/удаляет элементы спецификации** (PlaceElements-цепочка); читает выделение и свойства. [по коду]
- Вызывает: `GetRuleFromDefaultElem` (:185), `SpecFilter` (:177/:196), `SpecArray` (:204), `GetSelectedElements` (Helpers.cpp:695). [по коду]
- Вызывается из: `MenuCommandHandler` (SomeStuff_Main.cpp:441) и `SomeStuffCommand.Spec` (`json_commands/SpecCommand.cpp`; JSON API AC25–29). Menu-path сохраняет `SpecDG` и `ClickAPoint`; JSON-команда требует `placementPoint {x, y}`, необязательно принимает `ruleNames` и `includeParameters` (default false, #227) и возвращает `status`, `resultCode`, `elementsToCreate`, `elementsToModify`, `elementsToDelete`, `elapsedSeconds`, `includeParameters`; при `includeParameters=true` дополнительно `created`, `modified`, `deleted`. AC25 runtime: после вызова с точкой число объектов выросло с 548 до 582, в последних 34 объектах свойство «Спецификации материалов/Наименование в объект» заполнено. [по коду и AC25 runtime #208/#209]

### `Spec::PlaceElements(GS::Array<ElementDict> &elementstocreate, ParamDictValue &paramToWrite, ParamDictElement &paramOut, Point2D &startpos, SpecRunResult *runResult = nullptr) -> GSErrCode` (#227)
- Расположение: `Sources/AddOn/spec/Spec.cpp:2518`
- Назначение: размещает сформированные элементы в модели и заполняет их параметры. [из комментария]
- Контракт: **создание** и **запись/удаление** находятся в разных undo-вызовах; возврат `PlaceElements` на вызывающей стороне не проверяется (`Spec.cpp:1060`), а сама функция заканчивается `NoError`. Изменение этой политики ошибок — отдельный вопрос F2, не входит в #227. [по коду]
- #227: при `runResult != nullptr && runResult->includeDetails` заполняет `runResult->created`. Дамп собирается ДО `ACAPI_Element_Create` (Spec.cpp:2631–2642), потому что GDL-параметры после записи в memo удаляются из `param` (`param.Delete (rawname)`, :2690) и в `paramOut` их уже нет. GUID проставляется в дамп только после успешного создания (:2709). `favorite_name`/`sourceElements` копируются только под флагом — выключенный дамп не платит за копирование. [по коду]
- GDL-параметры в дампе берутся из фактического `API_AddParType` в момент записи в memo, а не из исходного `ParamValue`: приведение к типу параметра может изменить значение. [по коду]
- Побочные эффекты: **создание элементов в проекте** (из избранного `favorite_name`), запись параметров/GUID (`subguid`); изменение сетки размещения (startpos). [по коду]
- Вызывает: `GetElementForPlace` (:2461), `UnhideUnlockElementLayer` (CommonFunction.cpp:2472), `StringUnic` (CommonFunction.cpp:1473); создание элементов — `ACAPI_Element_Create` в `ACAPI_CallUndoableCommand` (:2449). [из callgraph.json]
- Вызывается из: `SpecArray` (Spec.cpp:1060). [по коду]

### #227 — дамп значений элементов в JSON-ответе
- Необязательный параметр `includeParameters` (bool, default false). При true ответ содержит `created` / `modified` / `deleted`; каждый элемент — `guid`, `favoriteName`, `sourceElement[]`, `property[]`, `gdlParameter[]`. Списки отсортированы по имени (`GS::Array` не имеет `Sort` ни в AC25, ни в AC29 -> `std::vector` + `std::sort`). [по коду `SpecCommand.cpp:60-133`]
- `created` собирается в `PlaceElements`, `modified` — в `SpecArray` (ветка `elements_mod`, там GUID известен), `deleted` — список GUID из `elements_delete` (`Spec.cpp:852`). Все три заполняются ДО применения операций: непустой массив не доказывает, что запись/удаление состоялись. [по коду]
- Грабли: `*runResult = {}` в `SpecAll:157` и `SpecArray:547` стирал `includeDetails`, заданный вызывающим. Исправлено сохранением флага перед сбросом; найдено прогоном (счётчик показывал 16 при `created=0`). [по коду]
- `modified` заполняется только через `FillDumpFromParamDict`, который исключает GDL-имена, поэтому `modified.gdlParameter` всегда пуст — в отличие от `created`. Одинаковая форма JSON не означает одинаковую полноту данных. [по коду]
- Значения сериализуются строками в формате записи в модель; GDL-числа через `%g`. Равенство текстов не доказывает равенство типов и полной числовой точности. [по коду]
- Стенд A/B: `Tools/spec_baseline.py capture|compare <tag>`, каталог `Reviews/spec_baseline/` (gitignored). Сравнение по семантическому ключу строки (favorite + значения без GUID-полей и «имени правила»), новые GUID не сравниваются. Граница: эталон подготовленной записи, а не снимок конечной модели. [по коду]

## Зависимости
- `Helpers.hpp`, `Propertycache.hpp`, `CommonFunction.hpp` [по include]

## Зависимости (используется в)
- `SomeStuff_Main.cpp` [по коду]

## Инварианты
- `stop_on_error = true` — остановка обработки правила при ошибке; `only_visible = true` — только видимые [из комментария, Spec.hpp:43-44]
- `isKM`/`isKZH` — правила для КМ/КЖ [из комментария, Spec.hpp:45-46]
