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
// Внутри заголовков rapidjson есть нестрогие memcpy — глушим только здесь,
// чтобы clangd не поднимал -Wnontrivial-memcall до ошибки в этом файле.
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnontrivial-memcall"
#include "document.h"
#include "prettywriter.h"
#include "stringbuffer.h"
#pragma clang diagnostic pop

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

// --------------------------------------------------------------------
// Сериализация / десериализация в бинарный канал — ТОЛЬКО для одноразовой
// миграции из старого хранилища (.dat / preferences проекта).
// --------------------------------------------------------------------
GSErrCode SyncSettings::Read (GS::IChannel &ic) {
    GS::InputFrame frame (ic, classInfo);
    ic.Read (syncAll);
    ic.Read (syncMon);
    ic.Read (wallS);
    ic.Read (widoS);
    ic.Read (objS);
    ic.Read (cwallS);
    ic.Read (logMon);
    ic.Read (showpalette);
    ic.Read (catchSelectionChanges);
    ic.Read (maxSelectionCount);
    USize filterPresetCount = 0;
    ic.Read (filterPresetCount);
    filterPresets.Clear ();
    for (UIndex i = 0; i < filterPresetCount; ++i) {
        FilterPreset preset;
        ic.Read (preset.label);
        ic.Read (preset.query);
        filterPresets.Push (preset);
    }
    if (ic.GetInputStatus () == NoError)
        EnsureFilterPresets ();
    return ic.GetInputStatus ();
}

GSErrCode SyncSettings::Write (GS::OChannel &oc) const {
    GS::OutputFrame frame (oc, classInfo);
    oc.Write (syncAll);
    oc.Write (syncMon);
    oc.Write (wallS);
    oc.Write (widoS);
    oc.Write (objS);
    oc.Write (cwallS);
    oc.Write (logMon);
    oc.Write (showpalette);
    oc.Write (catchSelectionChanges);
    oc.Write (maxSelectionCount);
    oc.Write (filterPresets.GetSize ());
    for (const FilterPreset &preset : filterPresets) {
        oc.Write (preset.label);
        oc.Write (preset.query);
    }
    return oc.GetOutputStatus ();
}

// --------------------------------------------------------------------
// Локальное (не проектное) хранилище настроек: JSON.
// ACAPI_SetPreferences пишет блоб аддона В ФАЙЛ ПРОЕКТА (док DevKit-25,
// Level3/Preferences_Save.html: «The preferences data is also stored in all
// project files»), т.е. каждая запись модифицирует БД проекта — в Teamwork это
// давало постоянные локальные изменения («аддон дописывает в файл»).
// Поэтому настройки хранятся в файле в пользовательской папке настроек
// (API_GraphisoftPrefsFolderID), а preferences проекта больше не трогаются.
// Формат — SomeStuffAddonConfig.json (UTF-8) прямо в базовой папке prefs
// (БЕЗ подпапки SomeStuff): чтение по ключам, неизвестные ключи игнорируются,
// отсутствующие — дефолты, поэтому поле version информационное
// (добавление нового поля не отбрасывает файл целиком, как это было в .dat).
// --------------------------------------------------------------------
static const GS::UniString SyncSettingsFileName ("SomeStuffAddonConfig.json");

// Возвращает папку файла настроек: Graphisoft prefs → Application prefs →
// User documents (по убыванию приоритета). Подпапка не создаётся — файл
// SomeStuffAddonConfig.json лежит прямо в базовой папке.
static bool GetSyncSettingsFolderLocation (IO::Location &folderLoc) {
    const API_SpecFolderID folderIds[] = {
        API_GraphisoftPrefsFolderID, API_ApplicationPrefsFolderID, API_UserDocumentsFolderID};
    for (API_SpecFolderID folderId : folderIds) {
        IO::Location baseLoc;
        if (ACAPI_Environment (APIEnv_GetSpecFolderID, &folderId, &baseLoc) != NoError)
            continue;
        if (baseLoc.GetStatus () != NoError)
            continue;
        folderLoc = baseLoc;
        return true;
    }
    // TODO Добавить в вывод в лог через msg_rep вывод ошибки, что папка настроек не найдена
    return false;
}

// --------------------------------------------------------------------
// Утилиты ключей JSON: отсутствующий ключ или несовместимый тип → дефолт.
// --------------------------------------------------------------------
static bool GetJsonBool (const rapidjson::Value &object, const char *key, bool defaultValue) {
    const auto member = object.FindMember (key);
    return (member != object.MemberEnd () && member->value.IsBool ()) ? member->value.GetBool () : defaultValue;
}

static USize GetJsonUSize (const rapidjson::Value &object, const char *key, USize defaultValue) {
    const auto member = object.FindMember (key);
    if (member == object.MemberEnd ())
        return defaultValue;
    if (member->value.IsUint64 ())
        return (USize)member->value.GetUint64 ();
    if (member->value.IsUint ())
        return (USize)member->value.GetUint ();
    return defaultValue;
}

static GS::UniString GetJsonUniString (const rapidjson::Value &object, const char *key) {
    const auto member = object.FindMember (key);
    if (member == object.MemberEnd () || !member->value.IsString ())
        return GS::UniString ();
    // Строки JSON — UTF-8, конвертация по явно указанному коду символов.
    return GS::UniString (member->value.GetString (), member->value.GetStringLength (), CC_UTF8);
}

// --------------------------------------------------------------------
// Чтение настроек из JSON-текста. Читаем во временный экземпляр с дефолтами:
// отсутствующие ключи оставляют значения по умолчанию. Ошибку даёт только
// битый JSON.
// --------------------------------------------------------------------
static bool ReadSyncSettingsFromJsonText (SyncSettings &syncSettings, const std::string &jsonText) {
    rapidjson::Document document;
    if (document.Parse (jsonText.c_str (), jsonText.size ()).HasParseError () || !document.IsObject ())
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

    const auto presetsMember = document.FindMember ("filterPresets");
    if (presetsMember != document.MemberEnd () && presetsMember->value.IsArray ()) {
        GS::Array<FilterPreset> presets;
        for (const auto &entry : presetsMember->value.GetArray ()) {
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
// Файла нет или он битый → false (вызывающая сторона пробует миграцию).
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

    return ReadSyncSettingsFromJsonText (syncSettings, std::string (data.data (), (size_t)fileSize));
}

// --------------------------------------------------------------------
// Сериализация настроек в JSON-текст (UTF-8, человекочитаемое форматирование).
// --------------------------------------------------------------------
static std::string SyncSettingsToJsonString (const SyncSettings &syncSettings) {
    rapidjson::StringBuffer stringBuffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer (stringBuffer);
    writer.StartObject ();

    writer.Key ("version");
    writer.Int (PreferencesVersion);

    auto writeBool = [&writer] (const char *key, bool value) {
        writer.Key (key);
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

    writer.Key ("maxSelectionCount");
    writer.Uint64 ((UInt64)syncSettings.GetMaxSelectionCount ());

    writer.Key ("filterPresets");
    writer.StartArray ();
    for (const FilterPreset &preset : syncSettings.GetFilterPresets ()) {
        writer.StartObject ();
        writer.Key ("label");
        writer.String (preset.label.ToCStr (0, MaxUSize, CC_UTF8).Get ());
        writer.Key ("query");
        writer.String (preset.query.ToCStr (0, MaxUSize, CC_UTF8).Get ());
        writer.EndObject ();
    }
    writer.EndArray ();

    writer.EndObject ();
    return std::string (stringBuffer.GetString (), stringBuffer.GetSize ());
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
// успешной записи (в observer-путях запись вызывается часто, сравнение
// строк дешевле I/O).
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
// Чтение настроек: JSON-файл, при его отсутствии — одноразовая миграция из
// старого бинарного .dat, затем из preferences проекта; мигрированное сразу
// записывается в JSON-файл.
// --------------------------------------------------------------------
static bool ReadSyncSettings (SyncSettings &syncSettings) {
    // Миграция из старых хранилищ (SomeStuff\SyncSettings.json / .dat / prefs
    // проекта) удалена — аддон ещё не используется другими пользователями
    // (решение автора, #190).
    return ReadSyncSettingsFromFile (syncSettings);
}

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
