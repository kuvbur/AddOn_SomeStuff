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
| `Read` (2 перегрузки) | 531, 547 | Заполнение словаря параметров элемента (± состав конструкции/ListData) |
| `ElementsRead` (2 перегрузки) | 529, 538 | То же для множества элементов |
| `ReadProperty` | 448 | Чтение значений свойств в ParamDictValue |
| `ReadIFC` | 456 | IFC-свойства (до AC29) |
| `ReadClassification` | 461 | Данные классификации |
| `ReadAttributeValues` | 466 | Слой из заголовка элемента; для Wall/Slab/Roof/Shell считывает активный состав/стройматериал и тип Basic/Composite. Профиль и колонны/балки не обрабатываются (#235). [по коду] |
| `ReadID` | 471 | ID элемента |
| `ReadGDL` | 476 | GDL-параметр по имени или описанию |
| `ReadMorphParam` | 284 | Морф: координаты → словарь |
| `ReadCoords` | 304 | Координаты (symb_pos_*; панель CW — центр, колонна/объект — центр+отм., зона — без отм.) |
| `ReadMaterial` | 704 | Материалы и состав конструкции |
| `ReadFormula` | 676 | Свойства с формулами |
| `ReadListData` | 678 | GDL COMPONENT-данные |
| `ReadQuantities` | 683 | Количества |
| `ReadElementValues` | 685 | [назначение не установлено — по имени: значения элемента] |
| `ReadFile` | 690 | Поиск значений в файлах: индекс строк по файлу и первой колонке поиска создаётся на время вызова; совпадения проверяются в исходном порядке. [по коду Helpers.cpp] |
| `Components` | 750 | «Вытаскивает всё из состава элемента» |
| `ComponentsBasicStructure` | 711 | Однородная конструкция |
| `ComponentsCompositeStructure` | 728 | Многослойная конструкция |
| `ComponentsProfileStructure` | 739 | Сложный профиль (AC24+): повторный `GetAttributeValues` пропускается после успешного чтения того же индекса материала в рамках профиля. [по коду Helpers.cpp] |
| `ComponentsGetUnic` | 723 | Уникальные слои |
| `GetAttributeValues` | 759 | Данные одного слоя (определение — Helpers.cpp:9753); для чтения определений свойств материала допускает только параметры с `fromAttribDefinition` (рабочее дерево #209, open). Расширение на произвольный `fromPropertyDefinition` отменено. [по коду] |
| `ReadMaterial_ReadAddParam` | 696 | Доп. параметры материалов |
| `SubGuid_GetDefinition` / `SubGuid_GetParamValue` | 641, 647 | Описания/значения с GUID родителя |
| `GDLParamByDescription` / `GDLParamByName` | 660, 668 | Поиск GDL-параметра (только чтение / чтение+запись) |

### Запись
| Функция | Строка | Назначение |
|---|---|---|
| `ElementsWrite` | 484 | Запись словаря параметров для множества элементов. На время записи создаёт `SuspendGroupsGuard` — запись в сгруппированный элемент не проходит (ACAPI_Element_ChangeMemo → APIERR_BADPARS), см. #222 |
| `Write` | 489 | Запись ParamDictValue в один элемент |
| `WriteInfo` | 494 | Запись в информацию о проекте |
| `WriteClassification` / `WriteID` / `WriteAttribute` / `WriteCoord` / `WriteGDL` / `WriteProperty` | 499-524 | Запись в классификацию / ID / атрибуты / координаты / GDL / свойства |

`WriteProperty` (`Helpers.cpp:5484`, тело :5484-5574): если свойство ещё не загружено, уже известное определение добавляется в общую выборку `ACAPI_Element_GetPropertyValues`; поиск определения по имени нужен только при пустом GUID. Оба пути после загрузки проходят через существующий пакетный `ACAPI_Element_SetProperties`. [по коду Helpers.cpp:5484-5574; read-back AC25 #223]

`WriteAttribute` (#235): прежняя запись слоя через `API_AttributeIndexFindByName` сохранена; чтение только Layer не запрашивает полный `API_Element`, поэтому `Read` переносит полученный `elem_head` в `element.header` перед `ReadAttributeValues` — иначе теряется GUID/слой. Для Wall/Slab/Roof/Shell разрешены только Basic ↔ Composite, не профиль. `Composite`/`BuildingMaterial` ищутся в кэше по имени (текст) либо индексу (число); выбранный индекс и пригодность состава для типа элемента проверяются через `ACAPI_Attribute_Get` и `APICWall_For*` до изменения. `CompositeType` принимает «многослойка» и «строительный материал» только при наличии валидного сохранённого индекса соответствующего атрибута. Пустое/невалидное значение не меняет конструкцию; колонны, балки и стены в режиме Profile пропускаются. Если одновременно заданы оба атрибута, приоритет у `Composite`. Изменение модели через `ACAPI_Element_Change`; runtime именно этих правил пока **not verified**. [по коду Helpers.cpp, DevKit-25]

### Преобразования ParamValue [из комментариев]
`NameToRawName` (278) — имя → rawname (скобки); `GetRawnamePrefixByTypeInx`/`GetTypeInxByRawnamePrefix` (286/288) — префикс источника; `SetParamValueSourseByName` (293) — источник по rawName; `SetArrayByRawname` (295); `ReplaceParamInExpression` (310) — подстановка значений; `GetParamValueForElements` (312); `ReplaceProcToBrace` (317); `ParseParamNameMaterial` (322) — имена в %% ; `ParseParamName` (327) — имена в {}; `AddValueToParamDictValue` (332); `needAdd` (339); `AddParamValue2ParamDict` (344) и `AddParamValue2ParamDictElement` ×2 (350/358); `CheckIgnoreVal` (363); `CompareParamValue` (368); `AddParamDictValue2ParamDictElement` (378); `AddProperty` (385); `AddBool/Length/Double/StringValueToParamDictValue` (390-425); `CompareParamDictValue` ×2 (438/443); `CompareParamDictElement` (654); `Array2ParamValue` (554); конвертации `ConvertToParamValue` — GDL (:555/559/579), свойство (:589), определение (:596), IFC (:617), строка (:601), int (:606), double (:611); `ConvertBoolToParamValue` (569); `ConvertAttributeToParamValue` (574); `SetrawNameFromProperty` (584); `ConvertToParamValue_CheckAttrib` (591); `ConvertByFormatString` (620); `GetUnitsPrefix` (692); `SetUnitsAndQty2ParamValueComposite` (694); `ToString` ×4 — ParamValue (764/769), API_Variant/Property (264-267); `isEng`-независимые единицы.

### Прочее
`ACAPI_Attribute_GetAttributesByType` (623, inline, до AC27) — обёртка GetNum+Get.

### Приведение double к целым — #221
Все приведения `double` → `Int32` из данных проекта идут через `CommonFunction::DoubleToInt32` / `DoubleToInt32RoundUp` (насыщение на границах диапазона + сообщение через `msg_rep`): `ReadID`, `NumToString`, `ConvertToParamValue` (GDL/строка/угол), `ConvertToParamValue(API_Property)`, `ConvertStringToParamValue`, `ConvertDoubleToParamValue`, `ConvertToParamValue(API_IFCProperty)`, `ConvertByFormatString`. [по коду, Helpers.cpp]

Не охвачены и остаются прямыми кастами там, где диапазон гарантирован кодом выше, а не данными: нормализованный угол направления (`CoordNorthAngle`, `dir %= 8` сразу после приведения) и индексы `API_AttributeIndex` (`ComponentsCompositeStructure`, `GetAttributeValues`). [по коду Helpers.cpp]

## Операторы сравнения [по коду, hpp:772-800]
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
  (`roombook/RoombookData.cpp`, «слой не ядро») с этим набором не пересекается [проверено, #248; путь актуализирован после рефакторинга roombook 2026-10-30]
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
- `ConvertToProperty` — TODO «Переписать всё под запись ParamValue» [из комментария, hpp:430]
- Мемо: все 7 объявлений `API_ElementMemo` в Helpers.cpp — `= {}` (проверено, DISCREPANCIES #7) [проверено]
- Дубликат `CompareParamDictValue` (hpp:438 и 443 — два одинаковых объявления) [по коду — кандидат в ревью]
