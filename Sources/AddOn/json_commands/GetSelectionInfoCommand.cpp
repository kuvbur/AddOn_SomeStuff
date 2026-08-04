#include "json_commands/GetSelectionInfoCommand.hpp"

GetSelectionInfoCommand::GetSelectionInfoCommand () : ReadOnlyCommand () {}

GS::String GetSelectionInfoCommand::GetName () const { return "GetSelectionInfo"; }

GS::Optional<GS::UniString> GetSelectionInfoCommand::GetInputParametersSchema () const { return GS::NoValue; }

GS::Optional<GS::UniString> GetSelectionInfoCommand::GetResponseSchema () const { return GS::NoValue; }

GS::ObjectState GetSelectionInfoCommand::Execute (const GS::ObjectState & /*parameters*/,
                                                  GS::ProcessControl & /*processControl*/) const {
    // Получаем GUID-ы всех выделенных элементов
    // Используем GetSelectedElements2 — оптимизированную версию без чтения настроек
    GS::Array<API_Guid> selectedElements = GetSelectedElements2 (false, true);

    GS::ObjectState response;
    response.Add ("count", selectedElements.GetSize ());

    return response;
}