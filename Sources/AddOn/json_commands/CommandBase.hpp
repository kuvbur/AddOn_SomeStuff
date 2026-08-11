#if !defined(COMMANDBASE_HPP)
#define COMMANDBASE_HPP

#include "api_headers/APIEnvir.h"
#include "ACAPinc.h"
#include "OnExit.hpp"

// ВАЖНО: ACAPI_CallUndoableCommand объявлен в ACAPinc.h
// Он принимает GS::UniString и лямбду, возвращающую GSErrCode

#include "ObjectState.hpp"

enum class CommonSchema
{
    Used,
    NotUsed
};

// -----------------------------------------------------------------------------
// Общая база для JSON-команд аддона.
//
// Все JSON команды наследуются от этого класса и реализуют
// имя команды, схему параметров и выполнение.
// -----------------------------------------------------------------------------
class CommandBase : public API_AddOnCommand
{
public:
    explicit CommandBase(CommonSchema commonSchema);

    // -------------------------------------------------------------------------
    // Возвращает namespace команды, используемый в JSON API клиента.
    // -------------------------------------------------------------------------
    virtual GS::String GetNamespace() const override final;

    // -------------------------------------------------------------------------
    // Возвращает политику выполнения команды в ArchiCAD.
    // -------------------------------------------------------------------------
    virtual API_AddOnCommandExecutionPolicy GetExecutionPolicy() const override final;

    // -------------------------------------------------------------------------
    // Вызывается при невалидном ответе команды.
    // -------------------------------------------------------------------------
    virtual void OnResponseValidationFailed(const GS::ObjectState& response) const override final;

    // -------------------------------------------------------------------------
    // Опционально возвращает определения JSON схемы для команды.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetSchemaDefinitions() const override final;

    // -------------------------------------------------------------------------
    // Опционально возвращает JSON схему входных параметров.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;

    // -------------------------------------------------------------------------
    // Опционально возвращает JSON схему ответа.
    // -------------------------------------------------------------------------
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;

private:
    CommonSchema mCommonSchema;
};

// -----------------------------------------------------------------------------
// Базовый класс для команд только чтения (не изменяют состояние модели).
// -----------------------------------------------------------------------------
class ReadOnlyCommand : public CommandBase
{
public:
    ReadOnlyCommand() : CommandBase(CommonSchema::Used) {}
    
protected:
    // -------------------------------------------------------------------------
    // Создание стандартного JSON ответа с ошибкой.
    // -------------------------------------------------------------------------
    GS::ObjectState CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage) const;

    // -------------------------------------------------------------------------
    // Создание стандартного успешного JSON ответа.
    // -------------------------------------------------------------------------
    GS::ObjectState CreateSuccessResponse() const;
};

// -----------------------------------------------------------------------------
// Базовый класс для команд изменения (ModifyCommand).
//
// ModifyCommand используется для команд, которые могут требовать undo
// и должны обрабатываться в контексте редактируемой модели.
// -----------------------------------------------------------------------------
class ModifyCommand : public CommandBase
{
public:
    ModifyCommand() : CommandBase(CommonSchema::Used) {}
    
protected:
    GS::ObjectState CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage) const;
    GS::ObjectState CreateSuccessResponse() const;
};

// Вспомогательные функции
API_Guid GetGuidFromObjectState(const GS::ObjectState& os);
GS::ObjectState CreateGuidObjectState(const API_Guid& guid);
GS::ObjectState CreateGuidObjectState(const GS::Guid& guid);
GS::ObjectState CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage);
GS::ObjectState CreateSuccessResponse();

#endif // COMMANDBASE_HPP
