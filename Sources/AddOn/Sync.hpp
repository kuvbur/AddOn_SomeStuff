//------------ kuvbur 2022 ------------
#pragma once
#if !defined(SYNC_HPP)
    #define SYNC_HPP
    #include "DG.h"
    #include "dialogs/SyncSettings.hpp"
    #include "Helpers.hpp"

// --------------------------------------------------------------------
// Структура для хранения одного правила
// Заполнение см. SyncString
// --------------------------------------------------------------------
struct SyncRule {
    // Имя исходного параметра или свойства.
    GS::UniString paramNameFrom;
    // Описание исходного свойства для синхронизации.
    API_PropertyDefinition paramFrom = {};
    // Имя целевого параметра или свойства.
    GS::UniString paramNameTo;
    // Описание целевого свойства для синхронизации.
    API_PropertyDefinition paramTo = {};
    // Набор значений, которые следует игнорировать.
    SkipValues ignorevals = {};
    // Шаблон строки форматирования для преобразования значения.
    GS::UniString templatestring;
    // Тип выполняемой синхронизации.
    SyncMode synctype = SYNC_NO;
    // Направление синхронизации.
    SyncMode syncdirection = SYNC_NO;
};

struct WriteData {
    // GUID целевого элемента для записи.
    API_Guid guidTo = APINULLGuid;
    // GUID исходного элемента для чтения.
    API_Guid guidFrom = APINULLGuid;
    // Значение параметра, полученное из исходного источника.
    ParamValue paramFrom = {};
    // Значение параметра, которое будет записано в целевой объект.
    ParamValue paramTo = {};
    // Значения, которые нужно пропустить при записи.
    SkipValues ignorevals = {};
    // Формат строки для преобразования значения (например, #mm или #0).
    FormatString formatstring = {};
    // Признак записи в дочерние элементы.
    bool toSub = false;
    // Признак чтения из дочерних элементов.
    bool fromSub = false;
};

// Специальное значение для игнорирования пустых строк.
const GS::UniString ignorevals_emp = reinterpret_cast<const char *> ("empty");
// Специальное значение для игнорирования пустых строк после обрезки.
const GS::UniString ignorevals_trim_emp = reinterpret_cast<const char *> ("trim_empty");
// Специальное значение для игнорирования значений по умолчанию.
const GS::UniString ignorevals_def = reinterpret_cast<const char *> ("def");

// Словарь с параметрами для записи
typedef GS::HashTable<API_Guid, GS::Array<WriteData>> WriteDict;

// Ограничивает повторную синхронизацию одного GUID в коротком каскаде уведомлений.
bool IsElementThrottled (const API_Guid &guid);
// Независимо ограничивает полный обход размеров после EndEvents.
bool IsDimensionScanThrottled ();
// Сбрасывает метки при смене проекта или отключении наблюдения.
void ClearSyncThrottleCache ();

// -----------------------------------------------------------------------------
// Подключение мониторинга
// -----------------------------------------------------------------------------
void MonAll (SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Подключение мониторинга по типам
// -----------------------------------------------------------------------------
bool MonByType (const API_ElemTypeID &elementType, const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку всех объектов, заданных в настройке
// -----------------------------------------------------------------------------
void SyncAndMonAll (SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Синхронизация элементов по типу
// -----------------------------------------------------------------------------
bool SyncByType (const API_ElemTypeID &elementType,
                 const SyncSettings &syncSettings,
                 GS::Int32 &nPhase,
                 ParamDictElement &paramToWrite,
                 int dummymode,
                 UnicGuid &syncedelem);

// -----------------------------------------------------------------------------
// Синхронизация элемента и его подэлементов
// -----------------------------------------------------------------------------
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
void SyncSelected (const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запускает обработку переданного массива
// -----------------------------------------------------------------------------
GS::Array<API_Guid> SyncArray (const SyncSettings &syncSettings, GS::Array<API_Guid> &guidArray);

// -----------------------------------------------------------------------------
// Запуск скрипта параметров выбранных элементов
// -----------------------------------------------------------------------------
void RunParamSelected (const SyncSettings &syncSettings);

// -----------------------------------------------------------------------------
// Запуск скрипта параметра элемента
// -----------------------------------------------------------------------------
// Запускает выполнение параметрических правил для одного элемента.
void RunParam (const API_Guid &elemGuid, const SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Поиск и синхронизация свойств связанных элементов
// --------------------------------------------------------------------
bool SyncRelationsElement (const API_ElemTypeID &elementType, const SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Синхронизация данных элемента согласно указаниям в описании свойств
// --------------------------------------------------------------------
bool SyncData (const API_Guid &elemGuid,
               const SyncSettings &syncSettings,
               GS::Array<API_Guid> &subelemGuids,
               ParamDictElement &paramToWrite,
               int dummymode);

bool SyncNeedResync (ParamDictElement &paramToRead, const UnicGuidString &property_write_guid);

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
                 SyncMode &syncdirection,
                 ParamValue &param,
                 SkipValues &ignorevals,
                 FormatString &stringformat,
                 bool syncall,
                 bool synccoord,
                 bool syncclass,
                 bool checkElementType = true);

// -----------------------------------------------------------------------------
// Связывает элементы, прописывая в основной элемент GUID привязанных элементов
// -----------------------------------------------------------------------------
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
void SyncShowSubelement (const SyncSettings &syncSettings, bool show_ui = false);

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
