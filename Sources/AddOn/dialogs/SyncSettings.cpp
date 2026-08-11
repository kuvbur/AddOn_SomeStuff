//------------ kuvbur 2022 ------------
#include "api_headers/APIEnvir.h"

#include "ACAPinc.h"

#include "SyncSettings.hpp"

#include <vector>

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
    : syncAll (false)
    , syncMon (false)
    , wallS (true)
    , widoS (true)
    , objS (true)
    , cwallS (true)
    , logMon (false)
    , showpalette (false)
    , catchSelectionChanges (true)
    , maxSelectionCount (10) {}

SyncSettings SyncSettings::CreateDefault () {
    return SyncSettings ();
}

SyncSettings SyncSettings::CreateWithSyncAll () {
    SyncSettings settings;
    settings.SetSyncAll (true);
    return settings;
}

bool SyncSettings::GetSyncAll () const {
    return syncAll;
}

void SyncSettings::SetSyncAll (bool value) {
    syncAll = value;
}

bool SyncSettings::GetSyncMon () const {
    return syncMon;
}

void SyncSettings::SetSyncMon (bool value) {
    syncMon = value;
}

bool SyncSettings::GetWallS () const {
    return wallS;
}

void SyncSettings::SetWallS (bool value) {
    wallS = value;
}

bool SyncSettings::GetWidoS () const {
    return widoS;
}

void SyncSettings::SetWidoS (bool value) {
    widoS = value;
}

bool SyncSettings::GetObjS () const {
    return objS;
}

void SyncSettings::SetObjS (bool value) {
    objS = value;
}

bool SyncSettings::GetCwallS () const {
    return cwallS;
}

void SyncSettings::SetCwallS (bool value) {
    cwallS = value;
}

bool SyncSettings::GetLogMon () const {
    return logMon;
}

void SyncSettings::SetLogMon (bool value) {
    logMon = value;
}

bool SyncSettings::GetShowPalette () const {
    return showpalette;
}

void SyncSettings::SetShowPalette (bool value) {
    showpalette = value;
}

bool SyncSettings::GetCatchSelectionChanges () const {
    return catchSelectionChanges;
}

void SyncSettings::SetCatchSelectionChanges (bool value) {
    catchSelectionChanges = value;
}

USize SyncSettings::GetMaxSelectionCount () const {
    return maxSelectionCount;
}

void SyncSettings::SetMaxSelectionCount (USize value) {
    maxSelectionCount = value;
}
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
// Вспомогательная функция для загрузки настроек из Preferences Archicad.
// Читает бинарные настройки и десериализует их в объект.
// --------------------------------------------------------------------
static bool ReadSyncSettings (SyncSettings &syncSettings) {
    GSErrCode err = NoError;
    Int32 version = PreferencesVersion;
    GSSize bytes = 0;
    err = ACAPI_GetPreferences (&version, &bytes, nullptr);
    if (err != NoError || version == 0 || bytes == 0) {
        return false;
    }
    // Проверяем, что версия сохранённых настроек совпадает с текущей.
    // Если версия старая — не читаем, вернём false, чтобы использовались дефолты.
    if (version != PreferencesVersion) {
        return false;
    }

    std::vector<char> data (bytes);
    err = ACAPI_GetPreferences (&version, &bytes, data.data());
    if (err != NoError) {
        return false;
    }

    SyncSettings tempsyncSettings;
    MemoryIChannel inputChannel (data.data(), bytes);
    err = tempsyncSettings.Read (inputChannel);
    if (err != NoError) {
        return false;
    }

    syncSettings = tempsyncSettings;
    return true;
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
    GSErrCode err = NoError;
    MemoryOChannel outputChannel;
    err = syncSettings.Write (outputChannel);
    if (err != NoError) {
        return false;
    }
    UInt64 bytes = outputChannel.GetDataSize ();
    const char *data = outputChannel.GetDestination ();
    err = ACAPI_SetPreferences (PreferencesVersion, (GSSize)bytes, data);
    if (err != NoError) {
        return false;
    }
    // Обновляем кэш сохранённых настроек после успешной записи.
    GetSyncSettingsCache (false) = syncSettings;
    return true;
}
