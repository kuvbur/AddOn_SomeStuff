#ifndef MAZESETTINGS_HPP
#define MAZESETTINGS_HPP
#include "ACAPinc.h"
#include "MemoryIChannel.hpp"
#include "MemoryOChannel.hpp"
#include "Object.hpp"

class SyncSettings : public GS::Object
{
    DECLARE_CLASS_INFO;

public:
    SyncSettings ();
    SyncSettings (bool syncAll, bool syncMon, bool wallS, bool widoS, bool objS, bool cwallS, bool logMon);

    virtual	GSErrCode	Read (GS::IChannel& ic) override;
    virtual	GSErrCode	Write (GS::OChannel& oc) const override;

    bool	syncAll;
    bool	syncMon;
    bool	wallS;
    bool	widoS;
    bool	objS;
    bool	cwallS;
    bool	logMon;
};

#if defined (ServerMainVers_2500)
using MemoryIChannel = GS::MemoryIChannel;
using MemoryOChannel = GS::MemoryOChannel;
#else
using MemoryIChannel = IO::MemoryIChannel;
using MemoryOChannel = IO::MemoryOChannel;
#endif

bool LoadSyncSettingsFromPreferences (SyncSettings& syncSettings);
bool WriteSyncSettingsToPreferences (const SyncSettings& syncSettings);

#endif