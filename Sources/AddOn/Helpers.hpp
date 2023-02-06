//------------ kuvbur 2022 ------------
#pragma once

#ifndef HELPERS_HPP
#define	HELPERS_HPP

#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"StringConversion.hpp"
#include	"ResourceIds.hpp"
#include	"SyncSettings.hpp"
#include	"exprtk.hpp"

#define ELEMSTR_LEN				256
#define	CURR_ADDON_VERS			0x0006
#define	 Menu_MonAll		1
#define	 Menu_SyncAll		2
#define	 Menu_SyncSelect	3
#define	 Menu_wallS			4
#define	 Menu_widoS			5
#define	 Menu_objS			6
#define	 Menu_cwallS		7
#define	 Menu_ReNum			8
#define	 Menu_Sum			9
#define	 Menu_Log			10
#define	 Menu_LogShow		11

#define SYNC_GDL 1
#define SYNC_PROPERTY 2
#define SYNC_MATERIAL 3
#define SYNC_INFO 4
#define SYNC_IFC 5
#define SYNC_MORPH 6
#define SYNC_CLASS 7

static const GSResID AddOnInfoID = ID_ADDON_INFO;
static const short AddOnMenuID = ID_ADDON_MENU;
static const short AddOnPromtID = ID_ADDON_PROMT;

static const GSResID AddOnStringsID = ID_ADDON_STRINGS;

static const Int32 AddOnNameID = 1;
static const Int32 AddOnDescriptionID = 2;

static const Int32 MonAll_CommandID = 1;
static const Int32 SyncAll_CommandID = 2;
static const Int32 SyncSelect_CommandID = 3;
static const Int32 wallS_CommandID = 4;
static const Int32 widoS_CommandID = 5;
static const Int32 objS_CommandID = 6;
static const Int32 cwallS_CommandID = 7;
static const Int32 ReNum_CommandID = 8;
static const Int32 Sum_CommandID = 9;
static const Int32 Log_CommandID = 10;
static const Int32 LogShow_CommandID = 11;

static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 6;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;
static const Int32 ErrorSelectID = 6;

static const Int32 UndoDimRound = 7;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------

typedef struct {
	GS::Array <API_Guid>	guid;
} SortGUID;

typedef struct {
	GS::Array <UInt32>	inx;
} SortInx;

// Хранение данных параметра
// type - API_VariantType (как у свойств)
// name - имя для поиска
// uniStringValue, intValue, boolValue, doubleValue - значения
// canCalculate - можно ли использовать в математических вычислениях
typedef struct {
	API_VariantType type = API_PropertyUndefinedValueType;
	GS::UniString rawName = ""; // Имя для сопоставления в словаре - с указанием откуда взято
	GS::UniString name = ""; //Очищенное имя для поиска
	// Собственно значения
	GS::UniString uniStringValue = "";
	GS::Int32 intValue = 0;
	bool boolValue = false;
	double doubleValue = 0.0;
	// -----------------
	bool canCalculate = false; // Может быть использован в формулах
	bool isValid = false; // Валидность (был считан без ошибок)
	API_PropertyDefinition definition = {}; // Описание свойства, для упрощения чтения/записи
	API_Property property = {}; // Само свойство, для упрощения чтения/записи
	// Тут храним способ, которым нужно получить значение
	bool fromGDLparam = false; // Найден в гдл параметрах
	bool fromGDLdescription = false; // Найден по описанию
	bool fromProperty = false; // Найден в свойствах
	bool fromMorph = false; // Найден свойствах морфа
	bool fromInfo = false; // Найден в инфо о проекте
	bool fromIFCProperty = false;
	bool fromCoord = false; //Координаты
	bool fromPropertyDefinition = false; //Задан определением, искать не нужно
	bool fromMaterial = false; // Взять инфо из состава конструкции
	API_Guid fromGuid = APINULLGuid; //Откуда прочитать
} ParamValue;

// Словарь с заранее вычисленными данными в пределах обного элемента
typedef GS::HashTable<GS::UniString, ParamValue> ParamDictValue;

// Словарь с параметрами для вычисления
// Служит для формирования уникального списка свойств и параметров
typedef GS::HashTable<GS::UniString, bool> ParamDict;

// Словарь с параметрами для элементов
typedef GS::HashTable<API_Guid, ParamDictValue> ParamDictElement;

bool is_equal(double x, double y);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal(const std::string& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal(const GS::UniString& ignoreval, const GS::UniString& val);

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal(const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val);

// --------------------------------------------------------------------
// Перевод метров, заданных типом double в мм Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM(const double& value);

// --------------------------------------------------------------------
// Округлить целое n вверх до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod(Int32 n, Int32 k);


// -----------------------------------------------------------------------------
// Проверка статуса и получение ID пользователя Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork(bool& isteamwork, short& userid);

// -----------------------------------------------------------------------------
// Добавление отслеживания (для разных версий)
// -----------------------------------------------------------------------------
GSErrCode	AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable(const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType);

// -----------------------------------------------------------------------------
// Резервируем, разблокируем, вообщем - делаем элемент редактируемым
// Единственное, что может нас остановить - объект находится в модуле.
// -----------------------------------------------------------------------------
bool ReserveElement(const API_Guid& objectId, GSErrCode& err);

// -----------------------------------------------------------------------------
// Обновление отмеченных в меню пунктов
// -----------------------------------------------------------------------------
void MenuSetState(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Вывод сообщения в отчёт
// -----------------------------------------------------------------------------
void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// -----------------------------------------------------------------------------
void MenuItemCheckAC(short itemInd, bool checked);

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/);

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid SyncSettings
// -----------------------------------------------------------------------------
void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/);

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType);

#ifdef AC_26
bool GetElementTypeString(API_ElemType elemType, char* elemStr);
#else
bool GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
#endif

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuid);

// -----------------------------------------------------------------------------
// Запись значения свойства в параметры объекта
// Пока записывает только GLOB_ID
// -----------------------------------------------------------------------------
GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property);

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// Записывает в массив только части, содержащие строку filter
// -----------------------------------------------------------------------------
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter);

// --------------------------------------------------------------------
// Получение списка GUID панелей, рам и аксессуаров навесной стены
// --------------------------------------------------------------------
GSErrCode GetCWElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids);

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing(const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids);

// -----------------------------------------------------------------------------
// Получение значения IFC свойства по имени свойства
// -----------------------------------------------------------------------------
GSErrCode GetIFCPropertyByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_IFCProperty& property);

// -----------------------------------------------------------------------------
// Получение размеров Морфа
// Формирует словарь ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
GSErrCode GetMorphParam(const API_Element& element, ParamDictValue& pdictvalue);

// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead(const API_Element& element, const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_ElemTypeID& elemType, const API_Guid& elemGuid, API_AddParType**& params);

// -----------------------------------------------------------------------------
// Получение координат объекта
// Level 3
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool FindLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);

// -----------------------------------------------------------------------------
// Обработка количества нулей и единиц измерения в имени свойства
// Удаляет из имени paramName найденные единицы измерения
// Возвращает строку для скармливания функции NumToStig
// -----------------------------------------------------------------------------
GS::UniString GetFormatString(GS::UniString& paramName);

// -----------------------------------------------------------------------------
// Обработка типа данных в имени
// $ - вернуть строку
// # - вернуть целое число
// По умолчанию - double
// Удаляет из имени paramName найденные указания на тип данных
// -----------------------------------------------------------------------------
API_VariantType GetTypeString(GS::UniString& paramName);

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool GetParamNameDict(const GS::UniString& expression, ParamDict& paramDict);

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ReplaceParamInExpression(const ParamDictValue& pdictvalue, GS::UniString& expression);

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression(GS::UniString& unistring_expression);

// -----------------------------------------------------------------------------
// Список возможных префиксов типов параметров
// -----------------------------------------------------------------------------
void GetParamTypeList(GS::Array<GS::UniString>& paramTypesList);

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark(short menuResID, short itemIndex);

namespace PropertyTestHelpers
{
	GS::UniString NumToString(const double& var, const GS::UniString stringformat);
	GS::UniString ToString(const API_Variant& variant, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Variant& variant);
	GS::UniString ToString(const API_Property& property, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Property& property);
}

bool operator== (const ParamValue& lhs, const ParamValue& rhs);

bool operator== (const API_Variant& lhs, const API_Variant& rhs);

bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs);

bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs);

bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs);

#if defined(AC_24)
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs);
#endif

bool Equals(const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType);

bool Equals(const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType);

bool operator== (const API_PropertyGroup& lhs, const API_PropertyGroup& rhs);

bool operator== (const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs);

bool operator== (const API_Property& lhs, const API_Property& rhs);

template <typename T>
bool operator!= (const T& lhs, const T& rhs)
{
	return !(lhs == rhs);
}

// -----------------------------------------------------------------------------
// Удаление данных аддона из элемента
// -----------------------------------------------------------------------------
void DeleteElementUserData(const API_Guid& elemguid);

// -----------------------------------------------------------------------------
// Удаление данных аддона из всех элементов
// -----------------------------------------------------------------------------
void DeleteElementsUserData();

void UnhideUnlockAllLayer(void);

// -----------------------------------------------------------------------------
// Конвертация значений ParamValue в свойства, находящиеся в нём
// Возвращает true если значения отличались
// -----------------------------------------------------------------------------
bool ParamValueToProperty(ParamValue& pvalue);


// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// TODO Переписать всё под запись ParamValue
// -----------------------------------------------------------------------------
bool ParamValueToProperty(const ParamValue& pvalue, API_Property& property);

//--------------------------------------------------------------------------------------------------------------------------
//Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name);

// --------------------------------------------------------------------
// Запись словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamDictElementWrite(ParamDictElement& paramToWrite);

// --------------------------------------------------------------------
// Запись параметров в один элемент
// --------------------------------------------------------------------
void ParamDictWrite(const API_Guid& elemGuid, ParamDictValue& params);

void ParamDictSetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamDictElementRead(ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Заполнение словаря с параметрами
// --------------------------------------------------------------------
void ParamDictRead(const API_Guid& elemGuid, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const GS::UniString& propertyname, API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// Получение определения свойства по имени свойства дляф заданного элемента
// Формат имени ГРУППА/ИМЯ_СВОЙСТВА
// -----------------------------------------------------------------------------
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_PropertyDefinition& definition);

// --------------------------------------------------------------------
// Получить все доступные свойства в формарте ParamDictValue
// --------------------------------------------------------------------
void GetAllPropertyDefinitionToParamDict(ParamDictValue& propertyParams);

// --------------------------------------------------------------------
// Пакетный поиск определений свойств
// --------------------------------------------------------------------
void ParamDictGetPropertyDefinition(ParamDictValue& params);

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamDictCompare(const ParamDictValue& paramsFrom, ParamDictValue& paramsTo);

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ParamDictGetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ParamDictGetGDLValues(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ не работает с элементами навесных стен
// -----------------------------------------------------------------------------
bool FindGDLParamByDescription(const API_Element& element, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Поиск по имени GDL параметра
// -----------------------------------------------------------------------------
bool FindGDLParamByName(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name);

// -----------------------------------------------------------------------------
// Конвертация параметров библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_AddParType& nthParameter);

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_Property& property);

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const Int32 intValue);

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const double doubleValue);

#endif