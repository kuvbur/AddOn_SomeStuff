#pragma once
#ifndef DIM_HPP
#define	DIM_HPP
#include	"APICommon.h"
#include	"DG.h"
#include	"SyncSettings.hpp"
#include	"Helpers.hpp"

typedef struct {
	short	pen_original;
	short	pen_rounded;
	UInt32	round_value;
	bool	flag_change;
	GS::UniString expression;
	GS::UniString layer;
	ParamDict paramDict;
} DimRule;

typedef GS::HashTable<API_Guid, bool> DoneElemGuid;

typedef GS::HashTable<GS::UniString, DimRule> DimRules;

GSErrCode DimReadPref(DimRules& dimrules);

bool DimParsePref(GS::UniString rawrule, DimRule& dimrule);

void DimAutoRoundSel(const API_Guid& elemGuid, const SyncSettings& syncSettings);

void DimSelected(const SyncSettings& syncSettings);
GSErrCode DimAutoRound(const API_Guid& elemGuid, DimRules& dimrules);

bool DimParse(const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, DimRule& dimrule);

void DimRoundAll(const SyncSettings& syncSettings);

bool DimRoundByType(const API_ElemTypeID typeID, DoneElemGuid& doneelemguid, DimRules& dimrules);

#endif