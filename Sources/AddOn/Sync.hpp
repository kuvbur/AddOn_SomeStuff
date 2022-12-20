//------------ kuvbur 2022 ------------
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"SyncSettings.hpp"

// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
typedef struct {
	GS::UniString paramName = "";
	GS::Array<GS::UniString> ignorevals;
	GS::UniString templatestring = "";
	int synctype = 0;
	int syncdirection = 0;
} SyncRule;

// --------------------------------------------------------------------
// Структура для хранения данных ос составе конструкций
// Заполнение см. SyncPropAndMatGetComponents
// --------------------------------------------------------------------
typedef struct {
	API_Attribute					buildingMaterial;
	GS::Array<API_PropertyDefinition>	definitions;
	GS::UniString						templatestring = "";
	double								fillThick = 0.0;
} LayerConstr;

bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

void SyncAndMonAll(SyncSettings& syncSettings);

void SyncSelected(const SyncSettings& syncSettings);

void SyncElement(const API_Guid& objectId, const SyncSettings& syncSettings);

void SyncRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings);

void SyncGetRelationsElement(const API_Guid& elemGuid, GS::Array<API_Guid>& subelemGuid);

void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings);

GSErrCode SyncOneProperty(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition);

bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID& elementType, const API_PropertyDefinition& definition, SyncRule syncRule);

bool SyncString(GS::UniString& description_string, GS::Array<SyncRule>& syncRules);

bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions, GS::UniString property_flag_name);

GSErrCode SyncPropAndProp(const API_Guid& elemGuid_from, const API_Guid& elemGuid_to, const SyncRule& syncRule, const API_PropertyDefinition& definition);

GSErrCode SyncMorphAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, const API_PropertyDefinition& definition);

GSErrCode SyncIFCAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, const API_PropertyDefinition& definition);

GSErrCode SyncParamAndProp(const API_Guid& elemGuid_from, const API_Guid& elemGuid_to, SyncRule& syncRule, const API_PropertyDefinition& definition);

GSErrCode SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

GSErrCode SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components);

void SyncPropAndMatReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring);

GSErrCode SyncPropAndMatWriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n);

GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, const API_PropertyDefinition& definition);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_IFCProperty& property);

#endif