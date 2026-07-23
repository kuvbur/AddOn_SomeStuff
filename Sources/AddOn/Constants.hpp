//------------ kuvbur 2022 ------------
#pragma once

#if !defined(CONST_HPP)
    #define CONST_HPP

    // --- Адаптация типов под старые версии Archicad (Archicad 22) ---
    #ifdef AC_22
        #define API_AttributeIndex short          // Тип индекса атрибута в Archicad 22
        #define Vector2D Vector                   // Псевдоним двумерного вектора
        #define Point2D Coord                     // Псевдоним двухмерных координат
        #define API_BeamSegmentID API_BeamID      // Совместимость ID сегмента балки
        #define API_ColumnSegmentID API_ColumnID  // Совместимость ID сегмента колонны
        #define API_OpeningID API_WindowID        // Совместимость ID проема
    #endif

    // --- Флаги режима синхронизации ---
    #define SYNC_RESET 1                          // Флаг сброса состояния синхронизации
    #define SYNC 2                                // Флаг запуска синхронизации

    // --- Режимы макета / тестового прогона (Dummy Mode) ---
    #define DUMMY_MODE_UNDEF 0                    // Тестовый режим не определен
    #define DUMMY_MODE_ON 1                       // Тестовый режим включен
    #define DUMMY_MODE_OFF 2                      // Тестовый режим выключен

    // --- Ограничения строк ---
    #define ELEMSTR_LEN 256                       // Максимальная длина строкового представления элемента

    // --- Типы операций с массивами GDL-параметров при конвертации в свойства ---
    #define ARRAY_UNDEF 0                         // Операция не определена
    #define ARRAY_UNIC 1                          // Вывод только уникальных значений
    #define ARRAY_SUM 2                           // Вывод суммы значений (для строк — конкатенация)
    #define ARRAY_MAX 3                           // Поиск максимального значения
    #define ARRAY_MIN 4                           // Поиск минимального значения

    // --- Флаги работы с файлами ---
    #define FILE_LOOKUP 1                         // Режим поиска / подстановки из файла

// --- ID ресурсов аддона (String Tables / Undo IDs) ---
static const Int32 VersionId = 49;                // Идентификатор версии аддона

static const Int32 AddOnNameID = 1;               // ID строки с названием аддона
static const Int32 AddOnDescriptionID = 2;        // ID строки с описанием аддона

// --- ID операций для стека отмены (Undo) ---
static const Int32 UndoSyncId = 1;                // ID операции отмены для синхронизации
static const Int32 SyncAllId = 2;                 // ID операции полной синхронизации
static const Int32 UndoReNumId = 3;               // ID операции отмены перенумерации
static const Int32 UndoSumId = 32;                // ID операции отмены суммирования

// --- ID системных/логических строк и ошибок ---
static const Int32 TrueId = 4;                    // ID строки "Истина" / True
static const Int32 FalseId = 5;                   // ID строки "Ложь" / False
static const Int32 ErrorSelectID = 6;             // ID сообщения об ошибке выбора элементов
static const Int32 UndoDimRound = 7;              // ID операции отмены округления размеров

// --- ID строк свойств строительных материалов ---
static const Int32 BuildingMaterialNameID = 8;         // ID имени стройматериала
static const Int32 BuildingMaterialDescriptionID = 9;  // ID описания стройматериала
static const Int32 BuildingMaterialDensityID = 10;     // ID плотности стройматериала
static const Int32 BuildingMaterialManufacturerID = 11;// ID производителя стройматериала
static const Int32 ThicknessID = 12;                   // ID толщины слоя
static const Int32 RenumIgnoreID = 13;                // ID флага "игнорировать при нумерации"
static const Int32 RenumAddID = 14;                   // ID флага "добавить при нумерации"
static const Int32 RenumSkipID = 15;                  // ID флага "пропустить при нумерации"
static const Int32 BuildingMaterialCutFillID = 16;    // ID штриховки сечения стройматериала

// --- ID строк единиц измерения ---
static const Int32 MeterStringID = 17;                 // ID строки "метр" (м)
static const Int32 CMeterStringID = 18;                // ID строки "сантиметр" (см)
static const Int32 DMeterStringID = 19;                // ID строки "дециметр" (дм)

// --- ID строк сторон света (Ориентация) ---
static const Int32 N_StringID = 20;                    // ID строки "Север" (N)
static const Int32 NW_StringID = 21;                   // ID строки "Северо-Запад" (NW)
static const Int32 W_StringID = 22;                    // ID строки "Запад" (W)
static const Int32 SW_StringID = 23;                   // ID строки "Юго-Запад" (SW)
static const Int32 S_StringID = 24;                    // ID строки "Юг" (S)
static const Int32 SE_StringID = 25;                   // ID строки "Юго-Восток" (SE)
static const Int32 E_StringID = 26;                    // ID строки "Восток" (E)
static const Int32 NE_StringID = 27;                   // ID строки "Северо-Восток" (NE)

// --- ID строк статусов изменений ---
static const Int32 Izm_StringID = 28;                  // ID строки "Изм."
static const Int32 Zam_StringID = 29;                  // ID строки "Зам."
static const Int32 Nov_StringID = 30;                  // ID строки "Нов."
static const Int32 Annul_StringID = 31;                // ID строки "Аннул."
static const Int32 RVI_StringID = 33;                  // ID строки "РВИ"

// --- ID сообщений/статусов обработки подэлементов и спецификаций ---
static const Int32 SubElementHotFoundId = 34;          // Подэлемент найден (горячая точка)
static const Int32 SubElementHotFoundId1 = 35;         // Вариант 1 найденного подэлемента
static const Int32 SubElementHotFoundId2 = 36;         // Вариант 2 найденного подэлемента
static const Int32 SubElementOtherPlanId = 37;         // Подэлемент расположен на другом плане
static const Int32 SubElementHiddenId = 38;            // Подэлемент скрыт
static const Int32 SubElementTotalId = 39;             // Итоговое количество подэлементов
static const Int32 SubElementNoSelectId = 40;          // Подэлемент не выбран
static const Int32 SubElementHalfId = 41;             // Подэлемент учтен частично
static const Int32 SpecRuleNotFoundId = 42;            // Правило спецификации не найдено
static const Int32 SpecRuleReadFoundId = 43;           // Правило чтение спецификации найдено
static const Int32 SpecWriteNotFoundId = 44;          // Ошибка записи спецификации: не найдено
static const Int32 SpecEmptyListdId = 45;             // Список спецификации пуст
static const Int32 SpecNotFoundParametersId = 46;      // Не найдены параметры спецификации
static const Int32 RoombookId = 47;                    // ID модуля экспликации помещений (Roombook)
static const Int32 SubElementNotExsistId = 48;         // Подэлемент не существует
static const Int32 SpecParamPlaceNotFoundId = 65;      // Не найдено место размещения параметра спецификации
static const Int32 SpecFlagOff = 66;                   // Флаг спецификации выключен

// --- Внутренние пути свойств материалов ---
static const GS::UniString MAT_SOME_STUFF_TH = "@property:buildingmaterialproperties/some_stuff_th";       // Путь к свойству толщины стройматериала
static const GS::UniString MAT_SOME_STUFF_UNITS = "@property:buildingmaterialproperties/some_stuff_units"; // Путь к свойству единиц измерения
static const GS::UniString MAT_SOME_STUFF_KZAP = "@property:buildingmaterialproperties/some_stuff_kzap";  // Путь к свойству коэффициента запаса

// --- Граничные значения размеров массивов ---
const GS::Int32 max_group_mat = 50;  // Максимальное количество материалов у одного элемента
const GS::Int32 max_group_lib = 100; // Максимальное количество библиотечных компонентов у одного элемента

// --- Префиксы и цифровые индексы типов тегов/параметров для парсинга ---
static const GS::UniString IDNAMEPREFIX = "{@id:";           // Префикс тега ID элемента
static const short IDTYPEINX = 1;                            // Индекс типа: ID элемента

static const GS::UniString PROPERTYNAMEPREFIX = "{@property:";// Префикс тега свойства Archicad
static const short PROPERTYTYPEINX = 2;                      // Индекс типа: Свойство

static const GS::UniString COORDNAMEPREFIX = "{@coord:";     // Префикс тега координат
static const short COORDTYPEINX = 3;                         // Индекс типа: Координаты

static const GS::UniString GDLNAMEPREFIX = "{@gdl:";         // Префикс тега GDL-параметра
static const short GDLTYPEINX = 4;                           // Индекс типа: GDL-параметр

static const GS::UniString GDLDESCNAMEPREFIX = "{@description:"; // Префикс тега описания GDL-параметра
static const short GDLDESCTYPEINX = 5;                       // Индекс типа: Описание GDL

static const GS::UniString INFONAMEPREFIX = "{@info:";       // Префикс тега инфо-параметра
static const short INFOTYPEINX = 6;                          // Индекс типа: Инфо-поле

static const GS::UniString IFCNAMEPREFIX = "{@ifc:";         // Префикс тега атрибута/свойства IFC
static const short IFCTYPEINX = 7;                           // Индекс типа: IFC

static const GS::UniString MORPHNAMEPREFIX = "{@morph:";     // Префикс тега параметров Морфа
static const short MORPHTYPEINX = 8;                         // Индекс типа: Морф

static const GS::UniString ATTRIBNAMEPREFIX = "{@attrib:";   // Префикс тега атрибута
static const short ATTRIBTYPEINX = 9;                        // Индекс типа: Атрибут

static const GS::UniString LISTDATANAMEPREFIX = "{@listdata:";// Префикс тега данных списка/сметы
static const short LISTDATATYPEINX = 10;                     // Индекс типа: Данные списка

static const GS::UniString MATERIALNAMEPREFIX = "{@material:";// Префикс тега покрытия/материала
static const short MATERIALTYPEINX = 11;                     // Индекс типа: Материал

static const GS::UniString GLOBNAMEPREFIX = "{@glob:";       // Префикс тега глобальных переменных GDL
static const short GLOBTYPEINX = 12;                         // Индекс типа: Глобальная переменная

static const GS::UniString CLASSNAMEPREFIX = "{@class:";     // Префикс тега классификации
static const short CLASSTYPEINX = 13;                        // Индекс типа: Классификация

static const GS::UniString FORMULANAMEPREFIX = "{@formula:"; // Префикс тега вычисляемой формулы
static const short FORMULATYPEINX = 14;                      // Индекс типа: Формула

static const GS::UniString ELEMENTNAMEPREFIX = "{@element:"; // Префикс тега свойств самого элемента
static const short ELEMENTTYPEINX = 15;                      // Индекс типа: Элемент

static const GS::UniString MEPNAMEPREFIX = "{@mep:";         // Префикс тега систем инженерных сетей (MEP)
static const short MEPTYPEINX = 16;                          // Индекс типа: MEP

static const GS::UniString FILENAMEPREFIX = "{@file:";       // Префикс тега внешней подстановки из файла
static const short FILETYPEINX = 17;                         // Индекс типа: Файл

// --- Константы спецсимволов, разделителей и единиц измерения ---
static const GS::UniString DOT = ".";                        // Точка (строка)
static const GS::UniString COMMA = ",";                      // Запятая (строка)
static const GS::UniChar CHARCOMMA = ',';                    // Запятая (символ)
static const GS::UniChar CHARDOT = ',';                      // Опечатка в исходнике (запятая вместо точки, символ)
static const GS::UniString METERS = "m";                     // Метры (строка)
static const GS::UniChar CHARMETERS = 'm';                   // Символ 'm'
static const GS::UniString MMETERS = "mm";                   // Миллиметры
static const GS::UniString CMETERS = "cm";                   // Сантиметры
static const GS::UniString DMETERS = "dm";                   // Дециметры
static const GS::UniString DSTRING = "d";                    // Префикс/Обозначение "д" (деци-)
static const GS::UniString CSTRING = "c";                    // Префикс/Обозначение "с" (санти-)
static const GS::UniString GSTRING = "g";                    // Обозначение "г" (грамм)
static const GS::UniString KSTRING = "k";                    // Префикс/Обозначение "к" (кило-)
static const GS::UniString GMETERS = "gm";                   // Грамм-метры / Погонная масса
static const GS::UniString KMETERS = "km";                   // Километры
static const GS::UniString DOTSET = "p";                     // Флаг настройки разделения точкой
static const GS::UniString RDSET = "r";                      // Флаг настройки округления
static const GS::UniString FSET = "f";                       // Флаг настройки форматирования
static const GS::UniString EMPTYSTRING = "";                 // Пустая строка
static const GS::UniString ZEROSTRING = "0";                 // Символ "0" (строка)
static const GS::UniChar CHARZERO = '0';                     // Символ '0'
static const GS::UniString SPACESTRING = " ";                // Пробел
static const GS::UniString MINUSSTRING = "-";                // Минус
static const GS::UniChar CHARFORMULASTART = '<';            // Открывающий символ формулы
static const GS::UniChar CHARFORMULAEND = '>';              // Закрывающий символ формулы
static const GS::UniString STRFORMULASTART = "<";            // Открывающая строка формулы
static const GS::UniString STRFORMULAEND = ">";              // Закрывающая строка формулы
static const GS::UniString BRACEEND = "}";                   // Закрывающая фигурная скобка (строка)
static const GS::UniString BRACESTART = "{";                 // Открывающая фигурная скобка (строка)
static const GS::UniString SEMICOLON = ";";                  // Точка с запятой (строка)
static const GS::UniChar CHARBSEMICOLON = ';';              // Точка с запятой (символ)
static const GS::UniChar CHARBRACEEND = '}';                 // Закрывающая фигурная скобка (символ)
static const GS::UniChar CHARBRACESTART = '{';               // Открывающая фигурная скобка (символ)
static const GS::UniString STRINGPROC = "%";                 // Символ процента (строка)
static const GS::UniChar CHARPROC = '%';                     // Символ процента (символ)
static const GS::UniChar CHARDQUT = '"';                     // Двойная кавычка (символ)
static const GS::UniString PVALPREFIX = "{@";                // Префикс начала тега параметра
static const GS::UniString RENUMFLAG = "Renum_flag";         // Имя параметра/свойства флага перенумерации
static const GS::UniString RENUM = "Renum";                  // Имя свойства перенумерации
static const GS::UniString PROPERTYSTRING = "property";      // Ключевое слово "property"
static const GS::UniString LINEBRAKE = "\n";                 // Перевод строки (LF)
static const GS::UniString LINEBRAKER = "\r";                // Возврат каретки (CR)
static const GS::UniString TABSTRING = "\t";                 // Символ табуляции
static const GS::UniString SLASHEKR = "\\/";                 // Экранированный слэш
static const GS::UniString SLASH = "/";                      // Прямой слэш
static const GS::UniString ATSIGN = "@";                     // Символ "собачка"
static const GS::UniString SYNCFLAG = "Sync_flag";           // Поле флага синхронизации
static const GS::UniString SYNCCORRECTFLAG = "Sync_correct_flag"; // Поле флага корректировки синхронизации
static const GS::UniString SYNCCLASSFLAG = "Sync_class_flag";     // Поле флага синхронизации классов
static const GS::UniString SYNCGUID = "Sync_GUID";           // Поле уникального идентификатора синхронизации

// --- Шаблоны форматирования чисел по умолчанию ---
static const GS::UniString DEFULTREALFSTRING = ".3m";        // Формат по умолчанию для вещественных чисел (3 знака)
static const GS::UniString DEFULTLEGHTFSTRING = "1mm";        // Формат по умолчанию для длины (в мм)
static const GS::UniString DEFULTINTFSTRING = "0m";          // Формат по умолчанию для целых чисел
static const GS::UniString SYNCNAME = "sync_name";           // Ключ имени синхронизации

// --- Настройки кодировки ---
static const GSCharCode GChCode = CC_Cyrillic;               // Выбор кириллической кодировки для GS::UniString

// --- Строковые ключи встроенных свойств Archicad (@property:) ---
static const GS::UniString PROP_PREFIX = "@property:";                            // Базовый префикс свойств
static const GS::UniString PROP_SYNC_NAME = "@property:sync_name";                // Свойство: Имя синхронизации
static const GS::UniString PROP_ID = "@property:id";                              // Свойство: ID элемента
static const GS::UniString PROP_N = "@property:n";                                // Свойство: Порядковый номер / Позиция
static const GS::UniString PROP_NS = "@property:ns";                              // Свойство: Номер спецификации / Подпозиция
static const GS::UniString PROP_LAYER_THICKNESS = "@property:layer_thickness";    // Свойство: Толщина слоя
static const GS::UniString PROP_TH = "@property:th";                              // Свойство: Сокращение толщины
static const GS::UniString PROP_LAYER_MIN_THICKNESS = "@property:layer_min_thickness"; // Свойство: Мин. толщина слоя
static const GS::UniString PROP_TH_MIN = "@property:th_min";                      // Свойство: Сокращение мин. толщины
static const GS::UniString PROP_BMAT_INX = "@property:bmat_inx";                  // Свойство: Индекс стройматериала
static const GS::UniString PROP_CUTFILL_INX = "@property:cutfill_inx";            // Свойство: Индекс штриховки сечения
static const GS::UniString PROP_SOME_STUFF_TH = "@property:some_stuff_th";        // Свойство: Кастомная толщина
static const GS::UniString PROP_SOME_STUFF_UNITS = "@property:some_stuff_units";  // Свойство: Кастомные единицы измерения
static const GS::UniString PROP_UNIT = "@property:unit";                          // Свойство: Единица измерения
static const GS::UniString PROP_KZAP = "@property:kzap";                          // Свойство: Коэффициент запаса
static const GS::UniString PROP_AREA = "@property:area";                          // Свойство: Площадь
static const GS::UniString PROP_VOLUME = "@property:volume";                      // Свойство: Объем
static const GS::UniString PROP_QTY = "@property:qty";                            // Свойство: Количество
static const GS::UniString PROP_UNIT_PREFIX = "@property:unit_prefix";            // Свойство: Префикс единицы измерения
static const GS::UniString PROP_LENGTH = "@property:length";                      // Свойство: Длина
static const GS::UniString PROP_AREA_SECTION = "@property:area_section";          // Свойство: Площадь сечения
static const GS::UniString PROP_WIDTH = "@property:width";                        // Свойство: Ширина
static const GS::UniString MAT_BUILDING_MATERIAL_ID = "@property:BuildingMaterialProperties/Building Material ID"; // Путь к ID стройматериала

// --- Строковые ключи свойств компонентов/материалов (@material:) ---
static const GS::UniString MAT_N = "@material:n";                                 // Материал: Номер/Позиция
static const GS::UniString MAT_NS = "@material:ns";                               // Материал: Подпозиция
static const GS::UniString MAT_LAYER_THICKNESS = "@material:layer thickness";      // Материал: Толщина слоя
static const GS::UniString MAT_LAYER_MIN_THICKNESS = "@material:layer min thickness"; // Материал: Мин. толщина слоя
static const GS::UniString MAT_BMAT_INX = "@material:bmat_inx";                   // Материал: Индекс стройматериала
static const GS::UniString MAT_CUTFILL_INX = "@material:cutfill_inx";             // Материал: Индекс штриховки
static const GS::UniString MAT_AREA = "@material:area";                           // Материал: Площадь
static const GS::UniString MAT_VOLUME = "@material:volume";                       // Материал: Объем
static const GS::UniString MAT_QTY = "@material:qty";                             // Материал: Количество
static const GS::UniString MAT_UNIT_PREFIX = "@material:unit_prefix";             // Материал: Префикс единицы
static const GS::UniString MAT_LENGTH = "@material:length";                       // Материал: Длина
static const GS::UniString MAT_AREA_SECTION = "@material:area_section";           // Материал: Площадь сечения
static const GS::UniString MAT_WIDTH = "@material:width";                         // Материал: Ширина

// --- Полные путевые свойства строительных материалов в системе Archicad ---
static const GS::UniString MAT_BUILDING_MATERIAL_NAME = "@property:BuildingMaterialProperties/Building Material Name";               // Наименование стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_DESCRIPTION = "@property:BuildingMaterialProperties/Building Material Description"; // Описание стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_DENSITY = "@property:BuildingMaterialProperties/Building Material Density";         // Плотность стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_MANUFACTURER = "@property:BuildingMaterialProperties/Building Material Manufacturer"; // Производитель стройматериала
static const GS::UniString MAT_BUILDING_MATERIAL_CUTFILL = "@property:BuildingMaterialProperties/Building Material CutFill";         // Штриховка сечения стройматериала

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
                                                MEPTYPEINX};

static const GS::UniString idRawname = "{@id:id}";             // Готовый тег для получения ID элемента
static const GS::UniString attrlayerRawname = "{@attrib:layer}";// Готовый тег для получения слоя элемента

// Список индексов типов тегов, в которые разрешена обратная запись параметров
static const GS::Array<short> paramTypesListWrite = {
    PROPERTYTYPEINX, GDLTYPEINX, IDTYPEINX, CLASSTYPEINX, ATTRIBTYPEINX, COORDTYPEINX};

#endif
