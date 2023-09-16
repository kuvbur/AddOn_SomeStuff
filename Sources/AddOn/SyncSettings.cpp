//------------ kuvbur 2022 ------------
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"SyncSettings.hpp"

static const Int32 PreferencesVersion = 1;

GS::ClassInfo SyncSettings::classInfo("SyncSettings", GS::Guid("B45089A9-B372-460B-B145-80E6EBF107C3"), GS::ClassVersion(1, 0));

SyncSettings::SyncSettings() :
	SyncSettings(false, false, true, true, true, true, false)
{
}

SyncSettings::SyncSettings(bool syncAll, bool syncMon, bool wallS, bool widoS, bool objS, bool cwallS, bool logMon) :
	syncAll(syncAll),
	syncMon(syncMon),
	wallS(wallS),
	widoS(widoS),
	objS(objS),
	cwallS(cwallS),
	logMon(logMon)
{
}

GSErrCode SyncSettings::Read(GS::IChannel& ic)
{
	GS::InputFrame frame(ic, classInfo);
	ic.Read(syncAll);
	ic.Read(syncMon);
	ic.Read(wallS);
	ic.Read(widoS);
	ic.Read(objS);
	ic.Read(cwallS);
	ic.Read(logMon);
	return ic.GetInputStatus();
}

GSErrCode SyncSettings::Write(GS::OChannel& oc) const
{
	GS::OutputFrame frame(oc, classInfo);
	oc.Write(syncAll);
	oc.Write(syncMon);
	oc.Write(wallS);
	oc.Write(widoS);
	oc.Write(objS);
	oc.Write(cwallS);
	oc.Write(logMon);
	return oc.GetOutputStatus();
}

bool LoadSyncSettingsFromPreferences(SyncSettings& syncSettings)
{
	DBPrintf("== SMSTF == LoadSyncSettingsFromPreferences\n");
	GSErrCode err = NoError;
	Int32 version = 3;
	GSSize bytes = 0;
	err = ACAPI_GetPreferences(&version, &bytes, nullptr);
	if (err != NoError || version == 0 || bytes == 0) {
		return false;
	}
	char* data = new char[bytes];
	err = ACAPI_GetPreferences(&version, &bytes, data);
	if (err != NoError) {
		delete[] data;
		return false;
	}
	SyncSettings tempsyncSettings;
	MemoryIChannel inputChannel(data, bytes);
	err = tempsyncSettings.Read(inputChannel);
	if (err != NoError) {
		delete[] data;
		return false;
	}
#ifdef PK_1
	tempsyncSettings.syncMon = true;
#endif
	syncSettings = tempsyncSettings;
	delete[] data;
	return true;
}

bool WriteSyncSettingsToPreferences(const SyncSettings& syncSettings)
{
	DBPrintf("== SMSTF == WriteSyncSettingsToPreferences\n");
	GSErrCode err = NoError;
	MemoryOChannel outputChannel;
	err = syncSettings.Write(outputChannel);
	if (err != NoError) {
		return false;
	}
	UInt64 bytes = outputChannel.GetDataSize();
	const char* data = outputChannel.GetDestination();
	err = ACAPI_SetPreferences(PreferencesVersion, (GSSize)bytes, data);
	if (err != NoError) {
		return false;
	}
	return true;
}