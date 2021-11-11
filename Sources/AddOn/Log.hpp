#if !defined (LOGS_HPP)
#define	LOGS_HPP
#pragma once
#include	"APICommon.h"
#include	"DG.h"

// Структура хранимого лога
// Тип изменения (создание, модификация, классификация)
//   Уровень (позапрошлое изменение, прошлое, текущее)
//     Тип и дата изменения

typedef struct {
	char	time[128];
	short	userId;
	bool	isempty;
} lgL3;

typedef struct {
	//TODO Переделать на массивы
	lgL3 old_2;
	lgL3 old_1;
	lgL3 current;
	bool isempty;
} lgL2;

typedef struct {
	lgL2	create;
	lgL2	edit;
	lgL2	change;
	lgL2	classification;
	lgL2	property;
} LogData;


bool LogCheckElementType(const API_ElemTypeID& elementType);

void LogGetEmpty(LogData& logData);
GSErrCode LogWriteElement(const API_Guid& guid, const LogData& logData);
GSErrCode LogReadElement(const API_Guid& guid, LogData& logData);
void LogRow(lgL3& record, const GS::HashTable<short, API_UserInfo>& userInfoTable);
void LogRow(lgL2& record, const GS::HashTable<short, API_UserInfo>& userInfoTable);
void LogShowElement(const API_Guid& elemGuid, const GS::HashTable<short, API_UserInfo>& userInfoTable);
void LogShowSelected();
void LogDataRotate(lgL2& partlogData);
void LogWriteElement(const API_NotifyElementType *elemType);

static GSErrCode GetTeamworkMembers(GS::HashTable<short, API_UserInfo>& userInfoTable);

#endif