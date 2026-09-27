#include <chrono>

#include "ACAPinc.h"

#include "json_commands/SyncAllCommand.hpp"

#include "dialogs/SyncSettings.hpp"
#include "Propertycache.hpp"
#include "Sync.hpp"

#if defined(ServerMainVers_2500)

// -----------------------------------------------------------------------------
// Создаёт JSON-команду запуска полной синхронизации.
// -----------------------------------------------------------------------------
SyncAllCommand::SyncAllCommand () : CommandBase (CommonSchema::NotUsed) {}

// -----------------------------------------------------------------------------
// Возвращает имя команды запуска полной синхронизации.
// -----------------------------------------------------------------------------
GS::String SyncAllCommand::GetName () const { return "SyncAll"; }

// -----------------------------------------------------------------------------
// Указывает, что команда полной синхронизации не принимает параметры.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SyncAllCommand::GetInputParametersSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Указывает, что команда полной синхронизации возвращает свободный ответ.
// -----------------------------------------------------------------------------
GS::Optional<GS::UniString> SyncAllCommand::GetResponseSchema () const { return GS::NoValue; }

// -----------------------------------------------------------------------------
// Запускает полную синхронизацию и возвращает длительность вызова.
// -----------------------------------------------------------------------------
GS::ObjectState SyncAllCommand::Execute (const GS::ObjectState & /*parameters*/,
                                         GS::ProcessControl & /*processControl*/) const {
    // Настройки перечитываются из локального конфига: у внешнего вызова нет
    // интерфейса, которым обычно включают флаги обхода (wallS/widoS/objS/cwallS).
    SyncSettings syncSettings;
    LoadSyncSettingsFromPreferences (syncSettings, true);
    // Кэш свойств питает SyncByType/SyncData. Пункт меню обновляет его перед вызовом
    // SyncAndMonAll; у JSON-вызова другой точки обновления нет.
    PROPERTYCACHE ().Update ();
    const auto start = std::chrono::steady_clock::now ();
    SyncAndMonAll (syncSettings);
    const auto finish = std::chrono::steady_clock::now ();

    const double elapsedSeconds = std::chrono::duration<double> (finish - start).count ();
    GS::ObjectState response;
    // SyncAndMonAll объявлена как void, поэтому доказуемы только факт возврата и
    // длительность вызова; успех синхронизации и корректность модели не заявляются.
    response.Add ("status", "returned");
    response.Add ("elapsedSeconds", elapsedSeconds);
    return response;
}

#endif // defined (ServerMainVers_2500)
