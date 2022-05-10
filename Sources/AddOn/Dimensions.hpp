#pragma once
#if !defined (DIM_HPP)
#define	DIM_HPP
#include	"APICommon.h"
#include	"DG.h"
#include	"SyncSettings.hpp"

typedef struct {
	short	pen_original;
	short	pen_rounded;
	UInt32	round_value;
	bool	flag_change;
	GS::UniString expression;
} DimRule;

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

typedef GS::HashTable<short, DimRule> DimRules;

GSErrCode DimReadPref(DimRules& dimrules);

bool DimParsePref(GS::UniString rawrule, DimRule& dimrule);

void DimAutoRoundSelected(const SyncSettings& syncSettings);

void DimAutoRoundSel(const API_Guid& elemGuid, const SyncSettings& syncSettings);

GSErrCode DimAutoRound(const API_Guid& elemGuid, DimRules& dimrules);

bool DimParse(const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, UInt32& round_value);

GSErrCode DimAddGrid(void);

void DimAutoRoundAll(const SyncSettings& syncSettings);

void DimAutoRoundInDB(const API_DatabaseID& commandID, API_AttributeIndex layerCombIndex, DimRules& dimrules, DoneElemGuid& doneelemguid);

void GetElementList(const API_ElemTypeID typeID, DoneElemGuid& doneelemguid, DimRules& dimrules);

void GetDIMList(DoneElemGuid& doneelemguid, DimRules& dimrules);

#endif