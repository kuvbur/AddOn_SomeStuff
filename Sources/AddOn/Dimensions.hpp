//------------ kuvbur 2022 ------------
#pragma once
#ifndef DIM_HPP
#define	DIM_HPP
#include "DG.h"
#include "Helpers.hpp"
#include "SyncSettings.hpp"

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
GSErrCode DimAutoRound (const API_Guid& elemGuid, const SyncSettings& syncSettings, bool isUndo);

// -----------------------------------------------------------------------------
// Обрабатывает размер и решает - что с ним делать
//	flag_change - менять текст размера, сбросить или не менять (DIM_CHANGE_ON, DIM_CHANGE_OFF, DIM_NOCHANGE)
//	flag_highlight - изменять перо текста, сбросить на оригинальное или не менять (DIM_HIGHLIGHT_ON, DIM_HIGHLIGHT_OFF, DIM_NOCHANGE)
// -----------------------------------------------------------------------------
bool DimParse (const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, const DimRule& dimrule);

void DimRoundOne (const API_Guid& elemGuid, const SyncSettings& syncSettings, bool isUndo);

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// -----------------------------------------------------------------------------
void DimRoundAll (const SyncSettings& syncSettings, bool isUndo);

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// -----------------------------------------------------------------------------
bool DimRoundByType (const API_ElemTypeID& typeID, DoneElemGuid& doneelemguid, const SyncSettings& syncSettings, bool isUndo);

#endif
