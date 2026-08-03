#ifndef SYNCSETTINGS_HPP
#define SYNCSETTINGS_HPP
#include "ACAPinc.h"

// Хранение и сериализацию настроек синхронизации add-on в памяти и в настройках Archicad.
#include "MemoryIChannel.hpp"
#include "MemoryOChannel.hpp"
#include "Object.hpp"
// --------------------------------------------------------------------
/// SyncSettings хранит флаги, определяющие поведение синхронизации и мониторинга.
/// Класс поддерживает сериализацию в память Archicad через Preferences и
/// загрузку/сохранение настроек из единого кэша.
// --------------------------------------------------------------------
class SyncSettings : public GS::Object {
    DECLARE_CLASS_INFO;

  public:
    /// Конструктор с разумными значениями по умолчанию.
    SyncSettings ();

    /// Возвращает стандартный набор настроек по умолчанию.
    static SyncSettings CreateDefault ();

    /// Возвращает стандартный набор настроек и включает `syncAll`.
    static SyncSettings CreateWithSyncAll ();

    /// Считывает настройки из канала памяти или предпочтений Archicad.
    virtual GSErrCode Read (GS::IChannel &ic) override;

    /// Сериализует настройки в канал памяти или предпочтений Archicad.
    virtual GSErrCode Write (GS::OChannel &oc) const override;

    /// Доступ к сохранённым флагам синхронизации.
    /// Флаг, определяющий выполнение полной синхронизации всех доступных элементов.
    bool GetSyncAll () const;
    void SetSyncAll (bool value);

    /// Флаг включения мониторинга изменений элементов.
    bool GetSyncMon () const;
    void SetSyncMon (bool value);

    /// Флаг обработки стен и связанных элементов.
    bool GetWallS () const;
    void SetWallS (bool value);

    /// Флаг обработки окон, дверей и Skylight.
    bool GetWidoS () const;
    void SetWidoS (bool value);

    /// Флаг обработки объектов, светильников и зон.
    bool GetObjS () const;
    void SetObjS (bool value);

    /// Флаг обработки curtain wall и связанных сегментов.
    bool GetCwallS () const;
    void SetCwallS (bool value);

    /// Включает режим логирования мониторинга.
    bool GetLogMon () const;
    void SetLogMon (bool value);

    /// Показывать ли палитру браузера.
    bool GetShowPalette () const;
    void SetShowPalette (bool value);

  private:
    bool syncAll;
    bool syncMon;
    bool wallS;
    bool widoS;
    bool objS;
    bool cwallS;
    bool logMon;
    bool showpalette;
};

#if defined(ServerMainVers_2500)
using MemoryIChannel = GS::MemoryIChannel;
using MemoryOChannel = GS::MemoryOChannel;
#else
using MemoryIChannel = IO::MemoryIChannel;
using MemoryOChannel = IO::MemoryOChannel;
#endif

// --------------------------------------------------------------------
// Кэш настроек
// Возвращает единственный экземпляр настроек, загруженный из кэша или из предпочтений.
// --------------------------------------------------------------------
SyncSettings &GetSyncSettingsCache (bool forceReload);
// --------------------------------------------------------------------
// Загружает настройки из предпочтений Archicad в переданный объект.
// --------------------------------------------------------------------
bool LoadSyncSettingsFromPreferences (SyncSettings &syncSettings, bool forceReload = false);
// --------------------------------------------------------------------
// Сохраняет настройки синхронизации в предпочтения Archicad.
// --------------------------------------------------------------------
bool WriteSyncSettingsToPreferences (const SyncSettings &syncSettings);

#endif
