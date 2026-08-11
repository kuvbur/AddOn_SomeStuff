#if !defined(GETPROPERTYDEFINITIONSCOMMAND_HPP)
#define GETPROPERTYDEFINITIONSCOMMAND_HPP
#include "CommandBase.hpp"
#include "Propertycache.hpp"
#include "CommonFunction.hpp"

// -----------------------------------------------------------------------------
// Команда получения списка всех доступных свойств из кэша.
//
// Возвращает набор метаданных по свойствам, загруженным в PROPERTYCACHE().property.
// -----------------------------------------------------------------------------
class GetPropertyDefinitionsCommand : public ReadOnlyCommand
{
public:
    GetPropertyDefinitionsCommand();
    
    // -------------------------------------------------------------------------
    // @return Имя команды JSON.
    // -------------------------------------------------------------------------
    virtual GS::String GetName() const override;

    // -------------------------------------------------------------------------
    // @return Схема входных параметров. В этой команде нет входных параметров.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;

    // -------------------------------------------------------------------------
    // @return Схема ответа. В этой команде схема не задаётся.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;

    // -------------------------------------------------------------------------
    // Выполняет команду и возвращает список всех свойств, доступных в кэше.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute(const GS::ObjectState& parameters,
                                   GS::ProcessControl& processControl) const override;
};

#endif // GETPROPERTYDEFINITIONSCOMMAND_HPP
