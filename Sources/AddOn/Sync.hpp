//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SYNC_HPP)
    #define SYNC_HPP
    #include "DG.h"
    #include "Helpers.hpp"
    #include "SyncSettings.hpp"

static const GS::UniString PROPERTYPREF = "Property:";
static const GS::UniString MORPHPREF = "Morph:";
static const GS::UniString COORDPREF = "Coord:";
static const GS::UniString INFOPREF = "Info:";
static const GS::UniString IFCPREF = "IFC:";
static const GS::UniString GLOBPREF = "Glob:";
static const GS::UniString CLASSPREF = "Class:";
static const GS::UniString ATTRIBPREF = "Attribute:";
static const GS::UniString ELEMENTPREF = "Element:";
static const GS::UniString MEPPREF = "MEP:";
static const GS::UniString FILEPREF = "File:";
static const GS::UniString LISTDATAPREF = "Listdata:";
static const GS::UniString QRPREF = "QRCode:";
static const GS::UniString MATERIALPREF = "Material:";
static const GS::UniString FROMGUIDBR = "from_GUID{";
static const GS::UniString FROMGUID = "from_GUID";
static const GS::UniString TOGUIDBR = "to_GUID{";
static const GS::UniString TOGUID = "to_GUID";
static const GS::UniString SYNCPART = "Sync_";
static const GS::UniString SYNCFROMSTRING = "from{";
static const GS::UniString SYNCFROMSUBSTRING = "from_sub{";
static const GS::UniString SYNCTOSTRING = "to{";
static const GS::UniString SYNCTOSUBSTRING = "to_sub{";

// Модуль синхронизации свойств и связанных элементов между различными источниками данных.
// Тип синхронизации
    #define SYNC_NO 0        // Не синхронизировать
    #define SYNC_FROM 1      // Взять значение свойства из другого места
    #define SYNC_TO 2        // Записать значение свойства в другое место
    #define SYNC_TO_SUB 3    // Записать значение свойства в дочерние элементы
    #define SYNC_FROM_SUB 4  // Взять значение свойства из дочерних элементов
    #define SYNC_FROM_GUID 5 // Взять значение свойства из другого объекта
    #define SYNC_FROM_ZONE 6 // Взять значение свойства из Зоны, в которой находится элемент
    #define SYNC_TO_ZONE 7   // Записать значение свойства в Зону, в которой находится элемент

// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
struct SyncRule {
    GS::UniString paramNameFrom = "";
    API_PropertyDefinition paramFrom = {};
    GS::UniString paramNameTo = "";
    API_PropertyDefinition paramTo = {};
    SkipValues ignorevals = {};
    GS::UniString templatestring = "";
    int synctype = 0;
    int syncdirection = 0;
};

struct WriteData {
    API_Guid guidTo = APINULLGuid;
    API_Guid guidFrom = APINULLGuid;
    ParamValue paramFrom = {};
    ParamValue paramTo = {};
    SkipValues ignorevals = {};
    FormatString formatstring = {}; // Формат строки (задаётся с помощью #mm или #0)
    bool toSub = false;
    bool fromSub = false;
};

const GS::UniString ignorevals_emp = reinterpret_cast<const char *> ("empty");
const GS::UniString ignorevals_trim_emp = reinterpret_cast<const char *> ("trim_empty");
const GS::UniString ignorevals_def = reinterpret_cast<const char *> ("def");

// Словарь с параметрами для записи
typedef GS::HashTable<API_Guid, GS::Array<WriteData>> WriteDict;

// Проверяет, не находится ли элемент в текущем throttled-кэше и не нужно ли пропустить его обработку.
bool IsElementThrottled (const API_Guid &guid);

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
// Подключает мониторинг изменений для всех активных типов элементов.
void MonAll (SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
// Подключает мониторинг только для заданного типа элементов.
bool MonByType (const API_ElemTypeID &elementType, const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
// Запускает полную синхронизацию и подключает мониторинг для активных элементов.
void SyncAndMonAll (SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
// Синхронизирует элементы выбранного типа и собирает список уже обработанных GUID.
bool SyncByType (const API_ElemTypeID &elementType,
                 const SyncSettings &syncSettings,
                 GS::Int32 &nPhase,
                 ParamDictElement &paramToWrite,
                 int dummymode,
                 UnicGuid &syncedelem);

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
// Синхронизирует один элемент и его подэлементы согласно правилам add-on.
bool SyncElement (const API_Guid &elemGuid,
                  const SyncSettings &syncSettings,
                  ParamDictElement &paramToWrite,
                  int dummymode);

bool SyncElement (const API_Guid &elemGuid,
                  const SyncSettings &syncSettings,
                  ParamDictElement &paramToWrite,
                  int dummymode,
                  UnicGuid &syncedelem);

// -----------------------------------------------------------------------------
// Запускает обработку выбранных, заданных в настройке
// -----------------------------------------------------------------------------
// Запускает синхронизацию только для выбранных элементов.
void SyncSelected (const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку переданного массива
// -----------------------------------------------------------------------------
// Синхронизирует переданный массив элементов и возвращает список обработанных GUID.
GS::Array<API_Guid> SyncArray (const SyncSettings &syncSettings, GS::Array<API_Guid> &guidArray);

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
// Запускает выполнение параметрических правил для выбранных элементов.
void RunParamSelected (const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
// Запускает выполнение параметрических правил для одного элемента.
void RunParam (const API_Guid &elemGuid, const SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
// Синхронизирует свойства связанных элементов для указанного типа.
bool SyncRelationsElement (const API_ElemTypeID &elementType, const SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
// Применяет правила синхронизации к элементу и его связанным подэлементам.
bool SyncData (const API_Guid &elemGuid,
               const SyncSettings &syncSettings,
               GS::Array<API_Guid> &subelemGuids,
               ParamDictElement &paramToWrite,
               int dummymode);

bool SyncNeedResync (ParamDictElement &paramToRead, UnicGuidString property_write_guid);

void SyncCalcRule (const WriteDict &syncRules,
                   const GS::Array<API_Guid> &subelemGuids,
                   const ParamDictElement &paramToRead,
                   ParamDictElement &paramToWrite,
                   UnicGuidString &property_write_guid);

// --------------------------------------------------------------------
// Добавление подэлементов и их параметров в правила синхорнизации
// --------------------------------------------------------------------
void SyncAddSubelement (const GS::Array<API_Guid> &subelemGuids,
                        GS::Array<WriteData> &mainsyncRules,
                        WriteDict &syncRules,
                        ParamDictElement &paramToRead);

// --------------------------------------------------------------------
// Запись правила в словарь правил WriteData, попутно заполняем словарь с параметрами элементов ParamDictElement
// --------------------------------------------------------------------
void SyncAddRule (const WriteData &writeSub, WriteDict &syncRules, ParamDictElement &paramToRead);

// -----------------------------------------------------------------------------
// Парсит описание свойства, заполняет массив с правилами (GS::Array <WriteData>)
// -----------------------------------------------------------------------------
bool ParseSyncString (const API_Guid &elemGuid,
                      const API_ElemTypeID &elementType,
                      const API_PropertyDefinition &definition,
                      GS::Array<WriteData> &syncRules,
                      ParamDictElement &paramToRead,
                      bool &hasSub,
                      bool syncall,
                      bool synccoord,
                      bool syncclass,
                      ParamDictValue &subproperty);

bool Name2Rawname (GS::UniString &name, GS::UniString &rawname);

// -----------------------------------------------------------------------------
// Парсит описание свойства
// -----------------------------------------------------------------------------
bool SyncString (const API_ElemTypeID &elementType,
                 GS::UniString rulestring_one,
                 int &syncdirection,
                 ParamValue &param,
                 SkipValues &ignorevals,
                 FormatString &stringformat,
                 bool syncall,
                 bool synccoord,
                 bool syncclass);

// -----------------------------------------------------------------------------
// Связывает элементы, прописывая в основной элемент GUID привязанных элементов
// -----------------------------------------------------------------------------
// Связывает элементы с дочерними подэлементами, записывая GUID в соответствующие свойства.
void SyncSetSubelement (SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запись Guid связанных элементов
// Функция для вызова из ACAPI_CallUndoableCommand
// -----------------------------------------------------------------------------
bool SyncSetSubelementScope (const API_Elem_Head &parentelementhead,
                             const GS::Array<API_Guid> &subguidArray,
                             ParamDictElement &paramToWrite,
                             const GS::UniString &suffix,
                             const bool &check_guid);

// --------------------------------------------------------------------
// Подсвечивает элементы, GUID которых указан в свойстве с описанием Sync_GUID
// --------------------------------------------------------------------
// Подсвечивает элементы, связанные через свойства Sync_GUID.
void SyncShowSubelement (const SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Получение словаря с GUID дочерних объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetParentelement (const GS::Array<API_Guid> &guidArray,
                           UnicGuidByGuid &parentGuid,
                           const GS::UniString &suffix,
                           int &errcode);

// --------------------------------------------------------------------
// Получение словаря с GUID родительских объектов для массива объектов
// --------------------------------------------------------------------
bool SyncGetSubelement (const GS::Array<API_Guid> &guidArray,
                        UnicGuidByGuid &parentGuid,
                        const GS::UniString &suffix,
                        int &errcode);

bool SyncGetSyncGUIDProperty (const GS::Array<API_Guid> &guidArray,
                              ParamDictElement &paramToRead,
                              const GS::UniString &suffix);

#endif
