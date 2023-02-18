//------------ kuvbur 2022 ------------
#pragma once
#ifndef CWALL_HPP
#define	CWALL_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#include	"DG.h"
#include	"Helpers.hpp"

typedef struct {
	API_Box3D coord = {};
	double ang = 0;
} BoxElem;

void AddHoleToSelectedCWall(const SyncSettings& syncSettings);

void Do_ChangeCWallWithUndo(const GS::Array<API_Guid>& elemsGuid, const GS::Array<BoxElem>& elemsCoord);

void Do_ChangeCWall(const API_Guid& elemGuid, const  GS::Array<BoxElem>& elemsCoord);

#endif