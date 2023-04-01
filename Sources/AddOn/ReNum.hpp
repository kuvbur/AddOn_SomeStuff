//------------ kuvbur 2022 ------------
#if !defined (RENUM_HPP)
#pragma once
#define	RENUM_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"Helpers.hpp"

// Типы нумерации (см. RenumElement.state)
#define RENUM_IGNORE 0	// Игнорировать, не менять и не объединять с другими элементами позицию.
#define RENUM_ADD 1		// Позицию не менять, добавить другие элементы при совпадении критерия
#define RENUM_NORMAL 2	// Обычная нумерация/перенумерация

// Типы простановки нулей для СТРОКОВОГО (API_PropertyStringValueType) свойства (см. RenumRule.nulltype)
#define NOZEROS 0		// Не добавлять нули в текстовое свойство
#define ADDZEROS 1		// Добавлять нули с учётом разбивки
#define ADDMAXZEROS 2	// Добавлять нули по максимальному количеству без учёта разбивки

typedef struct {
	bool state;		// Корректность правила
	GS::UniString flag;	// Описание свойства, в которое ставим позицию
	GS::UniString position;	// Описание свойства, в которое ставим позицию
	GS::UniString criteria;	// Описание свойства-критерия
	GS::UniString delimetr;	// Описание свойства-разбивки
	GS::UniString prefix;	// Описание свойства-префикса
	short nulltype;	// Тип постановки нулей в позиции
	GS::Array <API_Guid> elemts;		// Массив элементов
} RenumRule;

typedef GS::HashTable<API_Guid, RenumRule> Rules;	// Таблица правил

GSErrCode ReNumSelected(SyncSettings& syncSettings);

bool GetRenumElements(const GS::Array<API_Guid> guidArray, ParamDictElement& paramToWriteelem);

bool ReNumHasFlag(const GS::Array<API_PropertyDefinition> definitions);
short ReNumGetFlag(const ParamValue& paramflag, const ParamValue& paramposition);
bool ReNum_GetElement(const API_Guid& elemGuid, ParamDictValue& propertyParams, ParamDictValue& paramToRead, Rules& rules);
void ReNumOneRule(const RenumRule& rule, ParamDictElement& paramToReadelem, ParamDictElement& paramToWriteelem);

#endif