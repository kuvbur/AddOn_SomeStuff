#pragma once

#include "ACAPinc.h"

#if defined(TESTING) && defined(ServerMainVers_2500)
    #include "json_commands/CommandBase.hpp"

// Явный TESTING-вызов существующего writer по подготовленным 3D-граням.
GS::ObjectState DrawRoomSurfaceWalls (const GS::ObjectState &parameters);

// Диагностика вызывается только явно через JSON, не из автоматического TestKit.
class RoomGeometryDiagnosticCommand : public CommandBase {
  public:
    RoomGeometryDiagnosticCommand ();
    GS::String GetName () const override;
    GS::Optional<GS::UniString> GetInputParametersSchema () const override;
    GS::Optional<GS::UniString> GetResponseSchema () const override;
    GS::ObjectState Execute (const GS::ObjectState &parameters, GS::ProcessControl &processControl) const override;
};
#endif
