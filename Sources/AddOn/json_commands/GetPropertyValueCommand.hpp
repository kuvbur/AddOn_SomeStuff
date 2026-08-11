#if !defined(GETPROPERTYVALUECOMMAND_HPP)
#define GETPROPERTYVALUECOMMAND_HPP
#include "CommonFunction.hpp"
#include "json_commands/CommandBase.hpp"

// -----------------------------------------------------------------------------
// Команда получения значений одного свойства для выделенных элементов.
//
// Входные параметры:
//   - propertyId: идентификатор свойства.
//
// Ответ содержит имя свойства, флаг common и массив уникальных значений с их счетчиками.
// -----------------------------------------------------------------------------
class GetPropertyValueCommand : public ReadOnlyCommand {
  public:
    GetPropertyValueCommand ();

    // -------------------------------------------------------------------------
    // @return Имя команды JSON.
    // -------------------------------------------------------------------------
    virtual GS::String GetName () const override;

    // -------------------------------------------------------------------------
    // @return Схема входных параметров. В этой команде нет схемы.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema () const override;

    // -------------------------------------------------------------------------
    // @return Схема ответа. В этой команде схема не задаётся.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema () const override;

    // -------------------------------------------------------------------------
    // Выполняет команду, подсчитывает значения свойства среди выделенных элементов и возвращает статистику.
    // -------------------------------------------------------------------------
    virtual GS::ObjectState Execute (const GS::ObjectState &parameters,
                                     GS::ProcessControl &processControl) const override;
};

#endif // GETPROPERTYVALUECOMMAND_HPP