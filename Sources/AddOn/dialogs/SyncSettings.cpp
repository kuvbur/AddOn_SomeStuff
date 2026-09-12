//------------ kuvbur 2022 ------------
#include <cstring>
#include <vector>

#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"

#include "SyncSettings.hpp"

#include "CommonFunction.hpp"
#include "File.hpp"
#include "Folder.hpp"
#include "Location.hpp"
#include "Name.hpp"

static const Int32 PreferencesVersion = 5;

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
    : syncAll (false), syncMon (false), wallS (true), widoS (true), objS (true), cwallS (true), logMon (false),
      showpalette (false), catchSelectionChanges (true), maxSelectionCount (10) {}

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

// --------------------------------------------------------------------
// Сериализация / десериализация.
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
    return oc.GetOutputStatus ();
}

// --------------------------------------------------------------------
// Локальное (не проектное) хранилище настроек.
// ACAPI_SetPreferences пишет блоб аддона В ФАЙЛ ПРОЕКТА (док DevKit-25,
// Level3/Preferences_Save.html: «The preferences data is also stored in all
// project files»), т.е. каждая запись модифицирует БД проекта — в Teamwork это
// давало постоянные локальные изменения («аддон дописывает в файл»).
// Поэтому настройки хранятся в файле в пользовательской папке настроек
// (API_GraphisoftPrefsFolderID), а preferences проекта больше не трогаются.
// --------------------------------------------------------------------
static const UInt32 SyncSettingsFileMagic = 0x53535331; // 'SSS1'
static const GS::UniString SyncSettingsFolderName ("SomeStuff");
static const GS::UniString SyncSettingsFileName ("SyncSettings.dat");
// Заголовок пишется/читается полями через канал — размер одинаков на всех
// платформах (без #pragma pack и выравнивания структуры).
static const USize SyncSettingsHeaderSize = sizeof (UInt32) + sizeof (Int32) + sizeof (UInt64);

// Возвращает папку файла настроек: Graphisoft prefs → Application prefs →
// User documents (по убыванию приоритета) + подпапка SomeStuff.
static bool GetSyncSettingsFolderLocation (IO::Location &folderLoc) {
    const API_SpecFolderID folderIds[] = {
        API_GraphisoftPrefsFolderID, API_ApplicationPrefsFolderID, API_UserDocumentsFolderID};
    for (API_SpecFolderID folderId : folderIds) {
        IO::Location baseLoc;
        if (ACAPI_Environment (APIEnv_GetSpecFolderID, &folderId, &baseLoc) != NoError)
            continue;
        IO::Folder folder (baseLoc);
        const GSErrCode createErr = folder.CreateFolder (IO::Name (SyncSettingsFolderName));
        // TargetExists — папка уже создана предыдущими запусками, это не ошибка
        if (createErr != NoError && createErr != IO::Folder::TargetExists)
            continue;
        IO::Location candidate (baseLoc, IO::Name (SyncSettingsFolderName));
        if (candidate.GetStatus () != NoError)
            continue;
        folderLoc = candidate;
        return true;
    }
    return false;
}

// --------------------------------------------------------------------
// Чтение настроек из локального файла. Локального файла нет/он битый/версия
// не совпадает → false (вызывающая сторона решает, что делать).
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
    if (file.GetDataLength (&fileSize) != NoError || fileSize <= SyncSettingsHeaderSize) {
        file.Close ();
        return false;
    }

    std::vector<char> data ((size_t)fileSize);
    const GSErrCode readErr = file.ReadBin (data.data (), (USize)fileSize);
    file.Close ();
    if (readErr != NoError)
        return false;

    MemoryIChannel inputChannel (data.data (), (USize)fileSize);
    UInt32 magic = 0;
    Int32 version = 0;
    UInt64 blobSize = 0;
    // Заголовок: magic + версия + размер блоба — отсекает чужие/битые файлы
    // до десериализации.
    if (inputChannel.Read (magic) != NoError || inputChannel.Read (version) != NoError ||
        inputChannel.Read (blobSize) != NoError)
        return false;
    if (magic != SyncSettingsFileMagic || version != PreferencesVersion)
        return false;
    if (blobSize == 0 || blobSize != (UInt64)fileSize - (UInt64)SyncSettingsHeaderSize)
        return false;

    SyncSettings tempsyncSettings;
    if (tempsyncSettings.Read (inputChannel) != NoError)
        return false;

    syncSettings = tempsyncSettings;
    return true;
}

// --------------------------------------------------------------------
// Запись уже сериализованного блоба в локальный файл (заголовок + блоб).
// --------------------------------------------------------------------
static bool WriteSyncSettingsFile (const char *blobData, UInt64 blobSize) {
    if (blobData == nullptr || blobSize == 0)
        return false;

    IO::Location folderLoc;
    if (!GetSyncSettingsFolderLocation (folderLoc)) {
        msg_rep ("WriteSyncSettingsFile", "Cant resolve sync settings folder", NoError, APINULLGuid);
        return false;
    }

    MemoryOChannel headerChannel;
    headerChannel.Write (SyncSettingsFileMagic);
    headerChannel.Write (PreferencesVersion);
    headerChannel.Write (blobSize);
    if (headerChannel.GetOutputStatus () != NoError || headerChannel.GetDataSize () != SyncSettingsHeaderSize)
        return false;

    const IO::Location fileLoc (folderLoc, IO::Name (SyncSettingsFileName));
    IO::File file (fileLoc, IO::File::OnNotFound::Create);
    if (file.Open (IO::File::WriteEmptyMode) != NoError) {
        msg_rep ("WriteSyncSettingsFile", "Cant open " + fileLoc.ToDisplayText (), NoError, APINULLGuid);
        return false;
    }

    GSErrCode err = file.WriteBin (headerChannel.GetDestination (), (USize)headerChannel.GetDataSize ());
    if (err == NoError)
        err = file.WriteBin (blobData, (USize)blobSize);
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
// skipIfUnchanged — не писать, если блоб не изменился с прошлой успешной
// записи (в observer-путях запись вызывается часто, сравнение дешевле I/O).
// --------------------------------------------------------------------
static bool WriteSyncSettingsToFile (const SyncSettings &syncSettings, bool skipIfUnchanged) {
    MemoryOChannel outputChannel;
    if (syncSettings.Write (outputChannel) != NoError)
        return false;

    const UInt64 blobSize = outputChannel.GetDataSize ();
    const char *blobData = outputChannel.GetDestination ();
    if (blobData == nullptr || blobSize == 0)
        return false;

    static std::vector<char> lastWritten;
    if (skipIfUnchanged && lastWritten.size () == (size_t)blobSize &&
        std::memcmp (lastWritten.data (), blobData, (size_t)blobSize) == 0)
        return true;

    if (!WriteSyncSettingsFile (blobData, blobSize))
        return false;

    lastWritten.assign (blobData, blobData + blobSize);
    return true;
}

// --------------------------------------------------------------------
// Одноразовая миграция: читаем старые значения из preferences проекта
// (до локального файла настройки хранились там). После успешной миграции
// ACAPI_SetPreferences не вызывается больше никогда.
// --------------------------------------------------------------------
static bool ReadSyncSettingsFromLegacyPreferences (SyncSettings &syncSettings) {
    Int32 version = PreferencesVersion;
    GSSize bytes = 0;
    if (ACAPI_GetPreferences (&version, &bytes, nullptr) != NoError || version == 0 || bytes == 0)
        return false;
    // Если версия старая — не читаем, используем дефолты (как и раньше).
    if (version != PreferencesVersion)
        return false;

    std::vector<char> data ((size_t)bytes);
    if (ACAPI_GetPreferences (&version, &bytes, data.data ()) != NoError)
        return false;

    SyncSettings tempsyncSettings;
    MemoryIChannel inputChannel (data.data (), (size_t)bytes);
    if (tempsyncSettings.Read (inputChannel) != NoError)
        return false;

    syncSettings = tempsyncSettings;
    return true;
}

// --------------------------------------------------------------------
// Чтение настроек: локальный файл, при его отсутствии — одноразовая миграция
// из старых preferences проекта + сразу запись локального файла.
// --------------------------------------------------------------------
static bool ReadSyncSettings (SyncSettings &syncSettings) {
    if (ReadSyncSettingsFromFile (syncSettings))
        return true;

    if (ReadSyncSettingsFromLegacyPreferences (syncSettings)) {
        DBprnt ("SyncSettings: migrating legacy prefs to local file");
        // Переносим прочитанные значения в локальный файл, чтобы в preferences
        // проекта больше не возвращаться (skipIfUnchanged=false — пишем сразу).
        WriteSyncSettingsToFile (syncSettings, false);
        return true;
    }
    return false;
}

// --------------------------------------------------------------------
// Кэш настроек
// Кэширует экземпляр SyncSettings в статической области. При forceReload
// текущие настройки заново загружаются из Preferences Archicad.
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
    // Пишем локальный файл (не preferences проекта!) и только если настройки
    // действительно изменились.
    if (!WriteSyncSettingsToFile (syncSettings, true))
        return false;
    // Обновляем кэш только после успешной записи.
    GetSyncSettingsCache (false) = syncSettings;
    return true;
}
