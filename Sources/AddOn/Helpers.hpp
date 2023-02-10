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
#define	 Menu_RunParam		12

#define	 SYNC_RESET	1
#define	 SYNC	2

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
static const Int32 RunParam_CommandID = 12;

static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 6;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;
static const Int32 ErrorSelectID = 6;
static const Int32 UndoDimRound = 7;

typedef struct {
	GS::Array <API_Guid>	guid;
} SortGUID;

typedef struct {
	GS::Array <UInt32>	inx;
} SortInx;

// --------------------------------------------------------------------
// Структура для хранения данных о составе конструкций
// Заполнение см. SyncPropAndMatGetComponents
// --------------------------------------------------------------------
typedef struct {
	API_Attribute					buildingMaterial;
	GS::Array<API_PropertyDefinition>	definitions;
	GS::UniString						templatestring = "";
	double								fillThick = 0.0;
} LayerConstr;

// Хранение данных параметра
// type - API_VariantType (как у свойств)
// name - имя для поиска
// uniStringValue, intValue, boolValue, doubleValue - значения
// canCalculate - можно ли использовать в математических вычислениях
typedef struct {

	// Собственно значения
	GS::UniString uniStringValue = "";
	GS::Int32 intValue = 0;
	bool boolValue = false;
	double doubleValue = 0.0;
	bool canCalculate = false; // Может быть использован в формулах
} ParamValueData;

// Все данные - из свойств, из GDL параметров и т.д. хранятся в структуре ParamValue
// Это позволяет свободно конвертировать и записывать данные в любое место
typedef struct {
	API_VariantType type = API_PropertyUndefinedValueType;
	GS::UniString rawName = ""; // Имя для сопоставления в словаре - с указанием откуда взято
	GS::UniString name = ""; //Очищенное имя для поиска
	ParamValueData val = {};
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

// --------------------------------------------------------------------
// Сравнение double c учётом точности
// --------------------------------------------------------------------
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
GSErrCode AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings);

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
void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname);

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString& funcname /* = ""*/);

void CallOnSelectedElemSettings(void (*function)(const API_Guid&, const SyncSettings&, ParamDictValue& propertyParams, ParamDictElement& paramToWrite), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, const SyncSettings& syncSettings, GS::UniString& funcname);

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
GSErrCode GetRElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& elementsSymbolGuids);

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing(const API_Guid& elemGuid, GS::Array<API_Guid>& elementsGuids);

// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead(const API_Element& element, const API_Elem_Head& elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters(const API_ElemTypeID& elemType, const API_Guid& elemGuid, API_AddParType**& params);

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
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName(GS::UniString& name);

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression(GS::UniString& unistring_expression);

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark(short menuResID, short itemIndex);

namespace PropertyHelpers {
	GS::UniString	NumToString(const double& var, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Variant& variant, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Variant& variant);
	GS::UniString	ToString(const API_Property& property, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Property& property);
}

// -----------------------------------------------------------------------------
// Функции для работы с ParamDict и ParamValue
// -----------------------------------------------------------------------------
namespace ParamHelpers {

	// -----------------------------------------------------------------------------
	// Получение размеров Морфа
	// Формирует словарь ParamDictValue& pdictvalue со значениями
	// -----------------------------------------------------------------------------
	bool GetMorphParam(const API_Element& element, ParamDictValue& pdictvalue);

	// -----------------------------------------------------------------------------
	// Получение координат объекта
	// symb_pos_x , symb_pos_y, symb_pos_z
	// Для панелей навесной стены возвращает центр панели
	// Для колонны или объекта - центр колонны и отм. низа
	// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
	// -----------------------------------------------------------------------------
	bool GetLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);

	// -----------------------------------------------------------------------------
	// Замена имен параметров на значения в выражении
	// Значения передаются словарём, вычисление значений см. GetParamValueDict
	// -----------------------------------------------------------------------------
	bool ReplaceParamInExpression(const ParamDictValue& pdictvalue, GS::UniString& expression);

	// -----------------------------------------------------------------------------
	// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
	// -----------------------------------------------------------------------------
	bool ParseParamName(const GS::UniString& expression, ParamDictValue& paramDict);

	// -----------------------------------------------------------------------------
	// Добавление значения в словарь ParamDictValue
	// -----------------------------------------------------------------------------
	void AddVal(ParamDictValue& params, const GS::UniString& rawName_prefix, const GS::UniString& name, const double& val);

	// -----------------------------------------------------------------------------
	// Список возможных префиксов типов параметров
	// -----------------------------------------------------------------------------
	void GetParamTypeList(GS::Array<GS::UniString>& paramTypesList);

	// -----------------------------------------------------------------------------
	// Конвертация значений ParamValue в свойства, находящиеся в нём
	// Возвращает true если значения отличались
	// -----------------------------------------------------------------------------
	bool ToProperty(ParamValue& pvalue);

	// -----------------------------------------------------------------------------
	// Синхронизация ParamValue и API_Property
	// Возвращает true и подготовленное для записи свойство в случае отличий
	// TODO Переписать всё под запись ParamValue
	// -----------------------------------------------------------------------------
	bool ToProperty(const ParamValue& pvalue, API_Property& property);

	// --------------------------------------------------------------------
	// Сопоставление двух словарей ParamDictValue
	// --------------------------------------------------------------------
	void Compare(const ParamDictValue& paramsFrom, ParamDictValue& paramsTo);

	// --------------------------------------------------------------------
	// Чтение значений свойств в ParamDictValue
	// --------------------------------------------------------------------
	bool GetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

	// -----------------------------------------------------------------------------
	// Получение значения IFC свойств в ParamDictValue
	// -----------------------------------------------------------------------------
	bool GetIFCValues(const API_Guid& elemGuid, ParamDictValue& params);

	// -----------------------------------------------------------------------------
	// Получить значение GDL параметра по его имени или описанию в ParamValue
	// -----------------------------------------------------------------------------
	bool GetGDLValues(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

	// --------------------------------------------------------------------
	// Запись словаря параметров для множества элементов
	// --------------------------------------------------------------------
	void ElementsWrite(ParamDictElement& paramToWrite);

	// --------------------------------------------------------------------
	// Запись параметров в один элемент
	// --------------------------------------------------------------------
	void Write(const API_Guid& elemGuid, ParamDictValue& params);

	// --------------------------------------------------------------------
	// Запись ParamDictValue в свойства
	// --------------------------------------------------------------------
	void SetPropertyValues(const API_Guid& elemGuid, ParamDictValue& params);

	// --------------------------------------------------------------------
	// Заполнение словаря параметров для множества элементов
	// --------------------------------------------------------------------
	void ElementsRead(ParamDictElement& paramToRead, ParamDictValue& propertyParams);

	// --------------------------------------------------------------------
	// Заполнение словаря с параметрами
	// --------------------------------------------------------------------
	void Read(const API_Guid& elemGuid, ParamDictValue& params);

	// -----------------------------------------------------------------------------
	// Конвертация параметров библиотечного элемента в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const API_AddParType& nthParameter);

	// -----------------------------------------------------------------------------
	// Конвертация свойства в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const API_Property& property);

	// -----------------------------------------------------------------------------
	// Конвертация определения свойства в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const API_PropertyDefinition& definition);

	// -----------------------------------------------------------------------------
	// Конвертация целого числа в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const GS::UniString& paramName, const Int32 intValue);

	// -----------------------------------------------------------------------------
	// Конвертация double в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const GS::UniString& paramName, const double doubleValue);

	// -----------------------------------------------------------------------------
	// Конвертация API_IFCProperty в ParamValue
	// -----------------------------------------------------------------------------
	bool ConvValue(ParamValue& pvalue, const API_IFCProperty& property);

	void GetAllPropertyDefinitionToParamDict(ParamDictValue& propertyParams, GS::Array<API_PropertyDefinition>& definitions);

	// --------------------------------------------------------------------
	// Получить все доступные свойства в формарте ParamDictValue
	// --------------------------------------------------------------------
	void GetAllPropertyDefinitionToParamDict(ParamDictValue& propertyParams);

	// -----------------------------------------------------------------------------
	// Поиск по описанию GDL параметра
	// Данный способ работат только с объектами (только чтение)
	// -----------------------------------------------------------------------------
	bool GDLParamByDescription(const API_Element& element, ParamDictValue& params);

	// -----------------------------------------------------------------------------
	// Поиск по имени GDL параметра (чтение/запись)
	// -----------------------------------------------------------------------------
	bool GDLParamByName(const API_Element& element, const API_Elem_Head& elem_head, ParamDictValue& params);

	// -----------------------------------------------------------------------------
	// Перевод значения в строку в соответсвии с stringformat
	// -----------------------------------------------------------------------------
	GS::UniString ToString(const ParamValue& pvalue, const GS::UniString stringformat);
}

// -----------------------------------------------------------------------------
// Функции для получения состава конструкции в текстовом виде
// -----------------------------------------------------------------------------
namespace MaterialString {

	// -----------------------------------------------------------------------------
	// ОСНОВНАЯ ФУНКЦИЯ
	// Получание строки с составом конструкции (слоями)
	// -----------------------------------------------------------------------------
	bool GetComponentString(const API_Element& element, ParamValue& pvalue);

	// --------------------------------------------------------------------
	// Вытаскивает всё, что может, из информации о составе элемента
	// --------------------------------------------------------------------
	GSErrCode GetComponents(const API_Element& element, GS::Array<LayerConstr>& components);

	// --------------------------------------------------------------------
	// Заполнение данных для одного слоя
	// --------------------------------------------------------------------
	GSErrCode  GetOneComponent(const API_AttributeIndex& constrinx, const double& fillThick, LayerConstr& component);

	// -----------------------------------------------------------------------------
	// Ищем в строке - шаблоне свойства и возвращаем массив определений
	// Строка шаблона на входе
	//			%Имя свойства% текст %Имя группы/Имя свойства.5mm%
	// Строка шаблона на выходе
	//			@1@ текст @2@#.5mm#
	// Если свойство не найдено, %Имя свойства% заменяем на пустоту ("")
	// -----------------------------------------------------------------------------
	GSErrCode  ParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

	// --------------------------------------------------------------------
	// Заменяем в строке templatestring все вхождения @1@...@n@ на значения свойств
	// --------------------------------------------------------------------
	GSErrCode  WriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n);

	void ReplaceValue(const double& var, const GS::UniString& patternstring, GS::UniString& outstring);

	void ReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring);
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
bool operator!= (const T& lhs, const T& rhs) {
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

//--------------------------------------------------------------------------------------------------------------------------
//Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name);

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

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name);

#endif