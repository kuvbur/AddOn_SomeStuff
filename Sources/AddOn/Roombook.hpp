//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ROOMBOOK_HPP)
#define	ROOMBOOK_HPP
#include    "Helpers.hpp"
#include	"Sector2DData.h"
namespace AutoFunc
{

typedef struct
{
    double height = 0;
    double width = 0;
    double objLoc = 0;
    double lower = 0;
    API_Guid base_guid = APINULLGuid;
    API_Guid otd_guid = APINULLGuid;
} OtdOpening; // Проём в стене

typedef struct
{
    double height = 0;
    double bottomOffset = 0;
    bool flipped = false;
    GS::Array<Sector> edges;
} OtdElement; // Проём в стене

typedef struct
{
    double height = 0;
    double bottomOffset = 0;
    API_Coord begC;
    API_Coord endC;
    API_Guid base_guid = APINULLGuid;
    API_Guid otd_guid = APINULLGuid;
    bool base_flipped = false;
    GS::Array<ParamValueComposite> base_composite;
    GS::Array<OtdOpening> openings;
} OtdWall; // Структура со стенами для отделки

typedef struct
{
    GS::Array<Sector> walledges;
    GS::Array<Sector> columnedges;
    GS::Array<Sector> restedges;
    GS::Array<Sector> gableedges;
    GS::Array<API_WallPart> wallPart;
    GS::Array<API_BeamPart> beamPart;
    GS::Array<API_CWSegmentPart> cwSegmentPart;
    GS::Array<API_Niche> niches;
    GS::Array<OtdWall> otd;
    bool isEmpty = true;
} OtdRoom;

typedef GS::HashTable<API_Guid, GS::Array<API_Guid>> UnicElement; // Словарь GUID элемента - GUID зоны
typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();

bool CollectRoomInfo (API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead);

void ReadOneWall (API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable<API_Guid, OtdRoom>& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall);

// -----------------------------------------------------------------------------
// Полчение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void GetZoneEdges (API_Guid& zoneGuid, OtdRoom& roomedges);

bool FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool FindEdge (Sector& edge, GS::Array<Sector>& edges);

// -----------------------------------------------------------------------------
// Чтение данных об одной колонне
// -----------------------------------------------------------------------------
void ReadOneColumn (API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable<API_Guid, OtdRoom>& roomsinfo);

// -----------------------------------------------------------------------------
// Чтение данных об одном проёме
// -----------------------------------------------------------------------------
bool ReadOneWinDoor (const API_Guid& elGuid, API_Guid& wallguid, OtdOpening& op);

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из элементов
// -----------------------------------------------------------------------------
void GetparamToRead (ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite);


void DrawEdges (GS::HashTable < API_Guid, OtdRoom>& zoneelements);
void DrawEdge (OtdWall& edges, API_Element& wallelement);

void Do_CreateWindow (API_Element& wallelement, OtdOpening& op);

}

#endif
