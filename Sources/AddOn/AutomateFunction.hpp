//------------ kuvbur 2022 ------------
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

typedef struct {
    double angz = 0;
    double angz_1 = 0;
    double angz_2 = 0;
    Sector s;
    API_DatabaseUnId databaseUnId;
    API_Tranmat tm;
} SSectLine;

namespace AutoFunc {
    bool GetNear (const GS::Array<Sector>& k, const Point2D& start, UInt32& inx, bool& isend);
    double angle (API_Coord3D& begC1, API_Coord3D& endC1, API_Coord3D& begC2, API_Coord3D& endC2);
    GSErrCode GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo);
    GSErrCode Get3DProjectionInfo (API_3DProjectionInfo& proj3DInfo, double& angz);
    GSErrCode Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id);
    GSErrCode GetSectLine (API_Guid& elemguid, GS::Array<SSectLine>& lines, GS::UniString& id, Point2D& startpos);
    GSErrCode DoSect (SSectLine& sline, const GS::UniString& name, const GS::UniString& id);
    GSErrCode PlaceDocSect (SSectLine& sline, API_Element& elemline);
    void ProfileByLine ();
    GSErrCode AlignOneDrawingsByPoints (API_Guid& elemguid, Point2D& startpos, API_DatabaseInfo& databasestart, API_WindowInfo& windowstart);
    void AlignDrawingsByPoints ();
    void KM_ListUpdate ();
    GSErrCode KM_WriteGDLValues (API_Guid elemGuid, GS::Array<API_Coord>& coords);
}

#endif
