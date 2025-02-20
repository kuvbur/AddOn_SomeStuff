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
    double zBottom = 0; // Аболютная координата z низа
    double height = 0; // Высота проёма
    double width = 0; // Ширина проёма
    double objLoc = 0; // Расстояние от начала стены для середины проёма
    double lower = 0; // Привязка к низу стену 
    API_Guid base_guid = APINULLGuid; // GUID родительского окна
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки, в которую вставляется проём
} OtdOpening; // Проём в стене

typedef struct
{
    double height = 0; // Высота стены-отделки
    double zBottom = 0; // Аболютная координата z низа
    API_Coord begC; // Координата начала
    API_Coord endC; // Координата конца
    API_Guid base_guid = APINULLGuid; // GUID родительской стены
    API_Guid otd_guid = APINULLGuid; // GUID стены-отделки
    bool base_flipped = false; // Базовая стена отзеркалена (для чтения состава)
    GS::Array<ParamValueComposite> base_composite; // Состав базовой стены (только отделочные слои)
    GS::Array<OtdOpening> openings; // Проёмы в стене-отделке
    API_AttributeIndex material;
} OtdWall; // Структура со стенами для отделки

typedef struct
{
    double height = 0; // Высота зоны
    double zBottom = 0; // Аболютная координата z низа
    GS::Array<Sector> walledges; // Границы стен, не явяющихся границей зоны
    GS::Array<Sector> columnedges; // Границы колонн, не явяющихся границей зоны
    GS::Array<Sector> restedges; // Границы зоны
    GS::Array<Sector> gableedges;
    GS::Array<API_WallPart> wallPart; // Участки стен в зоне
    GS::Array<API_BeamPart> beamPart; // Участки балок в зоне
    GS::Array<API_CWSegmentPart> cwSegmentPart; // Навесные стены в зоне
    GS::Array<API_Niche> niches; // Ниши в зоне
    GS::Array<OtdWall> otd; // Стены-отделки, созданные для расчётов и отрисовки
    API_AttributeIndex material;
    bool isEmpty = true;
} OtdRoom; // Структура для хранения информации о зоне

typedef GS::HashTable<API_Guid, GS::Array<API_Guid>> UnicElement; // Словарь GUID элемента - массив GUID зон, где они встречаются
typedef GS::HashTable<API_ElemTypeID, UnicElement> UnicElementByType;
typedef GS::HashTable<API_ElemTypeID, GS::Array<API_Guid>> UnicGUIDByType;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();

void ClearZoneGUID (UnicElementByType& elementToRead, GS::Array<API_ElemTypeID>& typeinzone);

// -----------------------------------------------------------------------------
// Получение информации из зоны о полгионах и находящейся в ней элементах
// -----------------------------------------------------------------------------
bool CollectRoomInfo (const Stories& storyLevels, API_Guid& zoneGuid, OtdRoom& roominfo, UnicElementByType& elementToRead);

// -----------------------------------------------------------------------------
// Чтение данных об одном проёме
// -----------------------------------------------------------------------------
void ReadOneWinDoor (const Stories& storyLevels, const API_Guid& elGuid, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall);

// -----------------------------------------------------------------------------
// Создание стен-отделок для стен 
// -----------------------------------------------------------------------------
void ReadOneWall (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable<API_Guid, OtdRoom>& roomsinfo, GS::HashTable<API_Guid, GS::Array<OtdOpening>>& openinginwall, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Создание стен-отделок для колонны 
// -----------------------------------------------------------------------------
void ReadOneColumn (const Stories& storyLevels, API_Guid& elGuid, GS::Array<API_Guid>& zoneGuids, GS::HashTable<API_Guid, OtdRoom>& roomsinfo, UnicGUIDByType& guidselementToRead);

// -----------------------------------------------------------------------------
// Подготовка словаря с параметрами для чтения из элементов
// -----------------------------------------------------------------------------
void GetparamToRead (ParamDictValue& propertyParams, ParamDictValue& paramDict, ParamValue& param_composite);
// -----------------------------------------------------------------------------
// Получение очищенного полигона зоны, включая стены, колонны
// -----------------------------------------------------------------------------
void GetZoneEdges (API_Guid& zoneGuid, OtdRoom& roomedges);


bool FindOnEdge (Sector& edge, GS::Array<Sector>& edges, Sector& findedge);

// -----------------------------------------------------------------------------
// Проверка наличия отрезка в массиве отрезков.
// В случае нахождение проверяется направление, при необходимости разворачивается
// -----------------------------------------------------------------------------
bool FindEdge (Sector& edge, GS::Array<Sector>& edges);

void DrawEdges (const Stories& storyLevels, GS::HashTable < API_Guid, OtdRoom>& zoneelements, UnicElement& subelementByparent);
void DrawEdge (const Stories& storyLevels, OtdWall& edges, API_Element& wallelement, UnicElement& subelementByparent);
// -----------------------------------------------------------------------------
// Связывание созданных элементов отделки с базовыми элементами
// -----------------------------------------------------------------------------
void SetSyncOtdWall (UnicElement& subelementByparent, ParamDictValue& propertyParams);

void Do_CreateWindow (API_Element& wallelement, OtdOpening& op, UnicElement& subelementByparent);

}

#endif
