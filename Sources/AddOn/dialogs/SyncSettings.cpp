//------------ kuvbur 2022 ------------
#include "api_headers/APIEnvir.h"

#include "ACAPinc.h"

#include "SyncSettings.hpp"

static const Int32 PreferencesVersion = 4;

GS::ClassInfo SyncSettings::classInfo ("SyncSettings",
                                       GS::Guid ("B45089A9-B372-460B-B145-80E6EBF107C3"),
                                       GS::ClassVersion (1, 0));

SyncSettings::SyncSettings () : SyncSettings (false, false, true, true, true, true, false, false) {}

SyncSettings::SyncSettings (bool syncAll, bool syncMon, bool wallS, bool widoS, bool objS, bool cwallS, bool logMon, bool showpalette)
    : syncAll (syncAll), syncMon (syncMon), wallS (wallS), widoS (widoS), objS (objS), cwallS (cwallS),
      logMon (logMon), showpalette(showpalette) {}

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
    return oc.GetOutputStatus ();
}

static bool ReadSyncSettings (SyncSettings &syncSettings) {
    GSErrCode err = NoError;
    Int32 version = PreferencesVersion;
    GSSize bytes = 0;
    err = ACAPI_GetPreferences (&version, &bytes, nullptr);
    if (err != NoError || version == 0 || bytes == 0) {
        return false;
    }
    char *data = new char[bytes];
    err = ACAPI_GetPreferences (&version, &bytes, data);
    if (err != NoError) {
        delete[] data;
        return false;
    }
    SyncSettings tempsyncSettings;
    MemoryIChannel inputChannel (data, bytes);
    err = tempsyncSettings.Read (inputChannel);
    if (err != NoError) {
        delete[] data;
        return false;
    }
    syncSettings = tempsyncSettings;
    delete[] data;
    return true;
}

// --------------------------------------------------------------------
// Кэш настроек
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
    GetSyncSettingsCache (false) = syncSettings;
    return true;
}
