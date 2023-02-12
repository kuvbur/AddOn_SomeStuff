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

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
void MonByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll(SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType(const API_ElemTypeID& elementType, const SyncSettings& syncSettings, short& nPhase, ParamDictValue& propertyParams);

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
void SyncElement(const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictValue& propertyParams, ParamDictElement& paramToWrite);

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected(const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected(const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam(const API_Guid& elemGuid, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement(const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
void SyncData(const API_Guid& elemGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids, ParamDictValue& propertyParams, ParamDictElement& paramToWrite);

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement(const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись правила в словарь правил WriteData, попутно заполняем словарь с параметрами элементов ParamDictElement
// --------------------------------------------------------------------
void SyncAddRule(const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void SyncAddParam(const ParamValue& param, ParamDictElement& paramToRead);

void SyncAddParam(ParamDictValue& params, const API_Guid& elemGuid, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString(const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, ParamDictElement& paramToRead, bool& hasSub);

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString(const API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, GS::Array<GS::UniString>& ignorevals, GS::UniString& stringformat);

#endif