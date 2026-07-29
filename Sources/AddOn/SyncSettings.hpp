#ifndef MAZESETTINGS_HPP
#define MAZESETTINGS_HPP
#include "ACAPinc.h"

// Хранение и сериализация настроек синхронизации add-on в памяти и в настройках Archicad.
#include "MemoryIChannel.hpp"
#include "MemoryOChannel.hpp"
#include "Object.hpp"

class SyncSettings : public GS::Object {
    DECLARE_CLASS_INFO;

  public:
    // Создаёт набор настроек синхронизации с значениями по умолчанию.
    SyncSettings ();

    // Создаёт набор настроек синхронизации на основе выбранных флагов работы add-on.
    SyncSettings (bool syncAll, bool syncMon, bool wallS, bool widoS, bool objS, bool cwallS, bool logMon);

    // Считывает настройки из канала памяти или предпочтений Archicad.
    virtual GSErrCode Read (GS::IChannel &ic) override;

    // Сериализует настройки в канал памяти или предпочтений Archicad.
    virtual GSErrCode Write (GS::OChannel &oc) const override;

    bool syncAll;
    bool syncMon;
    bool wallS;
    bool widoS;
    bool objS;
    bool cwallS;
    bool logMon;
};

#if defined(ServerMainVers_2500)
using MemoryIChannel = GS::MemoryIChannel;
using MemoryOChannel = GS::MemoryOChannel;
#else
using MemoryIChannel = IO::MemoryIChannel;
using MemoryOChannel = IO::MemoryOChannel;
#endif

// Читает настройки синхронизации напрямую из текущего кэша.
static bool ReadSyncSettings (SyncSettings &syncSettings);

// --------------------------------------------------------------------
// Кэш настроек
// --------------------------------------------------------------------
// Возвращает единственный экземпляр настроек, загруженный из кэша или из предпочтений.
SyncSettings &GetSyncSettingsCache (bool forceReload);

// Загружает настройки из предпочтений Archicad в переданный объект.
bool LoadSyncSettingsFromPreferences (SyncSettings &syncSettings, bool forceReload = false);

// Сохраняет настройки синхронизации в предпочтения Archicad.
bool WriteSyncSettingsToPreferences (const SyncSettings &syncSettings);

#endif
