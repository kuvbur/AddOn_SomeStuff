//------------ kuvbur 2022 ------------
#pragma once
#if !defined(RESET_HPP)
    #define RESET_HPP
    #ifdef AC_25
        #include "APICommon25.h"
    #endif // AC_25
    #ifdef AC_26
        #include "APICommon26.h"
    #endif // AC_26
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        #include "APICommon27.h"
    #endif // AC_27
    #ifdef AC_28
        #include "APICommon28.h"
    #endif // AC_28
    #include "DG.h"
    #include "SyncSettings.hpp"

// Модуль сброса пользовательских свойств к значениям по умолчанию или к базовым значениям элемента.
//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств
//--------------------------------------------------------------------------------------------------------------------------
// Выполняет сброс пользовательских свойств в режиме add-on.
bool ResetProperty ();

//--------------------------------------------------------------------------------------------------------------------------
// Сброс свойств во всех БД файла и настройках по умолчанию
//--------------------------------------------------------------------------------------------------------------------------
// Сбрасывает свойства элементов к значениям по умолчанию во всех базах данных файла.
UInt32 ResetPropertyElement2Defult (const GS::Array<API_PropertyDefinition> &definitions_to_reset);

// Сбрасывает свойства элементов в одной конкретной базе данных проекта.
UInt32 ResetElementsInDB (const API_DatabaseID commandID,
                          const GS::Array<API_PropertyDefinition> &definitions_to_reset,
                          API_AttributeIndex layerCombIndex,
                          UnicGuid &doneelemguid);

// Сбрасывает свойства одного элемента к значениям по умолчанию.
GSErrCode ResetOneElemen (const API_Guid elemGuid, const GS::Array<API_PropertyDefinition> &definitions_to_reset);

// Сбрасывает свойства всех подходящих элементов к значениям по умолчанию.
UInt32 ResetElementsDefault (const GS::Array<API_PropertyDefinition> &definitions_to_reset);

// Сбрасывает свойства одного типа элементов в зависимости от варианта обработки.
GSErrCode ResetOneElemenDefault (API_ElemTypeID typeId,
                                 const GS::Array<API_PropertyDefinition> &definitions_to_reset,
                                 int variationID);

#endif
