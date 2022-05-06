
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#include	"APICommon.h"
#include	"DG.h"
#include	"SyncSettings.hpp"

// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
typedef struct {
	GS::UniString paramName;
	GS::Array<GS::UniString> ignorevals;
	GS::UniString templatestring;
	int synctype;
	int syncdirection;
} SyncRule;

// --------------------------------------------------------------------
// Структура для хранения данных ос составе конструкций
// Заполнение см. SyncPropAndMatGetComponents
// --------------------------------------------------------------------
typedef struct {
	API_Attribute					buildingMaterial;
	GS::Array<API_PropertyDefinition>	definitions;
	GS::UniString						templatestring;
	double								fillThick;
} LayerConstr;

bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

void SyncAndMonAll(const SyncSettings& syncSettings);

void SyncSelected(const SyncSettings& syncSettings);

GSErrCode SyncRelationsToWindow(const API_Guid& elemGuid, const SyncSettings& syncSettings);

GSErrCode SyncRelationsToDoor(const API_Guid& elemGuid, const SyncSettings& syncSettings);

void SyncRelationsElement(const API_Guid& elemGuid, const SyncSettings& syncSettings);

void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings);

GSErrCode SyncOneProperty(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition);

bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_Property property, SyncRule syncRule);

bool SyncString(GS::UniString& description_string, GS::Array<SyncRule>& syncRules);

bool SyncResetState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions, API_PropertyDefinition& definition_to_reset);

bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions);

GSErrCode SyncPropAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, API_Property& property);

GSErrCode SyncIFCAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, API_Property& property);

GSErrCode SyncParamAndProp(const API_Guid& elemGuid, SyncRule& syncRule, API_Property& property);

GSErrCode SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

GSErrCode SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components);

void SyncPropAndMatReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring);

GSErrCode SyncPropAndMatWriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n);

GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, API_Property property);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_IFCProperty& property);

#endif