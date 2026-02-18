//------------ kuvbur 2022 ------------
#pragma once
#if !defined (SYNC_HPP)
#define	SYNC_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#if defined(AC_27) || defined(AC_28)
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include	"APICommon28.h"
#endif // AC_28

#include	"DG.h"
#include	"SyncSettings.hpp"
#include	"Helpers.hpp"


// Тип синхронизации
#define SYNC_NO 0			// Не синхронизировать
#define SYNC_FROM 1			// Взять значение свойства из другого места
#define SYNC_TO 2			// Записать значение свойства в другое место
#define SYNC_TO_SUB 3		// Записать значение свойства в дочерние элементы
#define SYNC_FROM_SUB 4		// Взять значение свойства из дочерних элементов
#define SYNC_FROM_GUID 5	// Взять значение свойства из другого объекта
#define SYNC_FROM_ZONE 6	// Взять значение свойства из Зоны, в которой находится элемент
#define SYNC_TO_ZONE 7		// Записать значение свойства в Зону, в которой находится элемент

// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
struct SyncRule
{
    GS::UniString paramNameFrom = "";
    API_PropertyDefinition paramFrom = {};
    GS::UniString paramNameTo = "";
    API_PropertyDefinition paramTo = {};
    SkipValues ignorevals = {};
    GS::UniString templatestring = "";
    int synctype = 0;
    int syncdirection = 0;
};

struct WriteData
{
    API_Guid guidTo = APINULLGuid;
    API_Guid guidFrom = APINULLGuid;
    ParamValue paramFrom = {};
    ParamValue paramTo = {};
    SkipValues ignorevals = {};
    FormatString formatstring = {}; //Формат строки (задаётся с помощью #mm или #0)
    bool toSub = false;
    bool fromSub = false;
};

const GS::UniString ignorevals_emp = reinterpret_cast<const char*>("empty");
const GS::UniString ignorevals_trim_emp = reinterpret_cast<const char*>("trim_empty");
const GS::UniString ignorevals_def = reinterpret_cast<const char*>("def");

// Словарь с параметрами для записи
typedef GS::HashTable<API_Guid, GS::Array <WriteData>> WriteDict;

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll (SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
void MonByType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll (SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType (const API_ElemTypeID& elementType, const SyncSettings& syncSettings, GS::Int32& nPhase, ParamDictElement& paramToWrite, int dummymode, UnicGuid& syncedelem);

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
bool SyncElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictElement& paramToWrite, int dummymode);

bool SyncElement (const API_Guid& elemGuid, const SyncSettings& syncSettings, ParamDictElement& paramToWrite, int dummymode, UnicGuid& syncedelem);

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
void SyncSelected (const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку переданного массива
// -----------------------------------------------------------------------------
GS::Array<API_Guid> SyncArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray);

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected (const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
void RunParam (const API_Guid& elemGuid, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement (const API_ElemTypeID& elementType, const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
bool SyncData (const API_Guid& elemGuid, const API_Guid& rootGuid, const SyncSettings& syncSettings, GS::Array<API_Guid>& subelemGuids, ParamDictElement& paramToWrite, int dummymode);

bool SyncNeedResync (ParamDictElement& paramToRead, GS::HashTable<API_Guid, GS::UniString> property_write_guid);

void SyncCalcRule (const WriteDict& syncRules, const GS::Array<API_Guid>& subelemGuids, const ParamDictElement& paramToRead, ParamDictElement& paramToWrite, GS::HashTable<API_Guid, GS::UniString>& property_write_guid);

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement (const GS::Array<API_Guid>& subelemGuids, const GS::Array <WriteData>& mainsyncRules, WriteDict& syncRules, ParamDictElement& paramToRead);

// --------------------------------------------------------------------
// Запись правила в словарь правил WriteData, попутно заполняем словарь с параметрами элементов ParamDictElement
// --------------------------------------------------------------------
void SyncAddRule (const WriteData& writeSub, WriteDict& syncRules, ParamDictElement& paramToRead);

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString (const API_Guid& elemGuid, const  API_ElemTypeID& elementType, const API_PropertyDefinition& definition, GS::Array <WriteData>& syncRules, ParamDictElement& paramToRead, bool& hasSub, bool syncall, bool synccoord, bool syncclass, ParamDictValue& subproperty);

bool Name2Rawname (GS::UniString& name, GS::UniString& rawname);

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString (const API_ElemTypeID& elementType, GS::UniString rulestring_one, int& syncdirection, ParamValue& param, SkipValues& ignorevals, FormatString& stringformat, bool syncall, bool synccoord, bool syncclass);

// -----------------------------------------------------------------------------
// Связывает элементы, прописывая в основной элемент GUID привязанных элементов
// -----------------------------------------------------------------------------
void SyncSetSubelement (SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Запись Guid связанных элементов
// Функция для вызова из ACAPI_CallUndoableCommand
// -----------------------------------------------------------------------------
bool SyncSetSubelementScope (const API_Elem_Head& parentelementhead, GS::Array<API_Guid>& subguidArray, ParamDictElement& paramToWrite, const GS::UniString& suffix, const bool& check_guid);

// --------------------------------------------------------------------
// Подсвечивает элементы, GUID которых указан в свойстве с описанием Sync_GUID
// --------------------------------------------------------------------
void SyncShowSubelement (const SyncSettings& syncSettings);

// --------------------------------------------------------------------
// Получение словаря с GUID дочерних объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetParentelement (const GS::Array<API_Guid>& guidArray, UnicGuidByGuid& parentGuid, const GS::UniString& suffix, int& errcode);

// --------------------------------------------------------------------
// Получение словаря с GUID родительских объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetSubelement (const GS::Array<API_Guid>& guidArray, UnicGuidByGuid& parentGuid, const GS::UniString& suffix, int& errcode);

bool SyncGetSyncGUIDProperty (const GS::Array<API_Guid>& guidArray, ParamDictElement& paramToRead, const GS::UniString& suffix);

#endif
