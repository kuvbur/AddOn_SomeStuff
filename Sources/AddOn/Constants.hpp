//------------ kuvbur 2022 ------------
#pragma once

#if !defined(CONST_HPP)
    #define CONST_HPP // Include guard macro

    #define Menu_MonAll 1       // Команда меню: мониторинг всех элементов
    #define Menu_SyncAll 2      // Команда меню: синхронизация всех элементов
    #define Menu_SyncSelect 3   // Команда меню: синхронизация выбранных элементов
    #define Menu_wallS 4        // Команда меню: выборка стен
    #define Menu_widoS 5        // Команда меню: выборка окон/дверей
    #define Menu_objS 6         // Команда меню: выборка объектов
    #define Menu_cwallS 7       // Команда меню: выборка несущих стен
    #define Menu_ReNum 8        // Команда меню: перенумерация элементов
    #define Menu_Sum 9          // Команда меню: суммирование значений
    #define Menu_RunParam 10    // Команда меню: запуск параметров
    #define Menu_Spec 11        // Команда меню: спецификация
    #define Menu_ShowSub 12     // Команда меню: показать подэлементы
    #define Menu_SetRevision 13 // Команда меню: установить ревизию
    #define Menu_SetSub 14      // Команда меню: установить подпозицию
    #define Menu_RoomBook 15    // Команда меню: экспликация помещений
    #define Menu_AutoProfile 16 // Команда меню: автоматический профиль
    #define Menu_AutoLay 17     // Команда меню: автоматический раскрой
    #define Menu_Pallete 18     // Команда меню: открыть палитру

static const Int32 MonAll_CommandID = 1;       // ID команды мониторинга всех элементов
static const Int32 SyncAll_CommandID = 2;      // ID команды синхронизации всех элементов
static const Int32 SyncSelect_CommandID = 3;   // ID команды синхронизации выбранных элементов
static const Int32 wallS_CommandID = 4;        // ID команды выборки стен
static const Int32 widoS_CommandID = 5;        // ID команды выборки окон/дверей
static const Int32 objS_CommandID = 6;         // ID команды выборки объектов
static const Int32 cwallS_CommandID = 7;       // ID команды выборки несущих стен
static const Int32 ReNum_CommandID = 8;        // ID команды перенумерации
static const Int32 Sum_CommandID = 9;          // ID команды суммирования
static const Int32 RunParam_CommandID = 10;    // ID команды запуска параметров
static const Int32 Spec_CommandID = 11;        // ID команды спецификации
static const Int32 ShowSub_CommandID = 12;     // ID команды показа подэлементов
static const Int32 SetRevision_CommandID = 13; // ID команды установки ревизии
static const Int32 SetSub_CommandID = 14;      // ID команды установки подпозиции
static const Int32 RoomBook_CommandID = 15;    // ID команды экспликации помещений
static const Int32 Auto3D_CommandID = 16;      // ID команды автоматического профиля
static const Int32 AutoLay_CommandID = 17;     // ID команды автоматического раскроя
static const Int32 Pallete_CommandID = 18;     // ID команды открытия палитры
static const UInt32 MENU_ITEM_COUNT = 18;      // Количество пунктов меню (ID_ADDON_MENU: 1..18)

    // --- Адаптация типов под старые версии Archicad (Archicad 22) ---
    #ifndef ServerMainVers_2300
        #define API_AttributeIndex short         // Тип индекса атрибута в Archicad 22
        #define Vector2D Vector                  // Псевдоним двумерного вектора
        #define Point2D Coord                    // Псевдоним двухмерных координат
        #define API_BeamSegmentID API_BeamID     // Совместимость ID сегмента балки
        #define API_ColumnSegmentID API_ColumnID // Совместимость ID сегмента колонны
        #define API_OpeningID API_WindowID       // Совместимость ID проема
    #endif

    // --- Флаги режима синхронизации ---
    #define SYNC_RESET 1 // Флаг сброса состояния синхронизации
    #define SYNC 2       // Флаг запуска синхронизации

    // --- Режимы макета / тестового прогона (Dummy Mode) ---
    #define DUMMY_MODE_UNDEF 0 // Тестовый режим не определен
    #define DUMMY_MODE_ON 1    // Тестовый режим включен
    #define DUMMY_MODE_OFF 2   // Тестовый режим выключен

    // --- Ограничения строк ---
    #define ELEMSTR_LEN 256 // Максимальная длина строкового представления элемента

    // --- Типы операций с массивами GDL-параметров при конвертации в свойства ---
    #define ARRAY_UNDEF 0 // Операция не определена
    #define ARRAY_UNIC 1  // Вывод только уникальных значений
    #define ARRAY_SUM 2   // Вывод суммы значений (для строк — конкатенация)
    #define ARRAY_MAX 3   // Поиск максимального значения
    #define ARRAY_MIN 4   // Поиск минимального значения

    // --- Флаги работы с файлами ---
    #define FILE_LOOKUP 1 // Режим поиска / подстановки из файла

// --- ID ресурсов аддона (String Tables / Undo IDs) ---
static const Int32 VersionId = 49; // Идентификатор версии аддона

static const Int32 AddOnNameID = 1;        // ID строки с названием аддона
static const Int32 AddOnDescriptionID = 2; // ID строки с описанием аддона

// --- ID операций для стека отмены (Undo) ---
static const Int32 UndoSyncId = 1;  // ID операции отмены для синхронизации
static const Int32 SyncAllId = 2;   // ID операции полной синхронизации
static const Int32 UndoReNumId = 3; // ID операции отмены перенумерации
static const Int32 UndoSumId = 32;  // ID операции отмены суммирования

// --- ID системных/логических строк и ошибок ---
static const Int32 TrueId = 4;        // ID строки "Истина" / True
static const Int32 FalseId = 5;       // ID строки "Ложь" / False
static const Int32 ErrorSelectID = 6; // ID сообщения об ошибке выбора элементов
static const Int32 UndoDimRound = 7;  // ID операции отмены округления размеров

// --- ID строк свойств строительных материалов ---
static const Int32 BuildingMaterialNameID = 8;          // ID имени стройматериала
static const Int32 BuildingMaterialDescriptionID = 9;   // ID описания стройматериала
static const Int32 BuildingMaterialDensityID = 10;      // ID плотности стройматериала
static const Int32 BuildingMaterialManufacturerID = 11; // ID производителя стройматериала
static const Int32 ThicknessID = 12;                    // ID толщины слоя
static const Int32 RenumIgnoreID = 13;                  // ID флага "игнорировать при нумерации"
static const Int32 RenumAddID = 14;                     // ID флага "добавить при нумерации"
static const Int32 RenumSkipID = 15;                    // ID флага "пропустить при нумерации"
static const Int32 BuildingMaterialCutFillID = 16;      // ID штриховки сечения стройматериала

// --- ID строк единиц измерения ---
static const Int32 MeterStringID = 17;  // ID строки "метр" (м)
static const Int32 CMeterStringID = 18; // ID строки "сантиметр" (см)
static const Int32 DMeterStringID = 19; // ID строки "дециметр" (дм)

// --- ID строк сторон света (Ориентация) ---
static const Int32 N_StringID = 20;  // ID строки "Север" (N)
static const Int32 NW_StringID = 21; // ID строки "Северо-Запад" (NW)
static const Int32 W_StringID = 22;  // ID строки "Запад" (W)
static const Int32 SW_StringID = 23; // ID строки "Юго-Запад" (SW)
static const Int32 S_StringID = 24;  // ID строки "Юг" (S)
static const Int32 SE_StringID = 25; // ID строки "Юго-Восток" (SE)
static const Int32 E_StringID = 26;  // ID строки "Восток" (E)
static const Int32 NE_StringID = 27; // ID строки "Северо-Восток" (NE)

// --- ID строк статусов изменений ---
static const Int32 Izm_StringID = 28;   // ID строки "Изм."
static const Int32 Zam_StringID = 29;   // ID строки "Зам."
static const Int32 Nov_StringID = 30;   // ID строки "Нов."
static const Int32 Annul_StringID = 31; // ID строки "Аннул."
static const Int32 RVI_StringID = 33;   // ID строки "РВИ"

// --- ID сообщений/статусов обработки подэлементов и спецификаций ---
static const Int32 SubElementHotFoundId = 34;     // Подэлемент найден (горячая точка)
static const Int32 SubElementHotFoundId1 = 35;    // Вариант 1 найденного подэлемента
static const Int32 SubElementHotFoundId2 = 36;    // Вариант 2 найденного подэлемента
static const Int32 SubElementOtherPlanId = 37;    // Подэлемент расположен на другом плане
static const Int32 SubElementHiddenId = 38;       // Подэлемент скрыт
static const Int32 SubElementTotalId = 39;        // Итоговое количество подэлементов
static const Int32 SubElementNoSelectId = 40;     // Подэлемент не выбран
static const Int32 SubElementHalfId = 41;         // Подэлемент учтен частично
static const Int32 SpecRuleNotFoundId = 42;       // Правило спецификации не найдено
static const Int32 SpecRuleReadFoundId = 43;      // Правило чтение спецификации найдено
static const Int32 SpecWriteNotFoundId = 44;      // Ошибка записи спецификации: не найдено
static const Int32 SpecEmptyListdId = 45;         // Список спецификации пуст
static const Int32 SpecNotFoundParametersId = 46; // Не найдены параметры спецификации
static const Int32 RoombookId = 47;               // ID модуля экспликации помещений (Roombook)
static const Int32 SubElementNotExsistId = 48;    // Подэлемент не существует
static const Int32 SpecParamPlaceNotFoundId = 65; // Не найдено место размещения параметра спецификации
static const Int32 SpecFlagOff = 66;              // Флаг спецификации выключен

// --- Внутренние пути свойств материалов ---
static const GS::UniString MAT_SOME_STUFF_TH =               // Путь к свойству толщины стройматериала
    "@property:buildingmaterialproperties/some_stuff_th";    // Путь к свойству толщины стройматериала
static const GS::UniString MAT_SOME_STUFF_UNITS =            // Путь к свойству единиц измерения
    "@property:buildingmaterialproperties/some_stuff_units"; // Путь к свойству единиц измерения
static const GS::UniString MAT_SOME_STUFF_KZAP =             // Путь к свойству коэффициента запаса
    "@property:buildingmaterialproperties/some_stuff_kzap";  // Путь к свойству коэффициента запаса

// --- Граничные значения размеров массивов ---
const GS::Int32 max_group_mat = 50;  // Максимальное количество материалов у одного элемента
const GS::Int32 max_group_lib = 100; // Максимальное количество библиотечных компонентов у одного элемента

// Префикс для свойств, читаемых из параметров элемента.
static const GS::UniString PROPERTYPREF = "Property:";
// Префикс для значений, связанных с морфами.
static const GS::UniString MORPHPREF = "Morph:";
// Префикс для координат.
static const GS::UniString COORDPREF = "Coord:";
// Префикс для свойств проекта.
static const GS::UniString INFOPREF = "Info:";
// Префикс для IFC-атрибутов.
static const GS::UniString IFCPREF = "IFC:";
// Префикс для глобальных параметров.
static const GS::UniString GLOBPREF = "Glob:";
// Префикс для параметров класса.
static const GS::UniString CLASSPREF = "Class:";
// Префикс для атрибутов элемента.
static const GS::UniString ATTRIBPREF = "Attribute:";
// Префикс для свойств элемента как объекта.
static const GS::UniString ELEMENTPREF = "Element:";
// Префикс для MEP-данных.
static const GS::UniString MEPPREF = "MEP:";
// Префикс для данных из файлов.
static const GS::UniString FILEPREF = "File:";
// Префикс для данных из списков.
static const GS::UniString LISTDATAPREF = "Listdata:";
// Префикс для QR-кодов.
static const GS::UniString QRPREF = "QRCode:";
// Префикс для материалов.
static const GS::UniString MATERIALPREF = "Material:";
// Шаблон для правила синхронизации с GUID-входом.
static const GS::UniString FROMGUIDBR = "from_GUID{";
// Ключевое имя правила синхронизации с GUID-входом.
static const GS::UniString FROMGUID = "from_GUID";
// Шаблон для правила синхронизации с GUID-выходом.
static const GS::UniString TOGUIDBR = "to_GUID{";
// Ключевое имя правила синхронизации с GUID-выходом.
static const GS::UniString TOGUID = "to_GUID";
// Префикс для имен правил синхронизации.
static const GS::UniString SYNCPART = "Sync_";
// Маркер правила чтения значения из источника.
static const GS::UniString SYNCFROMSTRING = "from{";
// Маркер правила чтения значения из дочерних элементов.
static const GS::UniString SYNCFROMSUBSTRING = "from_sub{";
// Маркер правила записи значения в целевой объект.
static const GS::UniString SYNCTOSTRING = "to{";
// Маркер правила записи значения в дочерние элементы.
static const GS::UniString SYNCTOSUBSTRING = "to_sub{";

// Модуль синхронизации свойств и связанных элементов между различными источниками данных.
// Тип синхронизации
enum SyncMode {
    SYNC_NO = 0,        // Не синхронизировать
    SYNC_FROM = 1,      // Взять значение свойства из другого места
    SYNC_TO = 2,        // Записать значение свойства в другое место
    SYNC_TO_SUB = 3,    // Записать значение свойства в дочерние элементы
    SYNC_FROM_SUB = 4,  // Взять значение свойства из дочерних элементов
    SYNC_FROM_GUID = 5, // Взять значение свойства из другого объекта
    SYNC_FROM_ZONE = 6, // Взять значение свойства из Зоны, в которой находится элемент
    SYNC_TO_ZONE = 7    // Записать значение свойства в Зону, в которой находится элемент
};

// Модуль перенумерации элементов по правилам, заданным в свойствах проекта.
// Типы нумерации (см. RenumElement.state)
enum RenumMode {
    RENUM_SKIP = -1,  // Исключить из обработки
    RENUM_IGNORE = 0, // Не менять позицию, но добавлять похожие элементы
    RENUM_ADD = 1,    // Не менять позицию, если нет пропусков
    RENUM_NORMAL = 2  // Обычная нумерация/перенумерация
};

// Типы простановки нулей для СТРОКОВОГО (API_PropertyStringValueType) свойства (см. RenumRule.nulltype)
enum ZeroPaddingMode {
    NOZEROS = 0,     // Не добавлять нули в текстовое свойство
    ADDZEROS = 1,    // Добавлять нули с учётом разбивки
    ADDMAXZEROS = 2, // Добавлять нули по максимальному количеству без учёта разбивки
    ADDSPACE = 3,    // Добавлять пробелы с учётом разбивки
    ADDMAXSPACE = 4  // Добавлять пробелы по максимальному количеству без учёта разбивки
};

// Режимы суммирования значений для правил суммирования.
enum SumMode {
    // Конкатенация текстовых значений.
    TEXT_SUM = 1,
    // Суммирование числовых значений.
    NUM_SUM = 2,
    // Поиск минимального значения.
    MIN_SUM = 3,
    // Поиск максимального значения.
    MAX_SUM = 4
};

// Направление записи результата суммирования.
enum SumTarget {
    // Записать результат в свойство элемента.
    SUM_TO_PROPERTY = 1,
    // Записать результат в информацию проекта.
    SUM_TO_INFO = 2
};

// --- Префиксы и цифровые индексы типов тегов/параметров для парсинга ---
static const GS::UniString IDNAMEPREFIX = "{@id:"; // Префикс тега ID элемента
static const short IDTYPEINX = 1;                  // Индекс типа: ID элемента

static const GS::UniString PROPERTYNAMEPREFIX = "{@property:"; // Префикс тега свойства Archicad
static const short PROPERTYTYPEINX = 2;                        // Индекс типа: Свойство

static const GS::UniString COORDNAMEPREFIX = "{@coord:"; // Префикс тега координат
static const short COORDTYPEINX = 3;                     // Индекс типа: Координаты

static const GS::UniString GDLNAMEPREFIX = "{@gdl:"; // Префикс тега GDL-параметра
static const short GDLTYPEINX = 4;                   // Индекс типа: GDL-параметр

static const GS::UniString GDLDESCNAMEPREFIX = "{@description:"; // Префикс тега описания GDL-параметра
static const short GDLDESCTYPEINX = 5;                           // Индекс типа: Описание GDL

static const GS::UniString INFONAMEPREFIX = "{@info:"; // Префикс тега инфо-параметра
static const short INFOTYPEINX = 6;                    // Индекс типа: Инфо-поле

static const GS::UniString IFCNAMEPREFIX = "{@ifc:"; // Префикс тега атрибута/свойства IFC
static const short IFCTYPEINX = 7;                   // Индекс типа: IFC

static const GS::UniString MORPHNAMEPREFIX = "{@morph:"; // Префикс тега параметров Морфа
static const short MORPHTYPEINX = 8;                     // Индекс типа: Морф

static const GS::UniString ATTRIBNAMEPREFIX = "{@attrib:"; // Префикс тега атрибута
static const short ATTRIBTYPEINX = 9;                      // Индекс типа: Атрибут

static const GS::UniString LISTDATANAMEPREFIX = "{@listdata:"; // Префикс тега данных списка/сметы
static const short LISTDATATYPEINX = 10;                       // Индекс типа: Данные списка

static const GS::UniString MATERIALNAMEPREFIX = "{@material:"; // Префикс тега покрытия/материала
static const short MATERIALTYPEINX = 11;                       // Индекс типа: Материал

static const GS::UniString GLOBNAMEPREFIX = "{@glob:"; // Префикс тега глобальных переменных GDL
static const short GLOBTYPEINX = 12;                   // Индекс типа: Глобальная переменная

static const GS::UniString CLASSNAMEPREFIX = "{@class:"; // Префикс тега классификации
static const short CLASSTYPEINX = 13;                    // Индекс типа: Классификация

static const GS::UniString FORMULANAMEPREFIX = "{@formula:"; // Префикс тега вычисляемой формулы
static const short FORMULATYPEINX = 14;                      // Индекс типа: Формула

static const GS::UniString ELEMENTNAMEPREFIX = "{@element:"; // Префикс тега свойств самого элемента
static const short ELEMENTTYPEINX = 15;                      // Индекс типа: Элемент

static const GS::UniString MEPNAMEPREFIX = "{@mep:"; // Префикс тега систем инженерных сетей (MEP)
static const short MEPTYPEINX = 16;                  // Индекс типа: MEP

static const GS::UniString FILENAMEPREFIX = "{@file:"; // Префикс тега внешней подстановки из файла
static const short FILETYPEINX = 17;                   // Индекс типа: Файл

static const GS::UniString FLAGNAMEPREFIX = "{@flag:"; // Префикс тега флага
static const short FLAGTYPEINX = 18;                   // Индекс типа: Флаг

// --- Константы спецсимволов, разделителей и единиц измерения ---
static const GS::UniString DOT = ".";                             // Точка (строка)
static const GS::UniString COMMA = ",";                           // Запятая (строка)
static const GS::UniChar CHARCOMMA = ',';                         // Запятая (символ)
static const GS::UniChar CHARDOT = ',';                           // Опечатка в исходнике (запятая вместо точки, символ)
static const GS::UniString METERS = "m";                          // Метры (строка)
static const GS::UniChar CHARMETERS = 'm';                        // Символ 'm'
static const GS::UniString MMETERS = "mm";                        // Миллиметры
static const GS::UniString CMETERS = "cm";                        // Сантиметры
static const GS::UniString DMETERS = "dm";                        // Дециметры
static const GS::UniString DSTRING = "d";                         // Префикс/Обозначение "д" (деци-)
static const GS::UniString CSTRING = "c";                         // Префикс/Обозначение "с" (санти-)
static const GS::UniString GSTRING = "g";                         // Обозначение "г" (грамм)
static const GS::UniString KSTRING = "k";                         // Префикс/Обозначение "к" (кило-)
static const GS::UniString GMETERS = "gm";                        // Грамм-метры / Погонная масса
static const GS::UniString KMETERS = "km";                        // Километры
static const GS::UniString DOTSET = "p";                          // Флаг настройки разделения точкой
static const GS::UniString RDSET = "r";                           // Флаг настройки округления
static const GS::UniString FSET = "f";                            // Флаг настройки форматирования
static const GS::UniString EMPTYSTRING = "";                      // Пустая строка
static const GS::UniString ZEROSTRING = "0";                      // Символ "0" (строка)
static const GS::UniChar CHARZERO = '0';                          // Символ '0'
static const GS::UniString SPACESTRING = " ";                     // Пробел
static const GS::UniString MINUSSTRING = "-";                     // Минус
static const GS::UniChar CHARFORMULASTART = '<';                  // Открывающий символ формулы
static const GS::UniChar CHARFORMULAEND = '>';                    // Закрывающий символ формулы
static const GS::UniString STRFORMULASTART = "<";                 // Открывающая строка формулы
static const GS::UniString STRFORMULAEND = ">";                   // Закрывающая строка формулы
static const GS::UniString BRACEEND = "}";                        // Закрывающая фигурная скобка (строка)
static const GS::UniString BRACESTART = "{";                      // Открывающая фигурная скобка (строка)
static const GS::UniString SEMICOLON = ";";                       // Точка с запятой (строка)
static const GS::UniChar CHARBSEMICOLON = ';';                    // Точка с запятой (символ)
static const GS::UniChar CHARBRACEEND = '}';                      // Закрывающая фигурная скобка (символ)
static const GS::UniChar CHARBRACESTART = '{';                    // Открывающая фигурная скобка (символ)
static const GS::UniString STRINGPROC = "%";                      // Символ процента (строка)
static const GS::UniChar CHARPROC = '%';                          // Символ процента (символ)
static const GS::UniChar CHARDQUT = '"';                          // Двойная кавычка (символ)
static const GS::UniString PVALPREFIX = "{@";                     // Префикс начала тега параметра
static const GS::UniString RENUMFLAG = "Renum_flag";              // Имя параметра/свойства флага перенумерации
static const GS::UniString RENUM = "Renum";                       // Имя свойства перенумерации
static const GS::UniString PROPERTYSTRING = "property";           // Ключевое слово "property"
static const GS::UniString LINEBRAKE = "\n";                      // Перевод строки (LF)
static const GS::UniString LINEBRAKER = "\r";                     // Возврат каретки (CR)
static const GS::UniString TABSTRING = "\t";                      // Символ табуляции
static const GS::UniString SLASHEKR = "\\/";                      // Экранированный слэш
static const GS::UniString SLASH = "/";                           // Прямой слэш
static const GS::UniString ATSIGN = "@";                          // Символ "собачка"
static const GS::UniString SYNCFLAG = "Sync_flag";                // Поле флага синхронизации
static const GS::UniString SYNCCORRECTFLAG = "Sync_correct_flag"; // Поле флага корректировки синхронизации
static const GS::UniString SYNCCLASSFLAG = "Sync_class_flag";     // Поле флага синхронизации классов
static const GS::UniString SYNCGUID = "Sync_GUID";                // Поле уникального идентификатора синхронизации

// --- Шаблоны форматирования чисел по умолчанию ---
static const GS::UniString DEFULTREALFSTRING = ".3m";  // Формат по умолчанию для вещественных чисел (3 знака)
static const GS::UniString DEFULTLEGHTFSTRING = "1mm"; // Формат по умолчанию для длины (в мм)
static const GS::UniString DEFULTINTFSTRING = "0m";    // Формат по умолчанию для целых чисел
static const GS::UniString SYNCNAME = "sync_name";     // Ключ имени синхронизации

// --- Настройки кодировки ---
static const GSCharCode GChCode = CC_Cyrillic; // Выбор кириллической кодировки для GS::UniString

// --- Строковые ключи встроенных свойств Archicad (@property:) ---
static const GS::UniString PROP_PREFIX = "@property:";             // Базовый префикс свойств
static const GS::UniString PROP_SYNC_NAME = "@property:sync_name"; // Свойство: Имя синхронизации
static const GS::UniString PROP_ID = "@property:id";               // Свойство: ID элемента
static const GS::UniString PROP_N = "@property:n";                 // Свойство: Порядковый номер / Позиция
static const GS::UniString PROP_NS = "@property:ns";               // Свойство: Номер спецификации / Подпозиция
static const GS::UniString PROP_LAYER_THICKNESS = "@property:layer_thickness";         // Свойство: Толщина слоя
static const GS::UniString PROP_TH = "@property:th";                                   // Свойство: Сокращение толщины
static const GS::UniString PROP_LAYER_MIN_THICKNESS = "@property:layer_min_thickness"; // Свойство: Мин. толщина слоя
static const GS::UniString PROP_TH_MIN = "@property:th_min";               // Свойство: Сокращение мин. толщины
static const GS::UniString PROP_BMAT_INX = "@property:bmat_inx";           // Свойство: Индекс стройматериала
static const GS::UniString PROP_CUTFILL_INX = "@property:cutfill_inx";     // Свойство: Индекс штриховки сечения
static const GS::UniString PROP_SOME_STUFF_TH = "@property:some_stuff_th"; // Свойство: Кастомная толщина
static const GS::UniString PROP_SOME_STUFF_UNITS =                         // Свойство: Кастомные единицы измерения
    "@property:some_stuff_units";                                          // Свойство: Кастомные единицы измерения
static const GS::UniString PROP_UNIT = "@property:unit";                   // Свойство: Единица измерения
static const GS::UniString PROP_KZAP = "@property:kzap";                   // Свойство: Коэффициент запаса
static const GS::UniString PROP_AREA = "@property:area";                   // Свойство: Площадь
static const GS::UniString PROP_VOLUME = "@property:volume";               // Свойство: Объем
static const GS::UniString PROP_QTY = "@property:qty";                     // Свойство: Количество
static const GS::UniString PROP_UNIT_PREFIX = "@property:unit_prefix";     // Свойство: Префикс единицы измерения
static const GS::UniString PROP_LENGTH = "@property:length";               // Свойство: Длина
static const GS::UniString PROP_AREA_SECTION = "@property:area_section";   // Свойство: Площадь сечения
static const GS::UniString PROP_WIDTH = "@property:width";                 // Свойство: Ширина
static const GS::UniString MAT_BUILDING_MATERIAL_ID =                      // Путь к ID стройматериала
    "@property:BuildingMaterialProperties/Building Material ID";           // Путь к ID стройматериала

// --- Строковые ключи свойств компонентов/материалов (@material:) ---
static const GS::UniString MAT_N = "@material:n";                                     // Материал: Номер/Позиция
static const GS::UniString MAT_NS = "@material:ns";                                   // Материал: Подпозиция
static const GS::UniString MAT_LAYER_THICKNESS = "@material:layer thickness";         // Материал: Толщина слоя
static const GS::UniString MAT_LAYER_MIN_THICKNESS = "@material:layer min thickness"; // Материал: Мин. толщина слоя
static const GS::UniString MAT_BMAT_INX = "@material:bmat_inx";                       // Материал: Индекс стройматериала
static const GS::UniString MAT_CUTFILL_INX = "@material:cutfill_inx";                 // Материал: Индекс штриховки
static const GS::UniString MAT_AREA = "@material:area";                               // Материал: Площадь
static const GS::UniString MAT_VOLUME = "@material:volume";                           // Материал: Объем
static const GS::UniString MAT_QTY = "@material:qty";                                 // Материал: Количество
static const GS::UniString MAT_UNIT_PREFIX = "@material:unit_prefix";                 // Материал: Префикс единицы
static const GS::UniString MAT_LENGTH = "@material:length";                           // Материал: Длина
static const GS::UniString MAT_AREA_SECTION = "@material:area_section";               // Материал: Площадь сечения
static const GS::UniString MAT_WIDTH = "@material:width";                             // Материал: Ширина

// --- Полные путевые свойства строительных материалов в системе Archicad ---
static const GS::UniString MAT_BUILDING_MATERIAL_NAME =                    // Наименование стройматериала
    "@property:BuildingMaterialProperties/Building Material Name";         // Наименование стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_DESCRIPTION =             // Описание стройматериала
    "@property:BuildingMaterialProperties/Building Material Description";  // Описание стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_DENSITY =                 // Плотность стройматериала
    "@property:BuildingMaterialProperties/Building Material Density";      // Плотность стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_MANUFACTURER =            // Производитель стройматериала
    "@property:BuildingMaterialProperties/Building Material Manufacturer"; // Производитель стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_CUTFILL =                 // Штриховка сечения стройматериала
    "@property:BuildingMaterialProperties/Building Material CutFill";      // Штриховка сечения стройматериала

// --- Массивы допустимых типов для операций сбора/записи данных ---

// Полный список индексов типов тегов, поддерживаемых для считывания/обработки
static const GS::Array<short> paramTypesList = {IDTYPEINX,
                                                PROPERTYTYPEINX,
                                                COORDTYPEINX,
                                                GDLTYPEINX,
                                                INFOTYPEINX,
                                                IFCTYPEINX,
                                                MORPHTYPEINX,
                                                ATTRIBTYPEINX,
                                                LISTDATATYPEINX,
                                                MATERIALTYPEINX,
                                                GLOBTYPEINX,
                                                CLASSTYPEINX,
                                                FILETYPEINX,
                                                FORMULATYPEINX,
                                                ELEMENTTYPEINX,
                                                MEPTYPEINX}; // Список поддерживаемых типов тегов для чтения/обработки

static const GS::UniString idRawname = "{@id:id}";               // Готовый тег для получения ID элемента
static const GS::UniString attrlayerRawname = "{@attrib:layer}"; // Готовый тег для получения слоя элемента

// Список индексов типов тегов, в которые разрешена обратная запись параметров
static const GS::Array<short> paramTypesListWrite = {PROPERTYTYPEINX,
                                                     GDLTYPEINX,
                                                     IDTYPEINX,
                                                     CLASSTYPEINX,
                                                     ATTRIBTYPEINX,
                                                     COORDTYPEINX}; // Список типов тегов, разрешённых для записи

#endif
