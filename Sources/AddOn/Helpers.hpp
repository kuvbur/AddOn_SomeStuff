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
#include	"calculator.hpp"

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
	API_VariantType type;
	GS::UniString name = "";
	GS::UniString uniStringValue = "";
	GS::Int32 intValue = 0;
	bool boolValue = false;
	double doubleValue = 0.0;
	bool canCalculate = false;
} ParamValue;

typedef struct {
	GS::Array <API_Property> prop;
	GS::Array <ParamValue> param;
} WriteData;

// Словарь с заранее вычисленными данными для калькулятора
typedef GS::HashTable<GS::UniString, ParamValue> ParamDictValue;

// Словарь с параметрами для вычисления
typedef GS::HashTable<GS::UniString, bool> ParamDict;

// Словарь с параметрами для записи
typedef GS::HashTable<API_Guid, WriteData> WriteDict;

bool is_equal(double x, double y);

bool CheckIgnoreVal(const std::string& ignoreval, const GS::UniString& val);

bool CheckIgnoreVal(const GS::UniString& ignoreval, const GS::UniString& val);

bool CheckIgnoreVal(const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val);

Int32 DoubleM2IntMM(const double& value);

Int32 ceil_mod(Int32 n, Int32 k);

GSErrCode IsTeamwork(bool& isteamwork, short& userid);

GSErrCode AttachObserver(const API_Guid& objectId, const SyncSettings& syncSettings);

bool SyncCheckElementType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

bool IsElementEditable(const API_Guid& objectId, const SyncSettings& syncSettings, const bool needCheckElementType);

bool ReserveElement(const API_Guid& objectId, GSErrCode& err);

GSErrCode WriteProp2Param(const API_Guid& elemGuid, GS::UniString paramName, API_Property& property);

UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter);
GSErrCode GetCWElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& panelSymbolGuids);
GSErrCode GetCWElementsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& panelSymbolGuids);
GSErrCode GetGDLParametersHead(const API_Elem_Head elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);
bool FindGDLParametersByName(const GS::UniString& paramName, API_AddParType**& params, Int32& inx);
bool FindGDLParametersByDescription(const GS::UniString& paramName, const API_Elem_Head elem_head, Int32& inx);
bool FindGDLParameters(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);
GSErrCode GetGDLParameters(const API_Guid& elemGuid, const API_ElemTypeID& elemType, API_AddParType**& params);
GSErrCode GetGDLParameters(const API_Elem_Head elem_head, API_AddParType**& params);
bool FindLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);
bool GetParam(const API_Guid& elemGuid, const GS::UniString& paramName, ParamValue& pvalue);

bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, ParamValue& pvalue);

bool GetPropertyParam(const API_Guid& elemGuid, const GS::UniString& paramName, ParamValue& pvalue);

bool ConvParamValue(ParamValue& pvalue, const API_AddParType& nthParameter);
bool ConvParamValue(ParamValue& pvalue, const API_Property& property);
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const Int32 intValue);
bool ConvParamValue(ParamValue& pvalue, const GS::UniString& paramName, const double doubleValue);

void AddParam2Dict(const GS::UniString& paramName, const double doubleValue, ParamDictValue& pdictvalue);

GS::UniString GetFormatString(GS::UniString& paramName);
API_VariantType GetTypeString(GS::UniString& paramName);

bool GetParamNameDict(const GS::UniString& expression, ParamDict& paramDict);
bool GetParamValueDict(const API_Guid& elemGuid, const ParamDict& paramDict, ParamDictValue& pdictvalue);

bool ReplaceParamInExpression(const ParamDictValue& pdictvalue, GS::UniString& expression);
bool EvalExpression(GS::UniString& expression);

void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel = true, bool onlyEditable = true);
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel, bool onlyEditable);
void CallOnSelectedElemSettings(void(*function)(const API_Guid&, const SyncSettings&), bool assertIfNoSel, bool onlyEditable, const SyncSettings& syncSettings);
#ifndef AC_26
bool GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
#endif
bool MenuInvertItemMark(short menuResID, short itemIndex);
GSErrCode GetPropertyDefinitionByName(const GS::UniString& propertyname, API_PropertyDefinition& definition);
GSErrCode GetIFCPropertyByName(const API_Guid& elemGuid, const GS::UniString& tpropertyname, API_IFCProperty& property);
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_PropertyDefinition& definition);
GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name);
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType);
bool GetElementTypeString(API_ElemType elemType, char* elemStr);
void MenuItemCheckAC(short itemInd, bool checked);
GSErrCode GetMorphParam(const API_Guid& elemGuid, ParamDictValue& pdictvalue);
GSErrCode GetPropertyByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_Property& property);
GSErrCode GetGDLParameters(const API_Guid& elemGuid, const API_ElemTypeID& elemType, API_AddParType**& params);
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string);
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, ParamValue& pvalue);
bool IsEqualPropParamValue(const ParamValue& pvalue, API_Property& property);
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, const GS::UniString& paramName, API_Property& property);
GSErrCode WriteProp2Prop(const API_Guid& elemGuid, const API_Property& propertyfrom, API_Property& property);
void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid);
void MenuSetState(SyncSettings& syncSettings);

namespace PropertyTestHelpers
{
	GS::UniString NumToString(const double& var, const GS::UniString stringformat);
	GS::UniString ToString(const API_Variant& variant, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Variant& variant);
	GS::UniString ToString(const API_Property& property, const GS::UniString stringformat);
	GS::UniString	ToString(const API_Property& property);
}

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

void DeleteElementUserData(const API_Guid& elemguid);

void DeleteElementsUserData();

void UnhideUnlockAllLayer(void);

template <typename T>
bool operator!= (const T& lhs, const T& rhs)
{
	return !(lhs == rhs);
}

#endif