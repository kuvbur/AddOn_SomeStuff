//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "Helpers.hpp"
#include "Spec_libpart.hpp"
namespace ListData
{
GS::UniString GetSubposKey (const GS::UniString& subpos)
{
    GS::UniString key = "45@";
    key.Append (subpos); key.Append (ATSIGN);
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}

GS::UniString GetKey (const LibElement& el)
{
    GS::UniString key = GetSubposKey (el.pos);
    return key;
}

GS::UniString GetKey (const Subpos& p)
{
    GS::UniString key = GetSubposKey (p.pos);
    return key;
}

GS::UniString GetKey (const Mat& m)
{
    GS::UniString key = "30@";
    key.Append (m.pos); key.Append (ATSIGN);
    key.Append (m.obozn); key.Append (ATSIGN);
    key.Append (m.naen); key.Append (ATSIGN);
    key.Append (m.unit); key.Append (ATSIGN);
    key.Append (m.tip_konstr); key.Append (ATSIGN);
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}


GS::UniString GetKey (const Arm& p)
{
    GS::UniString key = "10@";  //Контрольная сумма
    key.Append (p.pos); key.Append (ATSIGN);
    key.Append (p.klass); key.Append (ATSIGN);
    key.Append (GS::UniString::Printf ("%d_", p.diam)); key.Append (ATSIGN);
    if (p.isPm) {
        key.Append (GS::UniString::Printf ("%f_", p.dlin)); key.Append (ATSIGN);
    }
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}

GS::UniString GetKey (const Prokat& p)
{
    GS::UniString key = "20@";  //Контрольная сумма
    key.Append (p.pos); key.Append (ATSIGN);
    key.Append (p.tip_konstr); key.Append (ATSIGN);
    key.Append (p.obozn); key.Append (ATSIGN);
    key.Append (p.tip_profile); key.Append (ATSIGN);
    if (p.isPm) {
        key.Append (GS::UniString::Printf ("%f_", p.dlin)); key.Append (ATSIGN);
    }
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}

GS::UniString GetParam (const GS::UniString& param_zone, GS::UniString param_name)
{
    GS::UniString param = "";
    if (!param_zone.Contains (param_name)) return param;
    if (!param_zone.Contains (ATSIGN)) return param;
    if (!param_zone.Contains ("=")) return param;
    GS::Array<GS::UniString> partstring = {};
    UInt32 n = StringSplt (param_zone, ATSIGN, partstring, param_name);
    param = partstring[0];
    param.ReplaceAll (param_name, EMPTYSTRING);
    param.ReplaceAll ("=", EMPTYSTRING);
    param.Trim ();
    return param;
}

void AddMat (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version)
{
    Mat p = {};
    GS::UniString subpos = "";
    if (version == 3) {
        if (partstring.GetSize () < 10) {
            return;
        }
        GS::UniString subpos = partstring[0];
        p.pos = partstring[2];
        p.tip_konstr = GetParam (partstring[5], "tk");
        p.naen = partstring[7];
        p.obozn = partstring[6];
        p.unit = partstring[10];
        if (!UniStringToDouble (partstring[9], p.qty)) return;
    }
    if (version == 4) {
        return;
    }
    GS::UniString subposkey = GetSubposKey (subpos);
    if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
        paramListDataToRead.subpos.Add (subposkey, {});
    }
    Subpos& s = paramListDataToRead.subpos.Get (subposkey);
    p.key = GetKey (p);
    if (s.mat.ContainsKey (p.key)) {
        Mat& m = s.mat.Get (p.key);
        m.qty += p.qty;
    } else {
        s.mat.Add (p.key, p);
    }
}


void AddArm (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version)
{
    Arm p = {};
    GS::UniString subpos = "";
    if (version == 3) {
        if (partstring.GetSize () < 8) {
            return;
        }
        subpos = partstring[0];
        p.pos = partstring[2];
        p.klass = partstring[6];
        if (!UniStringToDouble (partstring[7], p.diam)) return;
        if (!UniStringToDouble (partstring[8], p.dlin)) return;
        p.dlin = p.dlin / 1000.0; // Перевод в метры
        if (partstring[9] == "1") p.isPm = true;
        p.ves = qty;
    }
    if (version == 4) {
        return;
    }
    p.naen = GS::UniString::Printf ("d%d ", (int) p.diam);
    p.naen.Append (p.klass);
    if (p.isPm) {
        p.naen.Append ("  L = п.м.");
    } else {
        p.naen.Append (GS::UniString::Printf ("  L = %d ", DoubleM2IntMM (p.dlin)));
    }
    GS::UniString subposkey = GetSubposKey (subpos);
    if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
        paramListDataToRead.subpos.Add (subposkey, {});
    }
    Subpos& s = paramListDataToRead.subpos.Get (subposkey);
    p.key = GetKey (p);
    if (s.arm.ContainsKey (p.key)) {
        Arm& m = s.arm.Get (p.key);
        m.qty += p.qty;
        if (m.isPm) {
            m.dlin += p.dlin;
        }
    } else {
        s.arm.Add (p.key, p);
    }
}

void AddSubpos (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version)
{
    GS::UniString subpos = "";
    GS::UniString pos = "";  // Позиция
    GS::UniString obozn = "";  // ГОСТ
    GS::UniString naen = "";  // Наименование
    double _qty = 0; // Количество
    double ves = 0; // Масса ед.
    GS::UniString unit = "";  // Ед. измерения
    if (version == 3) {
        if (partstring.GetSize () < 9) {
            return;
        }
        subpos = partstring[0];
        pos = partstring[2];
        obozn = partstring[6];
        naen = partstring[7];
        if (!UniStringToDouble (partstring[9], _qty)) _qty = 1;
        if (!UniStringToDouble (partstring[9], ves)) ves = 0;
    }
    if (version == 4) {
        return;
    }
    GS::UniString subposkey = GetSubposKey (subpos);
    if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
        paramListDataToRead.subpos.Add (subposkey, {});
    }
    Subpos& s = paramListDataToRead.subpos.Get (subposkey);
    s.key = subposkey;
    if (!pos.IsEmpty ()) s.pos = pos;
    if (!obozn.IsEmpty ()) s.obozn = obozn;
    if (!naen.IsEmpty ()) s.naen = naen;
    s.qty += _qty;
    if (!is_equal (ves, 0)) s.ves = ves;
}

void AddProkat (LibElement& paramListDataToRead, const GS::Array<GS::UniString>& partstring, GS::UniString& unitcode, double& qty, const short& version)
{
    Prokat p = {};
    GS::UniString subpos = "";
    if (version == 3) {
        if (partstring.GetSize () < 14) {
            return;
        }
        subpos = partstring[0];
        p.pos = partstring[2];
        p.tip_konstr = partstring[6];
        p.obozn_mater = partstring[7];
        p.mater = partstring[8];
        p.obozn = partstring[10];
        p.tip_profile = partstring[11];
        if (!UniStringToDouble (partstring[9], p.qty)) p.qty = 1;
        if (!UniStringToDouble (partstring[12], p.dlin)) p.dlin = 0;
        p.dlin = p.dlin / 1000.0; // Перевод в метры
        if (!UniStringToDouble (partstring[14], p.ves_t)) p.ves_t = qty;
        p.ves = qty;
    }
    if (version == 4) {
        return;
    }
    p.naen = p.tip_profile;
    p.naen.Append (SPACESTRING);
    if (p.isPm) {
        p.naen.Append ("  L = п.м.");
    } else {
        p.naen.Append (GS::UniString::Printf ("  L = %d", DoubleM2IntMM (p.dlin)));
    }
    GS::UniString subposkey = GetSubposKey (subpos);
    if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
        paramListDataToRead.subpos.Add (subposkey, {});
    }
    Subpos& s = paramListDataToRead.subpos.Get (subposkey);
    p.key = GetKey (p);
    if (s.prokat.ContainsKey (p.key)) {
        Prokat& m = s.prokat.Get (p.key);
        m.qty += p.qty;
        if (m.isPm) {
            m.dlin += p.dlin;
        }
    } else {
        s.prokat.Add (p.key, p);
    }
}

GS::Array<GS::Pair<GS::UniString, GS::UniString>> GetAllKeys (const LibElement& el)
{
    #if defined(TESTING)
    DBprnt ("        Set keys for List Data");
    #endif
    GS::Array<GS::Pair<GS::UniString, GS::UniString>> keys = {};
    if (el.subpos.IsEmpty ()) return keys;
    for (const auto& sub : el.subpos) {
        #if defined(AC_28) || defined(AC_29)
        const Subpos& subpos = sub.value;
        const GS::UniString subposkey = sub.key;
        #else
        const Subpos& subpos = *sub.value;
        const GS::UniString subposkey = *sub.key;
        #endif
        if (!subpos.arm.IsEmpty ()) {
            for (const auto& arm : subpos.arm) {
                #if defined(AC_28) || defined(AC_29)
                const GS::UniString k = arm.key;
                #else
                const GS::UniString k = *arm.key;
                #endif
                keys.Push (GS::Pair<GS::UniString, GS::UniString> (subposkey, k));
            }
        }
        if (!subpos.mat.IsEmpty ()) {
            for (const auto& mat : subpos.mat) {
                #if defined(AC_28) || defined(AC_29)
                const GS::UniString k = mat.key;
                #else
                const GS::UniString k = *mat.key;
                #endif
                keys.Push (GS::Pair<GS::UniString, GS::UniString> (subposkey, k));
            }
        }
        if (!subpos.prokat.IsEmpty ()) {
            for (const auto& prokat : subpos.prokat) {
                #if defined(AC_28) || defined(AC_29)
                const GS::UniString k = prokat.key;
                #else
                const GS::UniString k = *prokat.key;
                #endif
                keys.Push (GS::Pair<GS::UniString, GS::UniString> (subposkey, k));
            }
        }
    }
    return keys;
}

void Add (LibElement& paramListDataToRead, GS::UniString& name, GS::UniString& unitcode, double& qty)
{
    short version = 0;
    if (name.Contains ("v3%%")) version = 3;
    if (name.Contains ("v4%%")) version = 4;
    if (version == 0) return;
    GS::Array<GS::UniString> partstring = {};
    UInt32 n = StringSplt_ (name, SEMICOLON, partstring, false);
    if (n < 1) return;
    GS::UniString tip_el = "";
    if (version == 3 || version == 4) tip_el = partstring[1];
    if (tip_el.IsEmpty () || tip_el.GetLength () != 2) return;
    if (tip_el == "10") AddArm (paramListDataToRead, partstring, unitcode, qty, version);
    if (tip_el == "20") AddProkat (paramListDataToRead, partstring, unitcode, qty, version);
    if (tip_el == "30") AddMat (paramListDataToRead, partstring, unitcode, qty, version);
    if (tip_el == "40") AddMat (paramListDataToRead, partstring, unitcode, qty, version);
    if (tip_el == "45") AddSubpos (paramListDataToRead, partstring, unitcode, qty, version);
    return;
}

bool AddLibdataToParamValueDict (const API_Guid& elemguid, const GS::Int32& n_layer, const LibElements& paramListDataToRead, const GS::UniString& rawname, ParamDictValue& params)
{
    if (!paramListDataToRead.ContainsKey (elemguid)) return false;
    const LibElement& p = paramListDataToRead.Get (elemguid);
    if (p.keys.IsEmpty ()) return false;
    GS::Int32 max_layers = p.keys.GetSize ();
    if (n_layer >= max_layers) {
        return false;
    }
    if (max_layers >= max_group_lib) {
        msg_rep ("Spec err", GS::UniString::Printf ("Max libdata over critical - %d", max_layers), APIERR_GENERAL, elemguid);
    }
    const GS::Pair<GS::UniString, GS::UniString>& keys = p.keys[n_layer];
    if (!p.subpos.ContainsKey (keys.first)) {
        return false;
    }
    const Subpos& s = p.subpos.Get (keys.first);
    if (keys.second.BeginsWith ("10@") && (rawname.Contains ("{@listdata:arm.") || rawname.Contains ("{@listdata:elem."))) {
        if (!s.arm.ContainsKey (keys.second)) {
            return false;
        }
        const Arm& arm = s.arm.Get (keys.second);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.pos", arm.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.unit", arm.unit, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.klass", arm.klass, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.ves_t", arm.ves_t, true);
        ParamHelpers::AddLengthValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.dlin", arm.dlin, true);
        ParamHelpers::AddLengthValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.diam", arm.diam, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.naen", arm.naen, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.pos", arm.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.unit", arm.unit, true);
        if (arm.isPm) {
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.qty", arm.dlin, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.ves", arm.ves_t, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.qty", arm.dlin, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.ves", arm.ves_t, true);
        } else {
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.qty", arm.qty, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "arm.ves", arm.ves, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.qty", arm.qty, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.ves", arm.ves, true);
        }
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.type", "arm", true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.naen", arm.naen, true);
        return true;
    }
    if (keys.second.BeginsWith ("20@") && (rawname.Contains ("{@listdata:prokat.") || rawname.Contains ("{@listdata:elem."))) {
        if (!s.prokat.ContainsKey (keys.second)) {
            return false;
        }
        const Prokat& prokat = s.prokat.Get (keys.second);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.pos", prokat.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.tip_konstr", prokat.tip_konstr, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.obozn_mater", prokat.obozn_mater, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.mater", prokat.mater, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.obozn", prokat.obozn, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.tip_profile", prokat.tip_profile, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.unit", prokat.obozn_mater, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.naen", prokat.naen, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.ves_t", prokat.ves_t, true);
        ParamHelpers::AddLengthValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.dlin", prokat.dlin, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.type", "prokat", true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.pos", prokat.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.obozn", prokat.obozn, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.unit", prokat.obozn_mater, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.naen", prokat.naen, true);
        if (prokat.isPm) {
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.ves", prokat.ves_t, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.ves", prokat.ves_t, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.qty", prokat.dlin, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.qty", prokat.dlin, true);
        } else {
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.ves", prokat.ves, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.ves", prokat.ves, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "prokat.qty", prokat.qty, true);
            ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.qty", prokat.qty, true);
        }
        return true;
    }
    if (keys.second.BeginsWith ("30@") && (rawname.Contains ("{@listdata:mat.") || rawname.Contains ("{@listdata:elem."))) {
        if (!s.mat.ContainsKey (keys.second)) {
            return false;
        }
        const Mat& mat = s.mat.Get (keys.second);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.pos", mat.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.tip_konstr", mat.tip_konstr, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.obozn", mat.obozn, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.naen", mat.naen, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.unit", mat.unit, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.qty", mat.qty, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "mat.ves", mat.ves, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.pos", mat.pos, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.obozn", mat.obozn, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.naen", mat.naen, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.unit", mat.unit, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.qty", mat.qty, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.ves", mat.ves, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.type", "mat.object", true);
        return true;
    }
    if (rawname.Contains ("{@listdata:subpos.")) {
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.obozn", s.obozn, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.naen", s.naen, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.unit", s.unit, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.ves", s.ves, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.pos", s.pos, true);
        ParamHelpers::AddDoubleValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "subpos.qty", s.qty, true);
        ParamHelpers::AddStringValueToParamDictValue (params, elemguid, LISTDATANAMEPREFIX, "elem.type", "subpos", true);
        return true;
    }
    return false;
}

}
