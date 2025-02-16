//------------ kuvbur 2022 ------------
#pragma once
#if !defined (AUTOMATE_HPP)
#define	AUTOMATE_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_26
#include	"Sector2DData.h"

namespace AutoFunc
{

// Структура со стенами для отделки
typedef struct
{
    double height = 0; // Угол подрезки конца отрезка
    Sector line;
    Int32 roomedge = 0;
} OtdWall;

typedef struct
{
    GS::Array<Sector> walledges;
    GS::Array<Sector> columnedges;
    GS::Array<Sector> restedges;
    GS::Array<Sector> gableedges;
    GS::Array<UnitVector_2D> gabledirection;
    bool isEmpty = true;
} OtdRoom;




// Структура с отрезками для создания 3д документов
typedef struct
{
    double angz = 0; // Угол отрезка
    double angz_1 = 0; // Угол подрезки начала отрезка
    double angz_2 = 0; // Угол подрезки конца отрезка
    Sector s; // Отрезок для построения 3д сечения
    API_DatabaseUnId databaseUnId; // UnId базы данных созданного или найденного 3д документа
    API_Tranmat tm; // Матрица преобразования координат, получена из API_3DProjectionInfo
} SSectLine;

// -----------------------------------------------------------------------------
// Запись в зону информации об отделке
// -----------------------------------------------------------------------------
void RoomBook ();
void ParseRoom (API_Guid& zoneGuid);
void GetZoneEdges (API_Guid& zoneGuid, OtdRoom& roomedges);
bool FindWall (Sector& walledge, OtdRoom& roomedges, double& toler);
bool FindEdge (Sector& edge, GS::Array<Sector> edges);
void DrawEdges (OtdRoom& roomedges);
void DrawEdge (GS::Array<Sector>& edges, API_Element& textelement, API_Element& lineelement, GS::UniString type);
// -----------------------------------------------------------------------------
// Ищет в массиве отрезок, начало или конец которого находятся возле точки start
// Возвращает индекс inx элемента в массиве, если точка была концом отрезка - поднимает флаг isend
// -----------------------------------------------------------------------------
bool GetNear (const GS::Array<Sector>& k, const Point2D& start, UInt32& inx, bool& isend);
// -----------------------------------------------------------------------------
// Устанавливает подрезку по отрезку, возвращает  API_3DCutPlanesInfo cutInfo
// -----------------------------------------------------------------------------
GSErrCode GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo);
// -----------------------------------------------------------------------------
// Устанавливает камеру перпендикулярно направлению angz, задаёт масштаб по осям x y
// -----------------------------------------------------------------------------
GSErrCode Get3DProjectionInfo (API_3DProjectionInfo& proj3DInfo, const double& angz, const double& koeff);
// -----------------------------------------------------------------------------
// Ищет 3д документ с именем и id. Если не находит - создаёт
// Возвращает информацию о БД API_DatabaseInfo& dbInfo
// -----------------------------------------------------------------------------
GSErrCode Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id);
// -----------------------------------------------------------------------------
// Извлекает из морфа отрезки, сортирует их по удалению от startpos
// Возвращает массив отрезков и ID морфа
// -----------------------------------------------------------------------------
GSErrCode GetSectLine (API_Guid& elemguid, GS::Array<SSectLine>& lines, GS::UniString& id, const  Point2D& startpos);
// -----------------------------------------------------------------------------
// Создание 3д документа для одного отрезка
// -----------------------------------------------------------------------------
GSErrCode DoSect (SSectLine& sline, const GS::UniString& name, const GS::UniString& id, const double& koeff);
// -----------------------------------------------------------------------------
// Размещение на созданном отрезке элементов оформления
// По краям отрезка устанавливаются hotspot
// -----------------------------------------------------------------------------
GSErrCode PlaceDocSect (SSectLine& sline, API_Element& elemline);
// -----------------------------------------------------------------------------
// Построение 3д документов вдоль морфа
// -----------------------------------------------------------------------------
void ProfileByLine ();
// -----------------------------------------------------------------------------
// Возвращает список API_Guid чертежей, отсортированных по именам
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetDrawingsSort (const GS::Array<API_Guid>& elems);
// -----------------------------------------------------------------------------
// Выравнивание одного чертежа
// Возвращает сдвинутую на ширину чертежа координату
// -----------------------------------------------------------------------------
GSErrCode AlignOneDrawingsByPoints (const API_Guid& elemguid, API_DatabaseInfo& databasestart, API_WindowInfo& windowstart, const API_Coord& zeropos, API_Coord& startpos, API_Coord& drawingpos);
// -----------------------------------------------------------------------------
// Выравнивание чертежей по расположенным в них hotspot
// -----------------------------------------------------------------------------
void AlignDrawingsByPoints ();
void KM_ListUpdate ();
GSErrCode KM_WriteGDL (API_Guid elemGuid, GS::Array<API_Coord>& coords);
}

#endif
