//------------ kuvbur 2022 ------------
#pragma once
#if !defined (COMMON_HPP)
#define	COMMON_HPP
#include "ACAPinc.h"
#include "APIEnvir.h"
#ifdef AC_22
#include "APICommon22.h"
#endif // AC_25
#ifdef AC_23
#include "APICommon23.h"
#endif // AC_25
#ifdef AC_24
#include "APICommon24.h"
#endif // AC_25
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include "APICommon28.h"
#endif // AC_27
#include "ResourceIds.hpp"
#include "DG.h"
#include "exprtk.h"
#include <unordered_map>
#include "alphanum.h"
#include <Definitions.hpp>
#include "Point2D.hpp"
#include "Polygon2DData.h"
#include "Polygon2DDataConv.h"
#include "Sector2DData.h"
#include <APIdefs_LibraryParts.h>
#ifdef AC_22
#define API_AttributeIndex short
#define Vector2D Vector
#define Point2D Coord
#define API_BeamSegmentID API_BeamID
#define API_ColumnSegmentID API_ColumnID
#define API_OpeningID API_WindowID
#endif

static const Int32 MeterStringID = 17;
static const Int32 CMeterStringID = 18;
static const Int32 DMeterStringID = 19;

#define ELEMSTR_LEN 256
static const GSCharCode GChCode = CC_Cyrillic;
typedef std::map<std::string, API_Guid, doj::alphanum_less<std::string>> SortByName; // Словарь для сортировки наруальным алгоритмом

struct Story {
    Story (short _index, double _level)
        : index (_index)
        , level (_level) {
    }
    short  index;
    double level;
};
using Stories = GS::Array<Story>; // Хранение информации об этажах в формате Индекс - Уровень

// Структура для хранения формата перевода чисел в строку и округления чисел
typedef struct {
    int n_zero = 3; //Количество нулей после запятой
    GS::UniString stringformat = ""; // Формат строки (задаётся с помощью .mm или .0)
    bool needRound = false; //Использовать в расчётах округлённые значения
    Int32 krat = 0; // Крутность округления
    double koeff = 1; //Коэфф. увеличения
    bool trim_zero = true; //Требуется образать нули после запятой
    bool isRead = false; // Строка успешно распарсена
    bool isEmpty = true; // Строка не задана
    bool forceRaw = false; // Использовать неокруглённое значение для записи
    GS::UniString delimetr = ",";
} FormatString;

// Словарь с форматированием и округлением
typedef GS::HashTable<API_PropertyMeasureType, FormatString> FormatStringDict;

// Словарь уникальных API_Guid
typedef GS::HashTable<API_Guid, bool> UnicGuid;
// -----------------------------------------------------------------------------
// Читает информацию об этажах в проекте
// -----------------------------------------------------------------------------
Stories GetStories ();

// -----------------------------------------------------------------------------
// Переводит абсолютную координату z в индекс этажа и относительный отступ низа
// -----------------------------------------------------------------------------
GS::Pair<short, double> GetFloorIndexAndOffset (const double zPos, const Stories& stories);

// -----------------------------------------------------------------------------
// Переводит индекс этажа и относительный отступ низа в абсолютную координату z
// -----------------------------------------------------------------------------
double GetzPos (const double bottomOffset, const short floorInd, const Stories& stories);

GS::UniString TextToQRCode (GS::UniString& text, int error_lvl);

GS::UniString TextToQRCode (GS::UniString& text);

GS::UniString GetPropertyNameByGUID (const API_Guid& guid);

void DBprnt (GS::UniString msg, GS::UniString reportString = "");

void DBtest (bool usl, GS::UniString reportString, bool asserton = false);

void DBtest (GS::UniString a, GS::UniString b, GS::UniString reportString, bool asserton = false);

void DBtest (double a, double b, GS::UniString reportString, bool asserton = false);

// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng ();

// -----------------------------------------------------------------------------
// Вывод сообщения в отчёт
// -----------------------------------------------------------------------------
void msg_rep (const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid, bool show = false);

// --------------------------------------------------------------------
// Отмечает заданный пункт активным/неактивным
// --------------------------------------------------------------------
void MenuItemCheckAC (short itemInd, bool checked);

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Версия без чтения настроек
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements2 (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/);

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid)
//  Версия без чтения настроек
// -----------------------------------------------------------------------------
void CallOnSelectedElem2 (void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString& funcname /* = ""*/, bool addSubelement);

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID (const API_Guid& elemGuid, API_ElemTypeID& elementType);

#if defined AC_26 || defined AC_27 || defined AC_28

// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemType elemType, char* elemStr);
#else

// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemTypeID typeID, char* elemStr);
#endif

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition& definision, GS::UniString& name);

// --------------------------------------------------------------------
// Проверка наличия дробной части, возвращает ЛОЖЬ если дробная часть есть
// --------------------------------------------------------------------
bool check_accuracy (double val, double tolerance);

Int32 ceil_mod_classic (Int32 n, Int32 k);

// --------------------------------------------------------------------
// Перевод метров, заданных типом double в мм Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM (const double& value);

// --------------------------------------------------------------------
// Округлить целое n вверх до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod (Int32 n, Int32 k);

// --------------------------------------------------------------------
// Сравнение double c учётом точности
// --------------------------------------------------------------------
bool is_equal (double x, double y);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const std::string& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const GS::UniString& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val);

// --------------------------------------------------------------------
// Перевод строки в число
// --------------------------------------------------------------------
bool UniStringToDouble (const GS::UniString& var, double& x);

// -----------------------------------------------------------------------------
// Замена \n на перенос строки
// -----------------------------------------------------------------------------
void ReplaceCR (GS::UniString& val, bool clear = false);

// -----------------------------------------------------------------------------
// Дополнение строки заданным количеством пробелов или табуляций
// -----------------------------------------------------------------------------
void GetNumSymbSpase (GS::UniString& outstring, GS::UniChar symb, char charrepl);

// -----------------------------------------------------------------------------
// Замена символов \\TAB и др. на юникод
// -----------------------------------------------------------------------------
void ReplaceSymbSpase (GS::UniString& outstring);

short GetFontIndex ();

double GetTextWidth (short& font, double& fontsize, GS::UniString& var);

GS::Array<GS::UniString> DelimTextLine (short& font, double& fontsize, double& width, GS::UniString& var);

// -----------------------------------------------------------------------------
// Проверка статуса и получение ID пользователя Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork (bool& isteamwork, short& userid);

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression (GS::UniString& unistring_expression);

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark (short menuResID, short itemIndex);

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
GS::UniString StringUnic (const GS::UniString& instring, const GS::UniString& delim);

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
UInt32 StringSpltUnic (const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);

GSCharCode GetCharCode (const GS::UniString& instring);

GSCharCode GetCharCode (const GS::UniString& instring, bool& findecode);

bool ProbeCharCode (const GS::UniString& instring, GSCharCode chcode);

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// Записывает в массив только части, содержащие строку filter
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter);

// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead (const API_Element& element, const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters (const API_ElemTypeID& elemType, const API_Guid& elemGuid, API_AddParType**& params);

// --------------------------------------------------------------------
// Получение списка GUID панелей, рам и аксессуаров навесной стены
// --------------------------------------------------------------------
GSErrCode GetRElementsForCWall (const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids);

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing (const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids);

// --------------------------------------------------------------------
// Возвращает координаты заданной точки после трансформации матрицей
// --------------------------------------------------------------------
API_Coord3D GetWordCoord3DTM (const API_Coord3D vtx, const  API_Tranmat& tm);

Point2D GetWordPoint2DTM (const Point2D vtx, const  API_Tranmat& tm);
bool ClickAPoint (const char* prompt, Point2D* c);

namespace FormatStringFunc {
    FormatString GetFormatStringFromFormula (const GS::UniString& formula, const  GS::UniString& part, GS::UniString& stringformat);
    // -----------------------------------------------------------------------------
    // Обработка количества нулей и единиц измерения в имени свойства
    // Удаляет из имени paramName найденные единицы измерения
    // Возвращает строку для скармливания функции NumToStig
    // -----------------------------------------------------------------------------
    GS::UniString GetFormatString (GS::UniString& paramName);

    // -----------------------------------------------------------------------------
    // Возвращает словарь строк-форматов для типов данных согласно настройкам Рабочей среды проекта
    // -----------------------------------------------------------------------------
    FormatStringDict GetFotmatStringForMeasureType ();

    // -----------------------------------------------------------------------------
    // Извлекает из строки информацио о единицах измерении и округлении
    // -----------------------------------------------------------------------------
    FormatString ParseFormatString (const GS::UniString& stringformat);

    // -----------------------------------------------------------------------------
    // Переводит число в строку согласно настройкам строки-формата
    // -----------------------------------------------------------------------------
    GS::UniString NumToString (const double& var, const FormatString& stringformat);

    void ReplaceMeters (GS::UniString& formatstring);

    void ReplaceMeters (GS::UniString& formatstring, Int32& iseng);

}

GSErrCode ConstructPolygon2DFromElementMemo (const API_ElementMemo& memo, Geometry::Polygon2D& poly);

GSErrCode ConvertPolygon2DToAPIPolygon (const Geometry::Polygon2D& polygon, API_Polygon& poly, API_ElementMemo& memo);

bool API_AttributeIndexFindByName (GS::UniString name, const API_AttrTypeID& type, API_AttributeIndex& attribinx);

GSErrCode Favorite_GetNum (const API_ElemTypeID& type, short* count, GS::Array<API_FavoriteFolderHierarchy>* folders, GS::Array<GS::UniString>* names);

API_ElemTypeID GetElemTypeID (const API_Elem_Head& elementhead);

API_ElemTypeID GetElemTypeID (const API_Element& element);

void SetElemTypeID (API_Element& element, const API_ElemTypeID eltype);

void SetElemTypeID (API_Elem_Head& elementhead, const API_ElemTypeID eltype);

namespace GDLHelpers {
    typedef struct {
        GS::Array <double> arr_num;
        GS::Array <GS::UniString> arr_str;
        Int32 dim1 = 1;
        Int32 dim2 = 1;
        double num = 0;
        GS::UniString str = "";
    } Param; // Структура для чтения/записи в объекты

    typedef GS::HashTable <GS::UniString, Param> ParamDict;

    bool ParamToMemo (API_ElementMemo& memo, ParamDict& param);

}

#endif
