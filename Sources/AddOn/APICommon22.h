// *****************************************************************************
// Helper functions for Add-On development
// API Development Kit 22; Mac/Win
//
// Namespaces:		Contact person:
//		-None-
//
// [SG compatible] - Yes
// *****************************************************************************

#ifndef	_APICOMMON_H_
#define	_APICOMMON_H_

#include "GSRoot.hpp"
#include "UniString.hpp"

#if PRAGMA_ENUM_ALWAYSINT
#pragma enumsalwaysint on
#elif PRAGMA_ENUM_OPTIONS
#pragma options enum=int
#endif



/* -- Messages ------------------------------- */

void CCALL	WriteReport (const char* format, ...);
void CCALL	WriteReport_Alert (const char* format, ...);
void CCALL	WriteReport_Err (const char* info, GSErrCode err);
void CCALL	WriteReport_End (GSErrCode err);

void 		ErrorBeep (const char* info, GSErrCode err);


/* -- Conversions ---------------------------- */

API_ElemTypeID			Neig_To_ElemID (API_NeigID neigID);

bool					ElemHead_To_Neig (API_Neig* neig, const API_Elem_Head* elemHead);

const char* ErrID_To_Name (GSErrCode err);
const char* LibID_To_Name (API_LibTypeID typeID);
const char* AttrID_To_Name (API_AttrTypeID typeID);
const GS::UniString		ElemID_To_Name (API_ElemTypeID typeID);


/* -- Interface support ---------------------- */

bool		ClickAPoint (const char* prompt,
                         API_Coord* c);

bool		GetAnArc (const char* prompt,
                      API_Coord* origin,
                      API_Coord* startPos,
                      API_Coord* endPos,
                      bool* isArcNegative = nullptr);

bool		ClickAnElem (const char* prompt,
                         API_ElemTypeID		needTypeID,
                         API_Neig* neig = nullptr,
                         API_ElemTypeID* typeID = nullptr,
                         API_Guid* guid = nullptr,
                         API_Coord3D* c = nullptr,
                         bool				ignorePartialSelection = true);


API_Neig** ClickElements_Neig (const char* prompt,
                                API_ElemTypeID	needTypeID,
                                Int32* nItem);

API_Elem_Head** ClickElements_ElemHead (const char* prompt,
                                        API_ElemTypeID	needTypeID,
                                        Int32* nItem);

bool		GetMenuItemMark (short menuResID, short itemIndex);
bool		InvertMenuItemMark (short menuResID, short itemIndex);
void		DisableEnableMenuItem (short menuResID, short itemIndex, bool disable);


/* -- Geometry support ----------------------- */

Int32		FindArc (const API_PolyArc* parcs, Int32 nArcs, Int32 node);
bool		ArcGetOrigo (const API_Coord* begC, const API_Coord* endC, double angle, API_Coord* origo);
double		ComputeFiPtr (const API_Coord* c1, const API_Coord* c2, bool enableNegativeAngle = false);
double		DistCPtr (const API_Coord* c1, const API_Coord* c2);


#if PRAGMA_ENUM_ALWAYSINT
#pragma enumsalwaysint reset
#elif PRAGMA_ENUM_OPTIONS
#pragma options enum=reset
#endif

#endif
