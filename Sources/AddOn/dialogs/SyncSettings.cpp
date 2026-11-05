//------------ kuvbur 2022 ------------
#include <cstring>
#include <string>
#include <vector>

#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"

#include "SyncSettings.hpp"

#include "CommonFunction.hpp"
#include "File.hpp"
#include "Folder.hpp"
#include "Location.hpp"
#include "Name.hpp"

// RapidJSON входит в DevKit (Support/Modules/RapidJSON, header-only);
// CMakeCommon.cmake добавляет Modules/* в include path.
// В AC30 GRAPHISOFT переложил заголовки во вложенную папку: rapidjson/document.h
// вместо RapidJSON/document.h. Собственные include у rapidjson относительные,
// поэтому достаточно префикса. Ветка по ServerMainVers_3000, а не по AC_30:
// константа означает «начиная с этой версии» (ACAPinc.h определяет их все разом).
// Внутри заголовков rapidjson есть нестрогие memcpy — глушим только здесь,
// чтобы clangd не поднимал -Wnontrivial-memcall до ошибки в этом файле.
// Под MSVC прагмы clang неизвестны; до AC25 предупреждение C4068 глушилось
// заголовком DevKit (APIdefs_AnalyticalModel.h), в AC26 там его нет и /WX
// превращает C4068 в ошибку сборки.
#ifdef __clang__
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wnontrivial-memcall"
#endif
#ifdef ServerMainVers_3000
    #include "rapidjson/document.h"
    #include "rapidjson/prettywriter.h"
    #include "rapidjson/stringbuffer.h"
#else
    #include "document.h"
    #include "prettywriter.h"
    #include "stringbuffer.h"
#endif
#ifdef __clang__
    #pragma clang diagnostic pop
#endif

static const Int32 PreferencesVersion = 7;

GS::ClassInfo SyncSettings::classInfo ("SyncSettings",
                                       GS::Guid ("B45089A9-B372-460B-B145-80E6EBF107C3"),
                                       GS::ClassVersion (1, 0));

// --------------------------------------------------------------------
// Конструктор и фабричные методы.
// Значения по умолчанию соответствуют типичному поведению add-on:
// связь с объектами, стенами, окнами/дверями и curtain walls включена,
// мониторинг и логирование изначально выключены.
// --------------------------------------------------------------------
SyncSettings::SyncSettings ()
    : syncAll (false), syncMon (true), wallS (true), widoS (true), objS (true), cwallS (true), logMon (false),
      showpalette (false), catchSelectionChanges (false), maxSelectionCount (10),
      filterPresets (CreateDefaultFilterPresets ()) {}

SyncSettings SyncSettings::CreateDefault () { return SyncSettings (); }

SyncSettings SyncSettings::CreateWithSyncAll () {
    SyncSettings settings;
    settings.SetSyncAll (true);
    return settings;
}

bool SyncSettings::GetSyncAll () const { return syncAll; }

void SyncSettings::SetSyncAll (bool value) { syncAll = value; }

bool SyncSettings::GetSyncMon () const { return syncMon; }

void SyncSettings::SetSyncMon (bool value) { syncMon = value; }

bool SyncSettings::GetWallS () const { return wallS; }

void SyncSettings::SetWallS (bool value) { wallS = value; }

bool SyncSettings::GetWidoS () const { return widoS; }

void SyncSettings::SetWidoS (bool value) { widoS = value; }

bool SyncSettings::GetObjS () const { return objS; }

void SyncSettings::SetObjS (bool value) { objS = value; }

bool SyncSettings::GetCwallS () const { return cwallS; }

void SyncSettings::SetCwallS (bool value) { cwallS = value; }

bool SyncSettings::GetLogMon () const { return logMon; }

void SyncSettings::SetLogMon (bool value) { logMon = value; }

bool SyncSettings::GetShowPalette () const { return showpalette; }

void SyncSettings::SetShowPalette (bool value) { showpalette = value; }

bool SyncSettings::GetCatchSelectionChanges () const { return catchSelectionChanges; }

void SyncSettings::SetCatchSelectionChanges (bool value) { catchSelectionChanges = value; }

USize SyncSettings::GetMaxSelectionCount () const { return maxSelectionCount; }

void SyncSettings::SetMaxSelectionCount (USize value) { maxSelectionCount = value; }

const GS::Array<FilterPreset> &SyncSettings::GetFilterPresets () const { return filterPresets; }

void SyncSettings::SetFilterPresets (const GS::Array<FilterPreset> &value) {
    filterPresets = value;
    EnsureFilterPresets ();
}

GS::Array<FilterPreset> SyncSettings::CreateDefaultFilterPresets () {
    GS::Array<FilterPreset> presets;
    presets.Push (FilterPreset{GS::UniString ("Все свойства"), GS::UniString ("")});
    presets.Push (FilterPreset{GS::UniString ("Только IFC"), GS::UniString ("/^IFC:/")});
    presets.Push (FilterPreset{GS::UniString ("Только геометрия"), GS::UniString ("/^(Coord|Geo):/")});
    presets.Push (FilterPreset{GS::UniString ("Незаполненные"), GS::UniString ("/пусто|empty/")});
    return presets;
}

void SyncSettings::EnsureFilterPresets () {
    if (filterPresets.IsEmpty ())
        filterPresets = CreateDefaultFilterPresets ();
}

static const GS::UniString SyncSettingsFileName ("SomeStuffAddonConfig.json");

// --------------------------------------------------------------------
// Возвращает папку файла настроек: Graphisoft prefs → Application prefs →
// User documents (по убыванию приоритета). Подпапка не создаётся — файл
// SomeStuffAddonConfig.json лежит прямо в базовой папке.
// --------------------------------------------------------------------
static bool GetSyncSettingsFolderLocation (IO::Location &folderLoc) {
    const API_SpecFolderID folderIds[] = {
        API_GraphisoftPrefsFolderID, API_ApplicationPrefsFolderID, API_UserDocumentsFolderID};
    for (API_SpecFolderID folderId : folderIds) {
        IO::Location baseLoc;
#ifdef ServerMainVers_2700
        if (ACAPI_ProjectSettings_GetSpecFolder (reinterpret_cast<API_SpecFolderID *> (&folderId),
                                                 reinterpret_cast<IO::Location *> (&baseLoc)) != NoError)
#else
        if (ACAPI_Environment (APIEnv_GetSpecFolderID, &folderId, &baseLoc) != NoError)
#endif
            continue;
        if (baseLoc.GetStatus () != NoError)
            continue;
        folderLoc = baseLoc;
        return true;
    }
    msg_rep ("GetSyncSettingsFolderLocation", "Cant resolve sync settings folder", NoError, APINULLGuid);
    return false;
}

// --------------------------------------------------------------------
// Утилиты ключей JSON: отсутствующий ключ или несовместимый тип → дефолт.
// --------------------------------------------------------------------
// Старый rapidjson из девкита AC23: FindMember приватный, MemberEnd() нет.
// Используем публичные HasMember + operator[] (есть и в новых версиях).
static bool GetJsonBool (const rapidjson::Value &object, const char *key, bool defaultValue) {
    return (object.HasMember (key) && object[key].IsBool ()) ? object[key].GetBool () : defaultValue;
}

static USize GetJsonUSize (const rapidjson::Value &object, const char *key, USize defaultValue) {
    if (!object.HasMember (key))
        return defaultValue;
    const rapidjson::Value &value = object[key];
    if (value.IsUint64 ())
        return (USize)value.GetUint64 ();
    if (value.IsUint ())
        return (USize)value.GetUint ();
    return defaultValue;
}

static GS::UniString GetJsonUniString (const rapidjson::Value &object, const char *key) {
    if (!object.HasMember (key) || !object[key].IsString ())
        return GS::UniString ();
    const rapidjson::Value &value = object[key];
    // Строки JSON — UTF-8, конвертация по явно указанному коду символов.
    return GS::UniString (value.GetString (), value.GetStringLength (), CC_UTF8);
}

// --------------------------------------------------------------------
// Чтение настроек из JSON-текста. Читаем во временный экземпляр с дефолтами:
// отсутствующие ключи оставляют значения по умолчанию. Ошибку даёт только
// битый JSON.
// --------------------------------------------------------------------
static bool ReadSyncSettingsFromJsonText (SyncSettings &syncSettings, const std::string &jsonText) {
    rapidjson::Document document;
    // Старый rapidjson из девкита AC23: Parse принимает только нультерминированную строку.
    if (document.Parse<0> (jsonText.c_str ()).HasParseError () || !document.IsObject ())
        return false;

    SyncSettings tempSettings;
    tempSettings.SetSyncAll (GetJsonBool (document, "syncAll", false));
    tempSettings.SetSyncMon (GetJsonBool (document, "syncMon", true));
    tempSettings.SetWallS (GetJsonBool (document, "wallS", true));
    tempSettings.SetWidoS (GetJsonBool (document, "widoS", true));
    tempSettings.SetObjS (GetJsonBool (document, "objS", true));
    tempSettings.SetCwallS (GetJsonBool (document, "cwallS", true));
    tempSettings.SetLogMon (GetJsonBool (document, "logMon", false));
    tempSettings.SetShowPalette (GetJsonBool (document, "showpalette", false));
    tempSettings.SetCatchSelectionChanges (GetJsonBool (document, "catchSelectionChanges", false));
    tempSettings.SetMaxSelectionCount (GetJsonUSize (document, "maxSelectionCount", 10));

    if (document.HasMember ("filterPresets") && document["filterPresets"].IsArray ()) {
        GS::Array<FilterPreset> presets;
        // Старый rapidjson не имеет GetArray — обходим через Begin/End.
        const rapidjson::Value &presetsJson = document["filterPresets"];
        for (rapidjson::Value::ConstValueIterator it = presetsJson.Begin (); it != presetsJson.End (); ++it) {
            const rapidjson::Value &entry = *it;
            if (!entry.IsObject ())
                continue;
            presets.Push (FilterPreset{GetJsonUniString (entry, "label"), GetJsonUniString (entry, "query")});
        }
        if (!presets.IsEmpty ())
            tempSettings.SetFilterPresets (presets);
    }

    syncSettings = tempSettings;
    return true;
}

// --------------------------------------------------------------------
// Чтение настроек из локального JSON-файла.
// Файла нет или он битый → false (вызывающая сторона использует дефолты).
// --------------------------------------------------------------------
static bool ReadSyncSettingsFromFile (SyncSettings &syncSettings) {
    IO::Location folderLoc;
    if (!GetSyncSettingsFolderLocation (folderLoc))
        return false;

    const IO::Location fileLoc (folderLoc, IO::Name (SyncSettingsFileName));
    IO::File file (fileLoc);
    if (file.GetStatus () != NoError || file.Open (IO::File::ReadMode) != NoError)
        return false;

    UInt64 fileSize = 0;
    if (file.GetDataLength (&fileSize) != NoError || fileSize == 0) {
        file.Close ();
        return false;
    }

    std::vector<char> data ((size_t)fileSize);
    const GSErrCode readErr = file.ReadBin (data.data (), (USize)fileSize);
    file.Close ();
    if (readErr != NoError)
        return false;

    if (!ReadSyncSettingsFromJsonText (syncSettings, std::string (data.data (), (size_t)fileSize)))
        return false;

    // Путь к файлу настроек логируем один раз за сессию
    static bool pathLogged = false;
    if (!pathLogged && IsTestProjectOpen ()) {
        msg_rep ("ReadSyncSettingsFromFile", "Settings file: " + fileLoc.ToDisplayText (), NoError, APINULLGuid);
        pathLogged = true;
    }
    return true;
}

// --------------------------------------------------------------------
// Сериализация настроек в JSON-текст (UTF-8, человекочитаемое форматирование).
// --------------------------------------------------------------------
// Старый rapidjson из девкита AC23 не имеет Key() — ключ пишется как String(..., copy = true).
static void WriteJsonKey (rapidjson::PrettyWriter<rapidjson::StringBuffer> &writer, const char *key) {
#if !defined(ServerMainVers_2500)
    writer.String (key, static_cast<rapidjson::SizeType> (std::strlen (key)), true);
#else
    writer.Key (key);
#endif
}

static std::string SyncSettingsToJsonString (const SyncSettings &syncSettings) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer (stringBuffer);
    writer.StartObject ();

    WriteJsonKey (writer, "version");
    writer.Int (PreferencesVersion);

    auto writeBool = [&writer] (const char *key, bool value) {
        WriteJsonKey (writer, key);
        writer.Bool (value);
    };
    writeBool ("syncAll", syncSettings.GetSyncAll ());
    writeBool ("syncMon", syncSettings.GetSyncMon ());
    writeBool ("wallS", syncSettings.GetWallS ());
    writeBool ("widoS", syncSettings.GetWidoS ());
    writeBool ("objS", syncSettings.GetObjS ());
    writeBool ("cwallS", syncSettings.GetCwallS ());
    writeBool ("logMon", syncSettings.GetLogMon ());
    writeBool ("showpalette", syncSettings.GetShowPalette ());
    writeBool ("catchSelectionChanges", syncSettings.GetCatchSelectionChanges ());

    WriteJsonKey (writer, "maxSelectionCount");
    writer.Uint64 ((UInt64)syncSettings.GetMaxSelectionCount ());

    WriteJsonKey (writer, "filterPresets");
    writer.StartArray ();
    for (const FilterPreset &preset : syncSettings.GetFilterPresets ()) {
        writer.StartObject ();
        WriteJsonKey (writer, "label");
        writer.String (preset.label.ToCStr (0, MaxUSize, CC_UTF8).Get ());
        WriteJsonKey (writer, "query");
        writer.String (preset.query.ToCStr (0, MaxUSize, CC_UTF8).Get ());
        writer.EndObject ();
    }
    writer.EndArray ();

    writer.EndObject ();
    return std::string (stringBuffer.GetString ());
}

// --------------------------------------------------------------------
// Запись уже сериализованного JSON-текста в локальный файл.
// --------------------------------------------------------------------
static bool WriteSyncSettingsFile (const std::string &jsonText) {
    if (jsonText.empty ())
        return false;

    IO::Location folderLoc;
    if (!GetSyncSettingsFolderLocation (folderLoc)) {
        msg_rep ("WriteSyncSettingsFile", "Cant resolve sync settings folder", NoError, APINULLGuid);
        return false;
    }

    const IO::Location fileLoc (folderLoc, IO::Name (SyncSettingsFileName));
    IO::File file (fileLoc, IO::File::OnNotFound::Create);
    if (file.Open (IO::File::WriteEmptyMode) != NoError) {
        msg_rep ("WriteSyncSettingsFile", "Cant open " + fileLoc.ToDisplayText (), NoError, APINULLGuid);
        return false;
    }

    GSErrCode err = file.WriteBin (jsonText.c_str (), (USize)jsonText.size ());
    const GSErrCode closeErr = file.Close ();
    if (err != NoError || closeErr != NoError) {
        msg_rep ("WriteSyncSettingsFile", "Cant write " + fileLoc.ToDisplayText (), err, APINULLGuid);
        return false;
    }

    DBprnt ("SyncSettings: saved to " + fileLoc.ToDisplayText ());
    return true;
}

// --------------------------------------------------------------------
// Сериализация настроек и запись их в локальный файл.
// skipIfUnchanged — не писать, если JSON-текст не изменился с прошлой
// успешной записи
// --------------------------------------------------------------------
static bool WriteSyncSettingsToFile (const SyncSettings &syncSettings, bool skipIfUnchanged) {
    const std::string jsonText = SyncSettingsToJsonString (syncSettings);
    if (jsonText.empty ())
        return false;

    static std::string lastWritten;
    if (skipIfUnchanged && lastWritten == jsonText)
        return true;

    if (!WriteSyncSettingsFile (jsonText))
        return false;

    lastWritten = jsonText;
    return true;
}

// --------------------------------------------------------------------
// Чтение настроек: только SomeStuffAddonConfig.json из корня prefs;
// --------------------------------------------------------------------
static bool ReadSyncSettings (SyncSettings &syncSettings) { return ReadSyncSettingsFromFile (syncSettings); }

// --------------------------------------------------------------------
// Кэш настроек
// Кэширует экземпляр SyncSettings в статической области. При forceReload
// текущие настройки заново загружаются из локального JSON-файла.
// --------------------------------------------------------------------
SyncSettings &GetSyncSettingsCache (bool forceReload) {
    static SyncSettings instance;
    static bool loaded = false;

    if (!loaded || forceReload) {
        ReadSyncSettings (instance);
        loaded = true;
    }
    return instance;
}

bool LoadSyncSettingsFromPreferences (SyncSettings &syncSettings, bool forceReload) {
    // Загружаем настройки из кэша. Если forceReload == true, кэш обновится.
    syncSettings = GetSyncSettingsCache (forceReload);
    return true;
}

bool WriteSyncSettingsToPreferences (const SyncSettings &syncSettings) {
    // Пишем локальный JSON-файл (не preferences проекта!) и только если
    // настройки действительно изменились.
    if (!WriteSyncSettingsToFile (syncSettings, true))
        return false;
    // Обновляем кэш только после успешной записи.
    GetSyncSettingsCache (false) = syncSettings;
    return true;
}

// --------------------------------------------------------------------
// Чистим только старые настройки preferences, не данные элементов.
// Проверка размера через _Platform не требует разбора межплатформенного
// блоба. Пустую запись проверяем чтением; сохранение и повторное
// открытие PLN отдельно проверяются в Archicad.
// --------------------------------------------------------------------
GSErrCode ClearLegacyProjectPreferences () {
#ifdef ServerMainVers_2700
    const auto readPreferences = ACAPI_Preferences_Platform;
#else
    const auto readPreferences = ACAPI_GetPreferences_Platform;
#endif
    Int32 version = 0;
    GSSize nByte = 0;
    GSErrCode err = readPreferences (&version, &nByte, nullptr, nullptr);
    if (err != NoError || nByte == 0)
        return err;
    err = ACAPI_SetPreferences (0, 0, nullptr);
    if (err != NoError)
        return err;
    err = readPreferences (&version, &nByte, nullptr, nullptr);
    if (err != NoError)
        return err;
    return nByte == 0 ? NoError : APIERR_GENERAL;
}
