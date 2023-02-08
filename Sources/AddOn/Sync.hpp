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
#include	"Helpers.hpp"
// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
typedef struct {
	GS::UniString paramNameFrom = "";
	API_PropertyDefinition paramFrom = {};
	GS::UniString paramNameTo = "";
	API_PropertyDefinition paramTo = {};
	GS::Array<GS::UniString> ignorevals;
	GS::UniString templatestring = "";
	int synctype = 0;
	int syncdirection = 0;
} SyncRule;

typedef struct {
	API_Guid guidTo = APINULLGuid;
	API_Guid guidFrom = APINULLGuid;
	ParamValue paramFrom;
	ParamValue paramTo;
	GS::Array<GS::UniString> ignorevals;
	GS::UniString stringformat = ""; //Формат строки (задаётся с помощью #mm или #0)
	bool toSub = false;
	bool fromSub = false;
} WriteData;

// Словарь с параметрами для записи
typedef GS::HashTable<API_Guid, GS::Array <WriteData>> WriteDict;

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


// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку всех элементов заданного типа
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings, short& nPhase);

void SyncElement(const API_Guid& elemGuid, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings);

void RunParamSelected(const SyncSettings& syncSettings);

void RunParam(const API_Guid& elemGuid, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids);

void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись правила в словарь
// --------------------------------------------------------------------
void SyncAddRule(const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись параметра в словарь
// --------------------------------------------------------------------
void SyncAddParam(const ParamValue& param, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, bool& hasSub);

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString(const API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, GS::UniString& stringformat);

// -----------------------------------------------------------------------------
// Ищем в строке - шаблоне свойства и возвращаем массив определений
// Строка шаблона на входе
//			%Имя свойства% текст %Имя группы/Имя свойства.5mm%
// Строка шаблона на выходе
//			@1@ текст @2@#.5mm#
// Если свойство не найдено, %Имя свойства% заменяем на пустоту ("")
// -----------------------------------------------------------------------------
GSErrCode  SyncPropAndMatParseString(const GS::UniString& templatestring, GS::UniString& outstring, GS::Array<API_PropertyDefinition>& outdefinitions);

GSErrCode  SyncPropAndMatOneGetComponent(const API_AttributeIndex& constrinx, const double& fillThick, LayerConstr& component);

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatGetComponents(const API_Guid& elemGuid, GS::Array<LayerConstr>& components);

void SyncPropAndMatReplaceValue(const double& var, const GS::UniString& patternstring, GS::UniString& outstring);

void SyncPropAndMatReplaceValue(const API_Property& property, const GS::UniString& patternstring, GS::UniString& outstring);

// --------------------------------------------------------------------
// Заменяем в строке templatestring все вхождения @1@...@n@ на значения свойств
// --------------------------------------------------------------------
GSErrCode  SyncPropAndMatWriteOneString(const API_Attribute& attrib, const double& fillThick, const GS::Array<API_PropertyDefinition>& outdefinitions, const GS::UniString& templatestring, GS::UniString& outstring, UInt32& n);

// -----------------------------------------------------------------------------
// Запись в свойство данных о материале
// -----------------------------------------------------------------------------
GSErrCode SyncPropAndMat(const API_Guid& elemGuid, const API_ElemTypeID elementType, const SyncRule syncRule, const API_PropertyDefinition& definition);

// -----------------------------------------------------------------------------
// Проверяем - содержит ли строка игнорируемые значения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const GS::UniString& val);

// -----------------------------------------------------------------------------
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_Property& property);

// -----------------------------------------------------------------------------
// Проверяем - содержит ли свойство игнорируемые значеения
// -----------------------------------------------------------------------------
bool SyncCheckIgnoreVal(const SyncRule& syncRule, const API_IFCProperty& property);


#endif
