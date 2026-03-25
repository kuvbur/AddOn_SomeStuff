//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include "Spec_libpart.hpp"
namespace ListData
{
GS::UniString GetSubposKey (const GS::UniString& subpos)
{
    GS::UniString key = "";
    if (subpos == "-" || subpos.IsEmpty ()) {
        key = ATSIGN;
        return key;
    }
    key.Append (subpos); key.Append (ATSIGN);
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
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
    GS::UniString key = "";
    key.Append (m.pos); key.Append (ATSIGN);
    key.Append (m.obozn); key.Append (ATSIGN);
    key.Append (m.naen); key.Append (ATSIGN);
    key.Append (m.units); key.Append (ATSIGN);
    key.Append (m.tip_konstr); key.Append (ATSIGN);
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}


GS::UniString GetKey (const Arm& p)
{
    GS::UniString key = "";  //Контрольная сумма
    key.Append (p.pos); key.Append (ATSIGN);
    key.Append (p.klass); key.Append (ATSIGN);
    key.Append (GS::UniString::Printf ("%d_", p.diam)); key.Append (ATSIGN);
    if (p.isPm) {
        key.Append (GS::UniString::Printf ("%f_", p.dlin)); key.Append (ATSIGN);
    }
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
    key.SetToLowerCase ();
    return key;
}

GS::UniString GetKey (const Prokat& p)
{
    GS::UniString key = "";  //Контрольная сумма
    key.Append (p.pos); key.Append (ATSIGN);
    key.Append (p.tip_konstr); key.Append (ATSIGN);
    key.Append (p.obozn); key.Append (ATSIGN);
    key.Append (p.tip_profile); key.Append (ATSIGN);
    if (p.isPm) {
        key.Append (GS::UniString::Printf ("%f_", p.dlin)); key.Append (ATSIGN);
    }
    key.ReplaceAll (SPACESTRING, EMPTYSTRING);
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
        GS::UniString subpos = partstring[0];
        p.pos = partstring[2];
        p.tip_konstr = GetParam (partstring[5], "tk");
        p.naen = partstring[7];
        p.obozn = partstring[6];
        p.units = partstring[10];
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
        subpos = partstring[0];
        p.pos = partstring[2];
        p.klass = partstring[4];
        if (!UniStringToDouble (partstring[6], p.diam)) return;
        if (!UniStringToDouble (partstring[7], p.dlin)) return;
        if (partstring[8] == "1") p.isPm = true;
        p.ves = qty;
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
    GS::UniString units = "";  // Ед. измерения
    if (version == 3) {
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
        subpos = partstring[0];
        p.pos = partstring[2];
        p.tip_konstr = partstring[6];
        p.obozn_mater = partstring[7];
        p.mater = partstring[8];
        p.obozn = partstring[10];
        p.tip_profile = partstring[11];
        if (!UniStringToDouble (partstring[9], p.qty)) p.qty = 1;
        if (!UniStringToDouble (partstring[12], p.dlin)) p.dlin = 0;
        if (!UniStringToDouble (partstring[14], p.ves_t)) p.ves_t = qty;
        p.ves = qty;
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



}
