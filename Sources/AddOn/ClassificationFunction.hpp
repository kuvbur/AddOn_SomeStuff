//------------ kuvbur 2022 ------------
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

typedef struct
{
    API_ClassificationSystem system;
    API_ClassificationItem item;
    API_ClassificationItem parent;
    GS::UniString parentname = "";
    GS::UniString itemname = "";
} ClassificationValues;

// Словарь с классами
typedef GS::HashTable<GS::UniString, ClassificationValues> ClassificationDict;

// Словарь с системами
typedef GS::HashTable<GS::UniString, ClassificationDict> SystemDict;

GSErrCode GetAllClassification (SystemDict& systemdict);
void GatherAllDescendantOfClassification (const API_ClassificationItem& item, ClassificationDict& classifications, const API_ClassificationSystem& system);
void AddClassificationItem (const API_ClassificationItem& item, const  API_ClassificationItem& parent, ClassificationDict& classifications, const API_ClassificationSystem& system);
void GetFullName (const API_ClassificationItem& item, const ClassificationDict& classifications, GS::UniString& fullname);
}

#endif