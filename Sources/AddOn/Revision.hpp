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

typedef struct {
    API_Coord startpoint;
    API_Guid markerguid;
    GS::UniString changeId;
    GS::UniString changeName;
    GS::UniString text; // Текст изменения
    GS::UniString number;
    GS::UniString inx;
} Change;

static void Do_GetChangeCustomScheme (void);

bool GetMarkerPos (API_Guid& markerguid, API_Coord& startpoint);

bool GetMarkerText (API_Guid& markerguid, GS::UniString& text, GS::UniString& number, GS::UniString& inx);

bool GetChangesMarker (GS::Array<Change>& changes);

bool GetAllChangesMarker (void);

void SetRevision (void);

#endif
