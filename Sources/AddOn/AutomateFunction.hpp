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

typedef struct {
    double angz = 0;
    double angz_1 = 0;
    double angz_2 = 0;
    API_Coord begC = {0,0};
    API_Coord endC = {0,0};
} SSectLine;

namespace AutoFunc {
    bool GetCuplane (const SSectLine sline, API_3DCutPlanesInfo& cutInfo);
    bool Get3DProjectionInfo (API_3DProjectionInfo& proj3DInfo, double& angz);
    bool Get3DDocument (API_DatabaseInfo& dbInfo, const GS::UniString& name, const GS::UniString& id);
    void ProfileByLine ();
    void KM_ListUpdate ();
    GSErrCode KM_WriteGDLValues (API_Guid elemGuid, GS::Array<API_Coord>& coords);
}

#endif
