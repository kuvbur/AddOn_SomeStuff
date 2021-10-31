
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#include	"APICommon.h"
#include	"DG.h"

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
	UInt32								fillThick;
} LayerConstr;

void SyncAndMonAll(void);

bool SyncByType(const API_ElemTypeID& elementType);

void SyncSelected(void);

GSErrCode SyncRelationsToWindow(const API_Guid& elemGuid);

GSErrCode SyncRelationsToDoor(const API_Guid& elemGuid);

void SyncRelationsElement(const API_Guid& elemGuid);

void SyncData(const API_Guid& elemGuid);

GSErrCode SyncOneProperty(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_PropertyDefinition definition);

bool SyncOneRule(const API_Guid& elemGuid, const API_ElemTypeID elementType, API_Property property, SyncRule syncRule);

bool SyncString(GS::UniString& description_string, GS::Array<SyncRule>& syncRules);

bool SyncState(const API_Guid& elemGuid, const GS::Array<API_PropertyDefinition> definitions);

GSErrCode SyncPropAndProp(const API_Guid& elemGuid, const SyncRule& syncRule, API_Property& property);

GSErrCode SyncParamAndProp(const API_Guid& elemGuid, SyncRule& syncRule, API_Property& property);

GSErrCode SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

GSErrCode SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components);

GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, API_Property property);

bool CheckElementType(const API_ElemTypeID& elementType);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);

bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);


#endif