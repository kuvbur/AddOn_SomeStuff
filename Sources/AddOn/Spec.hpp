//------------ kuvbur 2022 ------------
#pragma once
#if !defined (SPEC_HPP)
#define	SPEC_HPP
#ifdef AC_25
#include	"APICommon25.h"
#endif // AC_25
#ifdef AC_26
#include	"APICommon26.h"
#endif // AC_26
#ifdef AC_27
#include	"APICommon27.h"
#endif // AC_27
#ifdef AC_28
#include	"APICommon28.h"
#endif // AC_28
#include	"Helpers.hpp"

namespace Spec {

    typedef GS::HashTable<GS::UniString, SortGUID> SpecDict;


    typedef struct {
        GS::Array<GS::UniString> unic_paramrawname;
        GS::Array<GS::UniString> sum_paramrawname;
        bool is_Valid = true;
    } GroupSpec;

    typedef struct {
        GS::Array<ParamValue> out_unic_paramrawname;
        GS::Array<ParamValue> out_sum_paramrawname;
        GS::Array<API_Guid> elements;
    } Element;
    typedef GS::HashTable<GS::UniString, Element> ElementDict;

    typedef struct {
        GS::Array<GroupSpec> groups;
        GS::Array<GS::UniString> out_unic_paramrawname;
        GS::Array<GS::UniString> out_sum_paramrawname;
        GS::Array<API_Guid> elements;
        bool is_Valid = true;
    } SpecRule;

    typedef GS::HashTable<GS::UniString, SpecRule> SpecRuleDict;

    GSErrCode SpecAll (const SyncSettings& syncSettings);
    GSErrCode SpecArray (const SyncSettings& syncSettings, GS::Array<API_Guid>& guidArray);
    GSErrCode GetRuleFromElement (const API_Guid& elemguid, SpecRuleDict& rules);
    SpecRule GetRuleFromDescription (GS::UniString& description);
}

#endif
