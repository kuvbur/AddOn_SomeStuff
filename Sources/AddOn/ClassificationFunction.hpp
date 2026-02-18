//------------ kuvbur 2022 ------------
#pragma once
#if !defined (CLASSIF_HPP)
#define	CLASSIF_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include "APICommon28.h"
#endif // AC_28

namespace ClassificationFunc
{

struct ClassificationValues
{
    API_ClassificationSystem system; // Система
    API_ClassificationItem item; // Класс
    GS::UniString parentname = ""; // Имя родиельского класса
    GS::UniString itemname = ""; // Имя класса
}; // Структура для хранения класса

typedef GS::HashTable<GS::UniString, ClassificationValues> ClassificationDict; // Словарь классов в системе

typedef GS::HashTable<GS::UniString, ClassificationDict> SystemDict; // Словарь систем с вложенными классами

// -----------------------------------------------------------------------------
// Получение словаря со всеми классами во всех системах классифкации
// -----------------------------------------------------------------------------
GSErrCode GetAllClassification (SystemDict& systemdict);

void GatherAllDescendantOfClassification (const API_ClassificationItem& item, ClassificationDict& classifications, const API_ClassificationSystem& system);
void AddClassificationItem (const API_ClassificationItem& item, const  API_ClassificationItem& parent, ClassificationDict& classifications, const API_ClassificationSystem& system);

// -----------------------------------------------------------------------------
// Получение полного имени класса с чётом родительских классов
// -----------------------------------------------------------------------------
void GetFullName (const API_ClassificationItem& item, const ClassificationDict& classifications, GS::UniString& fullname);

// -----------------------------------------------------------------------------
// Поиск класса по ID в заданной классификации, возвращает Guid класса
// -----------------------------------------------------------------------------
API_Guid FindClass (GS::UniString& systemname, GS::UniString& classname);

// -----------------------------------------------------------------------------
// Назначение автокласса (класса с описанием some_stuff_class) элементу без классификации
// -----------------------------------------------------------------------------
void SetAutoclass (const API_Guid elemGuid);

bool ReadSystemDict ();
}

#endif
