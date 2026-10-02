# Helpers — Ядро: чтение/запись параметров и свойств

> Хеш базового коммита: 309606a (2026-09-28); разделы `WriteProperty` (#223) и «Приведение double к целым» (#221). **Полное покрытие объявлений** Helpers.hpp (808 строк); строки — hpp. Номера определений .cpp не фиксированы. Карточки горячих функций — с данными callgraph.

## Назначение
Ядро add-on: чтение из свойств/GDL/IFC/атрибутов/координат/морфов в единый ParamValue и запись обратно; большинство модулей зависят от него. [по коду include-графа]

## Файлы
- `Sources/AddOn/Helpers.cpp/hpp` (hpp 808 строк; symbols.json — 2145 символов)

## Ключевые типы [из комментария / по коду, Helpers.hpp:14-63]
`SortGUID`/`SortInx` — массивы для сортировки; `OrientedSegments` — отрезки с точкой начала (hpp:26); `SkipValues` — правила игнорирования (hpp:34-46); `DimRule`/`DimRules` — правила округления размеров (hpp:48-63).

## FormatStringFunc — формат из формулы [из комментариев]
| Функция | Строка | Назначение |
|---|---|---|
| `GetFormatStringFromFormula` | 68 | Разбор формата из формулы |
| `GetFormatString` | 76 | Обработка нулей/единиц в имени свойства; удаляет единицы измерения, возвращает строку для NumToString |
| `IsValid` | 78 | Проверка строки формата |
| `ParseFormatString` | 83 | Извлечение единиц измерения и округления из строки |
| `NumToString` | 88 | Число → строка по формату |
| `ReplaceMeters` | 90 | Замена в строке формата |

## Топ-уровневые функции [из комментариев]
| Функция | Строка | Назначение |
|---|---|---|
| `GetRuleFromSelected` (2 перегрузки) | 101, 118 | Правила из выбранных элементов по имени (+скобки); перегрузка — один элемент |
| `GetElementForPropertyDefinition` | 110 | Элементы, в которых видимо свойство |
| `AttachObserver` | 127 | Подключение отслеживания изменений (по версиям AC) |
| `CheckElementType` | 133 | Тип элемента в активных настройках синхронизации |
| `IsElementEditable` (3 перегрузки) | 140, 142, 150 | Доступность элемента для редактирования (модуль/блокировка/резерв); вариант с возвратом типа |
| `GetSelectedElements` (4 перегрузки) | 160, 164, 172, 177 | GUID выбранных (+подэлементы/зоны/связанные; с/без чтения настроек) |
| `GetParentGUIDSectElem` | 188 | GUID родителя секционного элемента |
| `CallOnSelectedElemSettings` | 194 | Вызов функции (Guid, SyncSettings) для выбранных |
| `GetRelationsElement` (3 перегрузки) | 201, 206, 215 | Связанные элементы (+зоны/связи; по типу) |
| `CoordNorthAngle` | 229 | Ориентация по углу поворота и северу (RUS+ENG текст) |
| `CoordRotAngle` | 236 | Угол поворота по началу/концу |
| `CoordCorrectAngle` | 241 | Дробная часть угла с точностью |
| `GetPropertyENGName` | 250 | Английское имя свойства по локализованному |
| `GetElemState` / `GetElemStateReverse` | 795, 801 | Свойство-флаг в описании: нужно ли обрабатывать элемент |
| `hasLibData` | 806 | Наличие GDL-компонент в описании |
| `test` | 222 | [назначение не установлено — нет комментария; тестовый стублика по имени] |

## PropertyHelpers [по коду, hpp:252-257]
`ToString` ×4 — API_Variant/API_Property (± FormatString) → строка. Для AC25+ перечисления читаются без копирования списка допустимых значений; множественный выбор сохраняет порядок определений при поиске выбранных GUID. [по коду Helpers.cpp]

## ParamHelpers — чтение/запись источников [из комментариев]

### Источники чтения
| Функция | Строка | Назначение |
|---|---|---|
| `Read` (2 перегрузки) | 520, 536 | Заполнение словаря параметров элемента (± состав конструкции/ListData) |
| `ElementsRead` (2 перегрузки) | 518, 527 | То же для множества элементов |
| `ReadProperty` | 437 | Чтение значений свойств в ParamDictValue |
| `ReadIFC` | 445 | IFC-свойства (до AC29) |
| `ReadClassification` | 450 | Данные классификации |
| `ReadAttributeValues` | 455 | Атрибуты элемента |
| `ReadID` | 460 | ID элемента |
| `ReadGDL` | 465 | GDL-параметр по имени или описанию |
| `ReadMorphParam` | 273 | Морф: координаты → словарь |
| `ReadCoords` | 293 | Координаты (symb_pos_*; панель CW — центр, колонна/объект — центр+отм., зона — без отм.) |
| `ReadMaterial` | 693 | Материалы и состав конструкции |
| `ReadFormula` | 665 | Свойства с формулами |
| `ReadListData` | 667 | GDL COMPONENT-данные |
| `ReadQuantities` | 672 | Количества |
| `ReadElementValues` | 674 | [назначение не установлено — по имени: значения элемента] |
| `ReadFile` | 679 | Поиск значений в файлах: индекс строк по файлу и первой колонке поиска создаётся на время вызова; совпадения проверяются в исходном порядке. [по коду Helpers.cpp] |
| `Components` | 739 | «Вытаскивает всё из состава элемента» |
| `ComponentsBasicStructure` | 700 | Однородная конструкция |
| `ComponentsCompositeStructure` | 717 | Многослойная конструкция |
| `ComponentsProfileStructure` | 728 | Сложный профиль (AC24+): повторный `GetAttributeValues` пропускается после успешного чтения того же индекса материала в рамках профиля. [по коду Helpers.cpp] |
| `ComponentsGetUnic` | 712 | Уникальные слои |
| `GetAttributeValues` | 748 | Данные одного слоя (определение — Helpers.cpp:9577); для чтения определений свойств материала допускает только параметры с `fromAttribDefinition` (рабочее дерево #209, open). Расширение на произвольный `fromPropertyDefinition` отменено. [по коду] |
| `ReadMaterial_ReadAddParam` | 685 | Доп. параметры материалов |
| `SubGuid_GetDefinition` / `SubGuid_GetParamValue` | 630, 636 | Описания/значения с GUID родителя |
| `GDLParamByDescription` / `GDLParamByName` | 649, 657 | Поиск GDL-параметра (только чтение / чтение+запись) |

### Запись
| Функция | Строка | Назначение |
|---|---|---|
| `ElementsWrite` | 473 | Запись словаря параметров для множества элементов. На время записи создаёт `SuspendGroupsGuard` — запись в сгруппированный элемент не проходит (ACAPI_Element_ChangeMemo → APIERR_BADPARS), см. #222 |
| `Write` | 478 | Запись ParamDictValue в один элемент |
| `WriteInfo` | 483 | Запись в информацию о проекте |
| `WriteClassification` / `WriteID` / `WriteAttribute` / `WriteCoord` / `WriteGDL` / `WriteProperty` | 488-513 | Запись в классификацию / ID / атрибуты / координаты / GDL / свойства |

`WriteProperty` (`Helpers.cpp:5326`): если свойство ещё не загружено, уже известное определение добавляется в общую выборку `ACAPI_Element_GetPropertyValues`; поиск определения по имени нужен только при пустом GUID. Оба пути после загрузки проходят через существующий пакетный `ACAPI_Element_SetProperties`. [по коду Helpers.cpp:5347-5412; read-back AC25 #223]

### Преобразования ParamValue [из комментариев]
`NameToRawName` (267) — имя → rawname (скобки); `GetRawnamePrefixByTypeInx`/`GetTypeInxByRawnamePrefix` (275/277) — префикс источника; `SetParamValueSourseByName` (282) — источник по rawName; `SetArrayByRawname` (284); `ReplaceParamInExpression` (299) — подстановка значений; `GetParamValueForElements` (301); `ReplaceProcToBrace` (306); `ParseParamNameMaterial` (311) — имена в %% ; `ParseParamName` (316) — имена в {}; `AddValueToParamDictValue` (321); `needAdd` (328); `AddParamValue2ParamDict` (333) и `AddParamValue2ParamDictElement` ×2 (339/347); `CheckIgnoreVal` (352); `CompareParamValue` (357); `AddParamDictValue2ParamDictElement` (367); `AddProperty` (374); `AddBool/Length/Double/StringValueToParamDictValue` (379-414); `CompareParamDictValue` ×2 (427/432); `CompareParamDictElement` (643); `Array2ParamValue` (543); конвертации `ConvertToParamValue` — GDL (:544/548/568), свойство (:578), определение (:585), IFC (:606), строка (:590), int (:595), double (:600); `ConvertBoolToParamValue` (558); `ConvertAttributeToParamValue` (563); `SetrawNameFromProperty` (573); `ConvertToParamValue_CheckAttrib` (580); `ConvertByFormatString` (609); `GetUnitsPrefix` (681); `SetUnitsAndQty2ParamValueComposite` (683); `ToString` ×4 — ParamValue (753/758), API_Variant/Property (253-256); `isEng`-независимые единицы.

### Прочее
`ACAPI_Attribute_GetAttributesByType` (612, inline, до AC27) — обёртка GetNum+Get.

### Приведение double к целым — #221
Все приведения `double` → `Int32` из данных проекта идут через `CommonFunction::DoubleToInt32` / `DoubleToInt32RoundUp` (насыщение на границах диапазона + сообщение через `msg_rep`): `ReadID`, `NumToString`, `ConvertToParamValue` (GDL/строка/угол), `ConvertToParamValue(API_Property)`, `ConvertStringToParamValue`, `ConvertDoubleToParamValue`, `ConvertToParamValue(API_IFCProperty)`, `ConvertByFormatString`. [по коду, Helpers.cpp]

Не охвачены и остаются прямыми кастами там, где диапазон гарантирован кодом выше, а не данными: нормализованный угол направления (`CoordNorthAngle`, `dir %= 8` сразу после приведения) и индексы `API_AttributeIndex` (`ComponentsCompositeStructure`, `GetAttributeValues`). [по коду Helpers.cpp]

## Операторы сравнения [по коду, hpp:761-789]
`operator+` (ParamValueData), `operator==`/`!=` (ParamValue), `operator==` (API_Variant, SingleVariant, ListVariant, SingleEnumerationVariant, MultipleEnumerationVariant <AC25, PropertyGroup, PropertyDefinition, Property), `Equals` (PropertyDefaultValue, PropertyValue).

## Зависимости
- `ClassificationFunction.hpp`, `CommonFunction.hpp`, `dialogs/SyncSettings.hpp`, `spec/Spec_libpart.hpp`, `StringConversion.hpp` [по include]

## Зависимые модули
Sync, Summ, Spec, ReNum, Dimensions, Roombook, MEPv1, Propertycache, pk/*, dialogs/* [по include]

## Инварианты и подводные камни
- `ParamValueComposite::structype` несёт **тип слоя**, а не флаги SDK: только `0`,
  `APICWallComp_Core` или `APICWallComp_Finish`. Сырые флаги (`API_CompositeQuantity.flags`,
  `API_CWallComponent.flagBits`) нормализует `LayerStructype (short flags)` — вызывать при
  каждом чтении флагов. Маска `~0x0F` неприменима: `APICWallComp_Core == APICWall_ForSlab
  == 0x02`. Проверять `Core` раньше `Finish`, как в примерах SDK. Сентинел `-1`
  (`Roombook.cpp`, «слой не ядро») с этим набором не пересекается [проверено, #248]
- `ReadQuantities`: для `API_ProfileStructure` длинный путь — **основной**, не запасной.
  Профиль задаёт порядок слоёв расстоянием от начала (`rfromstart`), а состав приходит в
  порядке компонентов ArchiCAD, поэтому сопоставление по номеру слоя неприменимо;
  признак — `longWayIsMain`, сообщение «Old method» для профиля не печатается [проверено, #248]
- `ReadQuantities`: длинный путь, как и короткий, читает только композиты со всеми слоями
  (`composite_pen <= 0`) и переносит `unit`/`kzap` из уже найденного `qtyPtr` перед
  `SetUnitsAndQty2ParamValueComposite`. Без переноса `kzap` количество считалось с
  `kzap = 1` — на кирпиче с запасом 2,5 это занижало объём в 2,5 раза
  [проверено на модели, #248]
- `ReadQuantities` длинный путь: `break` после первого непустого композита — проектное
  решение, не дефект; при нескольких заполнениях с `composite_pen < 0` остальные молча
  игнорируются [решение владельца 2026-10-02, не чинить]
- `ConvertToProperty` — TODO «Переписать всё под запись ParamValue» [из комментария, hpp:419]
- Мемо: все 7 объявлений `API_ElementMemo` в Helpers.cpp — `= {}` (проверено, DISCREPANCIES #7) [проверено]
- Дубликат `CompareParamDictValue` (hpp:427 и 432 — два одинаковых объявления) [по коду — кандидат в ревью]
