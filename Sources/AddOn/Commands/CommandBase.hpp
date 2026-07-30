#pragma once

#include "APIEnvir.h"
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

/**
 * Базовый класс для ВСЕХ команд аддона.
 */
class CommandBase : public API_AddOnCommand
{
public:
    explicit CommandBase(CommonSchema commonSchema);

    virtual GS::String GetNamespace() const override final;
    virtual API_AddOnCommandExecutionPolicy GetExecutionPolicy() const override final;
    virtual void OnResponseValidationFailed(const GS::ObjectState& response) const override final;
    virtual GS::Optional<GS::UniString> GetSchemaDefinitions() const override final;
    virtual GS::Optional<GS::UniString> GetInputParametersSchema() const override;
    virtual GS::Optional<GS::UniString> GetResponseSchema() const override;

private:
    CommonSchema mCommonSchema;
};

/**
 * Базовый класс для READ-ONLY команд (БЕЗ undo).
 */
class ReadOnlyCommand : public CommandBase
{
public:
    ReadOnlyCommand() : CommandBase(CommonSchema::Used) {}
    
protected:
    GS::ObjectState CreateErrorResponse(GSErrCode errorCode, const GS::UniString& errorMessage) const;
    GS::ObjectState CreateSuccessResponse() const;
};

/**
 * Базовый класс для MODIFY команд (С undo).
 */
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
