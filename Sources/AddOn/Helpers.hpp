#pragma once
#if !defined (HELPERS_HPP)
#define	HELPERS_HPP
#include	"APICommon.h"
#include	"DG.h"
#include	"StringConversion.hpp"
#include	"ResourceIds.hpp"

#define ELEMSTR_LEN				256
#define	CURR_ADDON_VERS			0x0006
#define	 Menu_MonAll		1
#define	 Menu_SyncAll		2
#define	 Menu_SyncSelect	3
#define	 Menu_wallS			4
#define	 Menu_widoS			5
#define	 Menu_objS			6
#define	 Menu_ReNum			7
#define	 Menu_Sum			8
#define	 Menu_Log			9
#define	 Menu_LogShow		10

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
static const Int32 ReNum_CommandID = 7;
static const Int32 Sum_CommandID = 8;
static const Int32 Log_CommandID = 9;
static const Int32 LogShow_CommandID = 10;


static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 6;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;
static const Int32 ErrorSelectID = 6;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
typedef struct {
	Int32	version;
	bool	syncAll;
	bool	syncMon;
	bool	wallS;
	bool	widoS;
	bool	objS;
	bool	logMon;
} SyncPrefs;

typedef struct {
	GS::Array <API_Guid>	guid;
} SortGUID;

typedef struct {
	GS::Array <UInt32>	inx;
} SortInx;

//typedef GS::HashTable<GS::UniString, API_PropertyDefinition> Prop;

bool is_equal(double x, double y);

bool CheckIgnoreVal(const std::string& ignoreval, const GS::UniString& val);

bool CheckIgnoreVal(const GS::UniString& ignoreval, const GS::UniString& val);

bool CheckIgnoreVal(const GS::Array<GS::UniString>& ignorevals, const GS::UniString& val);

Int32 DoubleM2IntMM(const double& value);

GSErrCode IsTeamwork(bool& isteamwork, short& userid);

GSErrCode AttachObserver(const API_Guid& objectId);
bool IsElementEditable(const API_Guid& objectId);
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);
UInt32 StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring, const GS::UniString& filter);
GSErrCode GetCWPanelsForCWall(const API_Guid& cwGuid, GS::Array<API_Guid>& panelSymbolGuids);
GSErrCode GetGDLParametersHead(const API_Elem_Head elem_head, API_ElemTypeID& elemType, API_Guid& elemGuid);
bool FindGDLParametersByName(const GS::UniString& paramName, API_AddParType**& params, Int32& inx);
bool FindGDLParametersByDescription(const GS::UniString& paramName, const API_Elem_Head elem_head, Int32& inx);
bool FindGDLParameters(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);
GSErrCode GetGDLParameters(const API_Elem_Head elem_head, API_AddParType**& params);
bool FindLibCoords(const GS::UniString& paramName, const API_Elem_Head& elem_head, API_AddParType& nthParameter);
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel = true, bool onlyEditable = true);
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel, bool onlyEditable);
bool GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
bool MenuInvertItemMark(short menuResID, short itemIndex);
GSErrCode GetPropertyDefinitionByName(const GS::UniString& propertyname, API_PropertyDefinition& definition);
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_PropertyDefinition& definition);
GSErrCode GetPropertyFullName(const API_PropertyDefinition& definision, GS::UniString& name);
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType);
void MenuItemCheckAC(short itemInd, bool checked);
void SyncSettingsGet(SyncPrefs& prefsData);
GSErrCode GetPropertyByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_Property& property);
GSErrCode GetGDLParameters(const API_Guid& elemGuid, const API_ElemTypeID& elemType, API_AddParType**& params);
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string);
GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, const GS::UniString& paramName, API_Property& property);
GSErrCode WriteProp2Prop(const API_Guid& elemGuid, const API_Property& propertyfrom, API_Property& property);
void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid);
void MenuSetState(void);

namespace PropertyTestHelpers
{
	GS::UniString NumToString(const double& var, const GS::UniString stringformat);
	GS::UniString ToString(const API_Variant& variant, const GS::UniString stringformat);
	GS::UniString	ToString (const API_Variant& variant);
	GS::UniString ToString(const API_Property& property, const GS::UniString stringformat);
GS::UniString	ToString (const API_Property& property);
}

bool operator== (const API_Variant& lhs, const API_Variant& rhs);

bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs);

bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs);

bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs);

#if defined(AC_24)
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs);
#endif

bool Equals (const API_PropertyDefaultValue& lhs, const API_PropertyDefaultValue& rhs, API_PropertyCollectionType collType);

bool Equals (const API_PropertyValue& lhs, const API_PropertyValue& rhs, API_PropertyCollectionType collType);

bool operator== (const API_PropertyGroup& lhs, const API_PropertyGroup& rhs);

bool operator== (const API_PropertyDefinition& lhs, const API_PropertyDefinition& rhs);

bool operator== (const API_Property& lhs, const API_Property& rhs);

template <typename T>
bool operator!= (const T& lhs, const T& rhs)
{
	return !(lhs == rhs);
}

#endif