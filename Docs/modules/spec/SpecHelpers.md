# SpecHelpers (шаг сетки размещения, дамп значений, общий builder записи)

> Хеш состояния: `2101967` (2026-10-02), строки — определения в
> `SpecHelpers.cpp` (1-based, собраны clangd и сверены с исходником).
> Обновлено 2026-10-02: добавлена пятая функция (`BuildRowParamToWrite`, R8.2)
> и карта вызовов.

Внутренний модуль движка спецификаций. Собран из помеченных в `Spec.cpp` как
`TODO : вынести в SpecHelpers` функций — шаг сетки при размещении и построение
дампа значений элемента.

| Что | Где | Назначение |
|---|---|---|
| `GetSizePlaceElement` | `SpecHelpers.cpp:29` | шаг сетки `dx`/`dy` по GDL-параметрам memo |
| `ParamValueToDumpString` | `SpecHelpers.cpp:96` | значение параметра → строка дампа |
| `FillDumpFromParamDict` | `SpecHelpers.cpp:118` | дамп из словаря записываемых параметров |
| `FillDumpGDLParameter` | `SpecHelpers.cpp:138` | дамп одного GDL-параметра из `API_AddParType` |
| `BuildRowParamToWrite` | `SpecHelpers.cpp:162` | общий сбор `param` строки: GUID-связь, носитель правила, выходные слоты, суммы (R8.2) |

## Почему отдельный модуль

Первые четыре функции были в конце `Spec.cpp`, три из них — `static`
с прямыми декларациями в начале того же файла. Дамп собирается только при
`runResult->includeDetails`, шаг сетки — только при размещении элементов, то
есть оба потребителя вызывают их из одной точки (`PlaceElements`), а тестам
`TestSpecSizes` нужен прямой доступ к `GetSizePlaceElement` без модели. Отдельный
заголовок убирает `static` и даёт тестам тот же контракт, что у прода.

Пятая функция (`BuildRowParamToWrite`, R8.2) добавлена позже и по другой
причине: два блока сбора `param` — для изменяемых и для создаваемых объектов —
были идентичны, и сбор вынесен в одну функцию с двумя вызовами. [по коду]

Тела первых четырёх перенесены **дословно** — проверено сравнением с `HEAD`: все
четыре идентичны посимвольно (снят только `static` в сигнатуре), мультимножество
идентификаторов не изменилось, число вызовов `ACAPI_*` — 0 в обоих.

## Карта вызовов

Собрана `prepareCallHierarchy` по каждой функции, строки 1-based. Пять функций,
девять входящих рёбер.

| Функция | Вызывается из | Вызывает (проектное) |
|---|---|---|
| `GetSizePlaceElement` :29 | `PlaceElements` (SpecExecutor.cpp:91); `TestSpecSizes` (TestSpec.cpp:6183, 6201, 6204, 6209, 6218, 6224) | `DoubleToInt32` (CommonFunction.cpp:955) |
| `ParamValueToDumpString` :96 | `FillDumpFromParamDict` (:129) | `NumToString` (Helpers.cpp:296) |
| `FillDumpFromParamDict` :118 | `PlaceElements` (SpecExecutor.cpp:108); `SpecArray` (Spec.cpp:1028) | `ParamValueToDumpString` (:129) |
| `FillDumpGDLParameter` :138 | `PlaceElements` (SpecExecutor.cpp:158) | — |
| `BuildRowParamToWrite` :162 | `PlaceElements` (SpecExecutor.cpp:94); `SpecArray` (Spec.cpp:1019); `TestSpecBuildRowParam` (TestSpec.cpp:593, 615, 632, 646, 660, 675) | `StringUnic` (CommonFunction.cpp:1508) |

### `Spec::BuildRowParamToWrite(const Element &el, const ParamDictValue &paramToWrite, ParamDictValue &param)`
- Расположение: `SpecHelpers.cpp:162`
- Назначение: единственное место, где собирается `param` для записи в модель —
  и для изменяемых объектов, и для создаваемых. [по коду]
- Контракт: порядок записи — сначала GUID-связь (`subguid_paramrawname`),
  затем носитель правила (`subguid_rulename`/`subguid_rulevalue`), затем
  выходные слоты, затем суммарные; тип суммы дополнительно приводится по
  `fromPropertyDefinition`, тип выхода не подменяется. Безымянный слот
  игнорируется, носитель правила не пишется при пустом значении. [по коду,
  закрыто набором `TestSpecBuildRowParam` 17/17]
- Побочные эффекты: нет — только заполнение переданного словаря `param`.
- Вызывается из: `PlaceElements` (SpecExecutor.cpp:94) для создаваемых строк,
  `SpecArray` (Spec.cpp:1019) для изменяемых.

## Границы

- `GetSizePlaceElement` **не обращается к модели**: читает только уже загруженный
  `memot.params`. [по коду]
- `dx`/`dy` заполняются только в ветках, где признак размещения определён;
  при `memot.params == nullptr` возвращается `false`, а параметры остаются
  нетронутыми. [по коду]
- Ветка `show_type` проверяется на 1 и на 2/3, но при другом значении (в том
  числе 0) **не выходит** — дальше работает ветка `A`/`B`. Набор
  `TestSpecSizes` закрепляет все пять значений `show_type` 0–4. [по коду]
- `ParamValueToDumpString` повторяет формат `ParamHelpers::ToString`, но без
  `DBBREAK` в ветке неизвестного типа: дамп — диагностический вывод и не должен
  прерывать построение. [из комментария]
- `FillDumpFromParamDict` исключает GDL-имена (`{@gdl:`), поэтому
  `modified.gdlParameter` всегда пуст — в отличие от `created`. [по коду]
- `FillDumpGDLParameter` берёт значение из `API_AddParType` в момент записи в
  memo, а не из `ParamValue`: приведение к типу параметра может изменить
  значение. [из комментария]

## Зависимости
- `Spec.hpp` [по include]

## Зависимости (используется в)
- `spec/SpecExecutor.cpp` (`PlaceElements`) [по коду]
- `spec/Spec.cpp` (ветка `elements_mod` в `SpecArray`, :1028) [по коду]
- `tests/TestSpec.cpp` (`TestSpecSizes`, `TestSpecBuildRowParam`) [по коду]