//------------ kuvbur 2022 ------------
#pragma once
#ifdef TESTING
#if !defined(TEST_HPP)
#define TEST_HPP
#ifdef AC_25
#include "APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include "APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include "APICommon27.h"
#endif // AC_26

namespace TestFunc {
    void Test ();
    void TestCalc ();
    void TestFormula ();
    void TestFormatString ();
    void ResetSyncPropertyArray (GS::Array<API_Guid> guidArray);
    void ResetSyncPropertyOne (const API_Guid& elemGuid);
    void ResetSyncPropertyOne (const API_Guid& elemGuid, GS::Array<API_Property>& propertywrite);
}

#endif
#endif
