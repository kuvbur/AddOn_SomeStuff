//------------ kuvbur 2022 ------------
#pragma once
#if !defined(REVISION_HPP)
#define REVISION_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_27
#include "DG.h"
#include "Helpers.hpp"

static const Int32 TypeNone = 0;
static const Int32 TypeIzm = 1;
static const Int32 TypeZam = 2;
static const Int32 TypeNov = 3;
static const Int32 TypeAnnul = 4;

typedef struct
{
    API_Coord startpoint = { 0,0 };
    API_Guid markerguid = APINULLGuid;
    GS::UniString changeId = "";
    GS::UniString changeName = "";
    GS::UniString text = ""; // Текст изменения
    GS::UniString number = "";
    GS::UniString inx = "";
    GS::Int32 typeizm = TypeNone;
} Change;

typedef struct
{
    GS::Array<Change> arr;
    GS::Int32 nuch = 0;
} Changes;

typedef std::map<std::string, Changes, doj::alphanum_less<std::string>> ChangeMarkerDict;

typedef std::map<std::string, ChangeMarkerDict, doj::alphanum_less<std::string>> ChangeMarkerByListDict;

static void Do_GetChangeCustomScheme (void);

void ChangeMarkerText (API_Guid& markerguid, GS::UniString& number, GS::UniString& inx);

bool GetMarkerPos (API_Guid& markerguid, API_Coord& startpoint);

bool GetMarkerText (API_Guid& markerguid, GS::UniString& text, GS::UniString& number, GS::UniString& inx, GS::Int32& type);

bool GetChangesMarker (ChangeMarkerDict& changes);

bool GetAllChangesMarker (void);

void SetRevision (void);

#endif
