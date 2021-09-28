#if !defined (HELPERS_HPP)
#define	HELPERS_HPP
#include "APICommon.h"
#include "DG.h"
#include "StringConversion.hpp"
#include "ResourceIds.hpp"
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

static const GSResID AddOnInfoID = ID_ADDON_INFO;
static const short AddOnMenuID = ID_ADDON_MENU;
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

static const Int32 UndoSyncId = 1;
static const Int32 SyncAllId = 2;
static const Int32 UndoReNumId = 3;
static const Int32 UndoSumId = 6;

static const Int32 TrueId = 4;
static const Int32 FalseId = 5;

// -----------------------------------------------------------------------------
// Helper functions
// -----------------------------------------------------------------------------
typedef struct {
	Int32		version;
	bool		syncAll;
	bool		syncMon;
	bool		wallS;
	bool		widoS;
	bool		objS;
	bool		logMon;
} SyncPrefs;

int StringSplt(const GS::UniString& instring, const GS::UniString& delim, GS::Array<GS::UniString>& partstring);

bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
void CallOnSelectedElem(void (*function)(const API_Guid&), bool assertIfNoSel = true, bool onlyEditable = true);
GS::Array<API_Guid>	GetSelectedElements(bool assertIfNoSel, bool onlyEditable);
bool	GetElementTypeString(API_ElemTypeID typeID, char* elemStr);
bool GetLibParam(const API_Guid& elemGuid, const GS::UniString& paramName, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
bool MenuInvertItemMark(short menuResID, short itemIndex);
GSErrCode GetPropertyDefinitionByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_PropertyDefinition& definition);
GSErrCode GetTypeByGUID(const API_Guid& elemGuid, API_ElemTypeID& elementType);
void	MenuItemCheckAC(short itemInd, bool checked);
void SyncSettingsGet(SyncPrefs& prefsData);
GSErrCode GetPropertyByName(const API_Guid& elemGuid, const GS::UniString& propertyname, API_Property& property);

GSErrCode WriteProp(const API_Guid& elemGuid, API_Property& property, GS::UniString& param_string, GS::Int32& param_int, bool& param_bool, double& param_real);
GSErrCode WriteProp2Prop(const API_Guid& elemGuid, API_Property& property, const API_Property& propertyfrom);
GSErrCode WriteParam2Prop(const API_Guid& elemGuid, API_Property& property, const GS::UniString& paramName);
void msg_rep(const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid);
void MenuSetState(void);

namespace PropertyTestHelpers
{

GS::UniString			ToString (const API_Variant& variant);

GS::UniString			ToString (const API_Property& property);
}

bool operator== (const API_Variant& lhs, const API_Variant& rhs);

bool operator== (const API_SingleVariant& lhs, const API_SingleVariant& rhs);

bool operator== (const API_ListVariant& lhs, const API_ListVariant& rhs);

bool operator== (const API_SingleEnumerationVariant& lhs, const API_SingleEnumerationVariant& rhs);

#ifndef AC_25
bool operator== (const API_MultipleEnumerationVariant& lhs, const API_MultipleEnumerationVariant& rhs);
#endif // !AC_25

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