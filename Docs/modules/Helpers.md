# Helpers — Ядро: чтение/запись параметров и свойств

> Хеш коммита: 1e67983 (2026-09-22). **Полное покрытие объявлений** Helpers.hpp (808 строк); строки — hpp. Номера определений .cpp не фиксированы. Карточки горячих функций — с данными callgraph.

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
`ToString` ×4 — API_Variant/API_Property (± FormatString) → строка.

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
| `ReadFile` | 679 | Поиск значений в файлах |
| `Components` | 739 | «Вытаскивает всё из состава элемента» |
| `ComponentsBasicStructure` | 700 | Однородная конструкция |
| `ComponentsCompositeStructure` | 717 | Многослойная конструкция |
| `ComponentsProfileStructure` | 728 | Сложный профиль (AC24+) |
| `ComponentsGetUnic` | 712 | Уникальные слои |
| `GetAttributeValues` | 748 | Данные одного слоя (определение — Helpers.cpp:9577); для чтения определений свойств материала допускает только параметры с `fromAttribDefinition` (рабочее дерево #209, open). Расширение на произвольный `fromPropertyDefinition` отменено. [по коду] |
| `ReadMaterial_ReadAddParam` | 685 | Доп. параметры материалов |
| `SubGuid_GetDefinition` / `SubGuid_GetParamValue` | 630, 636 | Описания/значения с GUID родителя |
| `GDLParamByDescription` / `GDLParamByName` | 649, 657 | Поиск GDL-параметра (только чтение / чтение+запись) |

### Запись
| Функция | Строка | Назначение |
|---|---|---|
| `ElementsWrite` | 473 | Запись словаря параметров для множества элементов |
| `Write` | 478 | Запись ParamDictValue в один элемент |
| `WriteInfo` | 483 | Запись в информацию о проекте |
| `WriteClassification` / `WriteID` / `WriteAttribute` / `WriteCoord` / `WriteGDL` / `WriteProperty` | 488-513 | Запись в классификацию / ID / атрибуты / координаты / GDL / свойства |

### Преобразования ParamValue [из комментариев]
`NameToRawName` (267) — имя → rawname (скобки); `GetRawnamePrefixByTypeInx`/`GetTypeInxByRawnamePrefix` (275/277) — префикс источника; `SetParamValueSourseByName` (282) — источник по rawName; `SetArrayByRawname` (284); `ReplaceParamInExpression` (299) — подстановка значений; `GetParamValueForElements` (301); `ReplaceProcToBrace` (306); `ParseParamNameMaterial` (311) — имена в %% ; `ParseParamName` (316) — имена в {}; `AddValueToParamDictValue` (321); `needAdd` (328); `AddParamValue2ParamDict` (333) и `AddParamValue2ParamDictElement` ×2 (339/347); `CheckIgnoreVal` (352); `CompareParamValue` (357); `AddParamDictValue2ParamDictElement` (367); `AddProperty` (374); `AddBool/Length/Double/StringValueToParamDictValue` (379-414); `CompareParamDictValue` ×2 (427/432); `CompareParamDictElement` (643); `Array2ParamValue` (543); конвертации `ConvertToParamValue` — GDL (:544/548/568), свойство (:578), определение (:585), IFC (:606), строка (:590), int (:595), double (:600); `ConvertBoolToParamValue` (558); `ConvertAttributeToParamValue` (563); `SetrawNameFromProperty` (573); `ConvertToParamValue_CheckAttrib` (580); `ConvertByFormatString` (609); `GetUnitsPrefix` (681); `SetUnitsAndQty2ParamValueComposite` (683); `ToString` ×4 — ParamValue (753/758), API_Variant/Property (253-256); `isEng`-независимые единицы.

### Прочее
`ACAPI_Attribute_GetAttributesByType` (612, inline, до AC27) — обёртка GetNum+Get.

## Операторы сравнения [по коду, hpp:761-789]
`operator+` (ParamValueData), `operator==`/`!=` (ParamValue), `operator==` (API_Variant, SingleVariant, ListVariant, SingleEnumerationVariant, MultipleEnumerationVariant <AC25, PropertyGroup, PropertyDefinition, Property), `Equals` (PropertyDefaultValue, PropertyValue).

## Зависимости
- `ClassificationFunction.hpp`, `CommonFunction.hpp`, `dialogs/SyncSettings.hpp`, `spec/Spec_libpart.hpp`, `StringConversion.hpp` [по include]

## Зависимые модули
Sync, Summ, Spec, ReNum, Dimensions, Roombook, MEPv1, Propertycache, pk/*, dialogs/* [по include]

## Инварианты и подводные камни
- `ConvertToProperty` — TODO «Переписать всё под запись ParamValue» [из комментария, hpp:419]
- Мемо: все 7 объявлений `API_ElementMemo` в Helpers.cpp — `= {}` (проверено, DISCREPANCIES #7) [проверено]
- Дубликат `CompareParamDictValue` (hpp:427 и 432 — два одинаковых объявления) [по коду — кандидат в ревью]
