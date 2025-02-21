//------------ kuvbur 2022 ------------
#pragma once
#ifndef DIM_HPP
#define	DIM_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#if defined(AC_27)
#include	"APICommon27.h"
#endif // AC_27
#include	"DG.h"
#include	"SyncSettings.hpp"
#include	"Helpers.hpp"

typedef struct
{
    short	pen_original = 0;
    short	pen_rounded = 0;
    UInt32	round_value = 0;
    bool	flag_change = false;
    bool	flag_deletewall = false;
    bool	flag_reset = false;
    bool	flag_custom = false;
    bool	classic_round_mode = false; // Использовать вместо округления вверх обычное округление
    GS::UniString expression = "";
    GS::UniString layer = "";
    ParamDictValue paramDict;
} DimRule;

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

typedef GS::HashTable<GS::UniString, DimRule> DimRules;


bool HasDimAutotext ();

// -----------------------------------------------------------------------------
// Чтение настроек из информации о проекте
//	Имя свойства: "Addon_Dimenstions"
// -----------------------------------------------------------------------------
bool GetDimAutotext (GS::UniString& autotext);

// -----------------------------------------------------------------------------
// Чтение настроек из информации о проекте
//	Имя свойства: "Addon_Dimenstions"
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
bool DimReadPref (DimRules& dimrules, GS::UniString& autotext);

// -----------------------------------------------------------------------------
// Обработка текста правила
// -----------------------------------------------------------------------------
bool DimParsePref (GS::UniString& rawrule, DimRule& dimrule, bool& hasexpression);

// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
GSErrCode DimAutoRound (const API_Guid& elemGuid, DimRules& dimrules, ParamDictValue& propertyParams, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Обрабатывает размер и решает - что с ним делать
//	flag_change - менять текст размера, сбросить или не менять (DIM_CHANGE_ON, DIM_CHANGE_OFF, DIM_NOCHANGE)
//	flag_highlight - изменять перо текста, сбросить на оригинальное или не менять (DIM_HIGHLIGHT_ON, DIM_HIGHLIGHT_OFF, DIM_NOCHANGE)
// -----------------------------------------------------------------------------
bool DimParse (const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, DimRule& dimrule, ParamDictValue& propertyParams);

void DimRoundOne (const API_Guid& elemGuid, const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// -----------------------------------------------------------------------------
void DimRoundAll (const SyncSettings& syncSettings);

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// -----------------------------------------------------------------------------
bool DimRoundByType (const API_ElemTypeID& typeID, DoneElemGuid& doneelemguid, DimRules& dimrules, ParamDictValue& propertyParams, const SyncSettings& syncSettings);

#endif
