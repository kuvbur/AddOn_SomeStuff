//------------ kuvbur 2022 ------------
#include "ACAPinc.h"

#include "api_headers/APIEnvir.h"

#include "spec/Spec_libpart.hpp"

#include "Helpers.hpp"

namespace ListData {
    // --------------------------------------------------------------------
    // GetSubposKey
    // Формирует ключ подпозиции: префикс '45@', затем код подпозиции и
    // завершающий символ @. Пробелы и дефисы удаляются, регистр приводится
    // к нижнему.
    // --------------------------------------------------------------------
    GS::UniString GetSubposKey (const GS::UniString &subpos) {
        GS::UniString key = "45@";
        key.Append (subpos);
        key.Append (ATSIGN);
        key.ReplaceAll (SPACESTRING, EMPTYSTRING);
        key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
        key.SetToLowerCase ();
        return key;
    }

    // --------------------------------------------------------------------
    // GetKey (const LibElement &)
    // Ключ записи библиотечного элемента: ключ его подпозиции
    // (вызов GetSubposKey для el.pos).
    // --------------------------------------------------------------------
    GS::UniString GetKey (const LibElement &el) {
        GS::UniString key = GetSubposKey (el.pos);
        return key;
    }

    // --------------------------------------------------------------------
    // GetKey (const Subpos &)
    // Ключ подпозиции: ключ её кода позиции (вызов GetSubposKey для p.pos).
    // --------------------------------------------------------------------
    GS::UniString GetKey (const Subpos &p) {
        GS::UniString key = GetSubposKey (p.pos);
        return key;
    }

    // --------------------------------------------------------------------
    // GetKey (const Mat &)
    // Ключ материала: префикс '30@' и через @ поля pos, obozn, naen,
    // unit, tip_konstr. Пробелы и дефисы удаляются, регистр приводится к нижнему.
    // --------------------------------------------------------------------
    GS::UniString GetKey (const Mat &m) {
        GS::UniString key = "30@";
        key.Append (m.pos);
        key.Append (ATSIGN);
        key.Append (m.obozn);
        key.Append (ATSIGN);
        key.Append (m.naen);
        key.Append (ATSIGN);
        key.Append (m.unit);
        key.Append (ATSIGN);
        key.Append (m.tip_konstr);
        key.Append (ATSIGN);
        key.ReplaceAll (SPACESTRING, EMPTYSTRING);
        key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
        key.SetToLowerCase ();
        return key;
    }

    // --------------------------------------------------------------------
    // GetKey (const Arm &)
    // Ключ арматуры: префикс '10@' и через @ поля pos, klass, diam.
    // Для погонного метра (isPm) дополнительно добавляется dlin. Пробелы и
    // дефисы удаляются, регистр приводится к нижнему.
    // --------------------------------------------------------------------
    GS::UniString GetKey (const Arm &p) {
        GS::UniString key = "10@";
        key.Append (p.pos);
        key.Append (ATSIGN);
        key.Append (p.klass);
        key.Append (ATSIGN);
        key.Append (GS::UniString::Printf ("%d_", (int)p.diam));
        key.Append (ATSIGN);
        if (p.isPm) {
            key.Append (GS::UniString::Printf ("%f_", p.dlin));
            key.Append (ATSIGN);
        }
        key.ReplaceAll (SPACESTRING, EMPTYSTRING);
        key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
        key.SetToLowerCase ();
        return key;
    }

    // --------------------------------------------------------------------
    // GetKey (const Prokat &)
    // Ключ проката: префикс '20@' и через @ поля pos, tip_konstr, obozn,
    // tip_profile, а для погонного метра (isPm) — dlin. Пробелы и дефисы
    // удаляются, регистр приводится к нижнему.
    // --------------------------------------------------------------------
    GS::UniString GetKey (const Prokat &p) {
        GS::UniString key = "20@";
        key.Append (p.pos);
        key.Append (ATSIGN);
        key.Append (p.tip_konstr);
        key.Append (ATSIGN);
        key.Append (p.obozn);
        key.Append (ATSIGN);
        key.Append (p.tip_profile);
        key.Append (ATSIGN);
        if (p.isPm) {
            key.Append (GS::UniString::Printf ("%f_", p.dlin));
            key.Append (ATSIGN);
        }
        key.ReplaceAll (SPACESTRING, EMPTYSTRING);
        key.ReplaceAll (MINUSSTRING, EMPTYSTRING);
        key.SetToLowerCase ();
        return key;
    }

    // --------------------------------------------------------------------
    // GetParam
    // Извлекает значение параметра с именем param_name из строки param_zone.
    // Возвращает пустую строку, если в строке нет param_name, символа @
    // или знака '='. Иначе строка делится по @, из первой части удаляются
    // имя параметра и '=', результат обрезается от пробелов.
    // --------------------------------------------------------------------
    GS::UniString GetParam (const GS::UniString &param_zone, GS::UniString param_name) {
        GS::UniString param;
        if (!param_zone.Contains (param_name))
            return param;
        if (!param_zone.Contains (ATSIGN))
            return param;
        if (!param_zone.Contains ("="))
            return param;
        GS::Array<GS::UniString> partstring = {};
        UInt32 n = StringSpltFilter (param_zone, ATSIGN, partstring, param_name);
        param = partstring[0];
        param.ReplaceAll (param_name, EMPTYSTRING);
        param.ReplaceAll ("=", EMPTYSTRING);
        param.Trim ();
        return param;
    }

    // --------------------------------------------------------------------
    // AddMat
    // Добавляет материал из разобранной строки описания в элемент списка.
    // Требует версию 3 и не менее 11 полей. Поля: 0 — подпозиция, 2 —
    // позиция, 5 — тип конструкции (параметр 'tk'), 6 — обозначение,
    // 7 — наименование, 9 — количество, 10 — единица измерения.
    // Запись добавляется в подпозицию по ключу subpos: при повторе
    // ключа количество суммируется. Версия 4 не обрабатывается (выход).
    // Параметры unitcode и qty не используются.
    // --------------------------------------------------------------------
    void AddMat (LibElement &paramListDataToRead,
                 const GS::Array<GS::UniString> &partstring,
                 GS::UniString &unitcode,
                 double &qty,
                 const short &version) {
        Mat p = {};
        GS::UniString subpos;
        if (version == 3) {
            if (partstring.GetSize () < 11) {
                return;
            }
            subpos = partstring[0];
            p.pos = partstring[2];
            p.tip_konstr = GetParam (partstring[5], "tk");
            p.naen = partstring[7];
            p.obozn = partstring[6];
            p.unit = partstring[10];
            if (!UniStringToDouble (partstring[9], p.qty))
                return;
        }
        if (version == 4) {
            return;
        }
        GS::UniString subposkey = GetSubposKey (subpos);
        if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
            paramListDataToRead.subpos.Add (subposkey, {});
        }
        Subpos &s = paramListDataToRead.subpos.Get (subposkey);
        p.key = GetKey (p);
        if (s.mat.ContainsKey (p.key)) {
            Mat &m = s.mat.Get (p.key);
            m.qty += p.qty;
        } else {
            s.mat.Add (p.key, p);
        }
    }

    // --------------------------------------------------------------------
    // AddArm
    // Добавляет арматуру из разобранной строки описания в элемент списка.
    // Требует версию 3 и не менее 10 полей. Поля: 0 — подпозиция, 2 —
    // позиция, 6 — класс, 7 — диаметр, 8 — длина (мм, переводится в метры),
    // 9 — признак погонного метра. ves берётся из qty. При пустой единице
    // используется unitcode. Наименование формируется как 'd<диаметр>',
    // класс и 'L = п.м.' либо 'L = <длина> мм'. Запись добавляется в
    // подпозицию по ключу subpos: при повторе ключа суммируются количество
    // и, для погонного метра, длина.
    // --------------------------------------------------------------------
    void AddArm (LibElement &paramListDataToRead,
                 const GS::Array<GS::UniString> &partstring,
                 GS::UniString &unitcode,
                 double &qty,
                 const short &version) {
        Arm p = {};
        GS::UniString subpos;
        if (version == 3) {
            if (partstring.GetSize () < 10) {
                return;
            }
            subpos = partstring[0];
            p.pos = partstring[2];
            p.klass = partstring[6];
            if (!UniStringToDouble (partstring[7], p.diam))
                return;
            if (!UniStringToDouble (partstring[8], p.dlin))
                return;
            p.dlin = p.dlin / 1000.0; // Перевод в метры
            if (partstring[9] == "1")
                p.isPm = true;
            p.ves = qty;
        }
        if (version == 4) {
            return;
        }
        if (p.unit.IsEmpty ())
            p.unit = unitcode;
        p.naen = GS::UniString::Printf ("d%d ", (int)p.diam);
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
        Subpos &s = paramListDataToRead.subpos.Get (subposkey);
        p.key = GetKey (p);
        if (s.arm.ContainsKey (p.key)) {
            Arm &m = s.arm.Get (p.key);
            m.qty += p.qty;
            if (m.isPm) {
                m.dlin += p.dlin;
            }
        } else {
            s.arm.Add (p.key, p);
        }
    }

    // --------------------------------------------------------------------
    // AddSubpos
    // Добавляет данные подпозиции из разобранной строки описания.
    // Требует версию 3 и не менее 10 полей. Поля: 0 — подпозиция, 2 —
    // позиция, 6 — обозначение, 7 — наименование, 9 — количество и масса.
    // Запись подпозиции создаётся при первом обращении к ключу. Непустые
    // позиция, обозначение и наименование записываются в поле, количество
    // прибавляется, ненулевая масса заменяет текущую. При пустой единице
    // используется unitcode.
    // --------------------------------------------------------------------
    void AddSubpos (LibElement &paramListDataToRead,
                    const GS::Array<GS::UniString> &partstring,
                    GS::UniString &unitcode,
                    double &qty,
                    const short &version) {
        GS::UniString subpos;
        GS::UniString pos;   // Позиция
        GS::UniString obozn; // ГОСТ
        GS::UniString naen;  // Наименование
        double _qty = 0;     // Количество
        double ves = 0;      // Масса ед.
        GS::UniString unit;  // Ед. измерения
        if (version == 3) {
            if (partstring.GetSize () < 10) {
                return;
            }
            subpos = partstring[0];
            pos = partstring[2];
            obozn = partstring[6];
            naen = partstring[7];
            if (!UniStringToDouble (partstring[9], _qty))
                _qty = 1;
            if (!UniStringToDouble (partstring[9], ves))
                ves = 0;
        }
        if (version == 4) {
            return;
        }
        GS::UniString subposkey = GetSubposKey (subpos);
        if (!paramListDataToRead.subpos.ContainsKey (subposkey)) {
            paramListDataToRead.subpos.Add (subposkey, {});
        }
        Subpos &s = paramListDataToRead.subpos.Get (subposkey);
        s.key = subposkey;
        if (!pos.IsEmpty ())
            s.pos = pos;
        if (!obozn.IsEmpty ())
            s.obozn = obozn;
        if (!naen.IsEmpty ())
            s.naen = naen;
        s.qty += _qty;
        if (!is_equal (ves, 0))
            s.ves = ves;
        if (s.unit.IsEmpty ())
            s.unit = unitcode;
    }

    // --------------------------------------------------------------------
    // AddProkat
    // Добавляет прокат из разобранной строки описания в элемент списка.
    // Требует версию 3 и не менее 15 полей. Поля: 0 — подпозиция, 2 —
    // позиция, 6 — тип конструкции, 7 — обозначение материала, 8 — материал,
    // 9 — количество, 10 — обозначение, 11 — тип профиля, 12 — длина (мм,
    // переводится в метры), 14 — масса тонны (при неудаче берётся qty).
    // Единица измерения — unitcode. Наименование формируется из типа
    // профиля и 'L = п.м.' либо 'L = <длина> мм'. Запись добавляется в
    // подпозицию по ключу subpos: при повторе ключа суммируются количество
    // и, для погонного метра, длина.
    // --------------------------------------------------------------------
    void AddProkat (LibElement &paramListDataToRead,
                    const GS::Array<GS::UniString> &partstring,
                    GS::UniString &unitcode,
                    double &qty,
                    const short &version) {
        Prokat p = {};
        GS::UniString subpos;
        if (version == 3) {
            if (partstring.GetSize () < 15) {
                return;
            }
            subpos = partstring[0];
            p.pos = partstring[2];
            p.tip_konstr = partstring[6];
            p.obozn_mater = partstring[7];
            p.mater = partstring[8];
            p.obozn = partstring[10];
            p.tip_profile = partstring[11];
            if (!UniStringToDouble (partstring[9], p.qty))
                p.qty = 1;
            if (!UniStringToDouble (partstring[12], p.dlin))
                p.dlin = 0;
            p.dlin = p.dlin / 1000.0; // Перевод в метры
            if (!UniStringToDouble (partstring[14], p.ves_t))
                p.ves_t = qty;
            p.ves = qty;
        }
        if (version == 4) {
            return;
        }
        p.unit = unitcode;
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
        Subpos &s = paramListDataToRead.subpos.Get (subposkey);
        p.key = GetKey (p);
        if (s.prokat.ContainsKey (p.key)) {
            Prokat &m = s.prokat.Get (p.key);
            m.qty += p.qty;
            if (m.isPm) {
                m.dlin += p.dlin;
            }
        } else {
            s.prokat.Add (p.key, p);
        }
    }

    // --------------------------------------------------------------------
    // GetAllKeys
    // Собирает все пары (ключ подпозиции, ключ позиции) элемента: сначала
    // перебираются арматура, материалы и прокат каждой подпозиции. Пустой
    // элемент или элемент без подпозиций даёт пустой массив.
    // --------------------------------------------------------------------
    GS::Array<GS::Pair<GS::UniString, GS::UniString>> GetAllKeys (const LibElement &el) {
#if defined(TESTING)
        DBprnt ("        Set keys for List Data");
#endif
        GS::Array<GS::Pair<GS::UniString, GS::UniString>> keys = {};
        if (el.subpos.IsEmpty ())
            return keys;
        for (const auto &sub : el.subpos) {
#ifdef ServerMainVers_2800
            const Subpos &subpos = sub.value;
            const GS::UniString subposkey = sub.key;
#else
            const Subpos &subpos = *sub.value;
            const GS::UniString subposkey = *sub.key;
#endif
            if (!subpos.arm.IsEmpty ()) {
                for (const auto &arm : subpos.arm) {
#ifdef ServerMainVers_2800
                    const GS::UniString k = arm.key;
#else
                    const GS::UniString k = *arm.key;
#endif
                    keys.Push (GS::Pair<GS::UniString, GS::UniString> (subposkey, k));
                }
            }
            if (!subpos.mat.IsEmpty ()) {
                for (const auto &mat : subpos.mat) {
#ifdef ServerMainVers_2800
                    const GS::UniString k = mat.key;
#else
                    const GS::UniString k = *mat.key;
#endif
                    keys.Push (GS::Pair<GS::UniString, GS::UniString> (subposkey, k));
                }
            }
            if (!subpos.prokat.IsEmpty ()) {
                for (const auto &prokat : subpos.prokat) {
#ifdef ServerMainVers_2800
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

    // --------------------------------------------------------------------
    // Add
    // Разбирает строку описания позиции name и добавляет её в элемент списка.
    // Версия определяется по маркеру 'v3%%' или 'v4%%' в имени, при
    // отсутствии маркера функция завершается. Строка делится по ';', второе
    // поле (тип элемента) должно быть двухзначным. Далее по коду типа
    // вызывается AddArm ('10'), AddProkat ('20'), AddMat ('30' и '40')
    // или AddSubpos ('45'). Параметры unitcode и qty передаются обработчику.
    // --------------------------------------------------------------------
    void Add (LibElement &paramListDataToRead, GS::UniString &name, GS::UniString &unitcode, double &qty) {
        short version = 0;
        if (name.Contains ("v3%%"))
            version = 3;
        if (name.Contains ("v4%%"))
            version = 4;
        if (version == 0)
            return;
        GS::Array<GS::UniString> partstring = {};
        UInt32 n = StringSplt (name, SEMICOLON, partstring, false);
        if (n < 2)
            return; // нет поля с обозначением
        GS::UniString tip_el = partstring[1];
        if (tip_el.IsEmpty () || tip_el.GetLength () != 2)
            return;
        if (tip_el == "10")
            AddArm (paramListDataToRead, partstring, unitcode, qty, version);
        if (tip_el == "20")
            AddProkat (paramListDataToRead, partstring, unitcode, qty, version);
        if (tip_el == "30")
            AddMat (paramListDataToRead, partstring, unitcode, qty, version);
        if (tip_el == "40")
            AddMat (paramListDataToRead, partstring, unitcode, qty, version);
        if (tip_el == "45")
            AddSubpos (paramListDataToRead, partstring, unitcode, qty, version);
        return;
    }

    // --------------------------------------------------------------------
    // AddLibdataToParamValueDict
    // Подставляет значения из собранных данных списка в словарь параметров
    // текстовой строки. По elemguid берётся запись LibElement, по номеру
    // n_layer — пара ключей (подпозиция, позиция). Префикс ключа позиции
    // определяет тип данных: '10@' — арматура, '20@' — прокат, '30@' — материал.
    // Значения записываются в params с префиксом LISTDATANAMEPREFIX в ветви
    // 'arm.'/'prokat.'/'mat.' и общую ветвь 'elem.'. Для погонного метра
    // количество и масса берутся из длины и массы тонны. Если rawname
    // содержит '{@listdata:subpos.', записываются параметры подпозиции.
    // Возвращает true, если данные добавлены, иначе false; при превышении
    // числа значений max_group_lib выводится msg_rep.
    // --------------------------------------------------------------------
    bool AddLibdataToParamValueDict (const API_Guid &elemguid,
                                     const GS::Int32 &n_layer,
                                     const LibElements &paramListDataToRead,
                                     const GS::UniString &rawname,
                                     ParamDictValue &params) {
        if (!paramListDataToRead.ContainsKey (elemguid))
            return false;
        const LibElement &p = paramListDataToRead.Get (elemguid);
        if (p.keys.IsEmpty ())
            return false;
        GS::Int32 max_layers = p.keys.GetSize ();
        if (n_layer >= max_layers) {
            return false;
        }
        if (max_layers >= max_group_lib) {
            msg_rep ("Spec err",
                     GS::UniString::Printf ("Max libdata over critical - %d", max_layers),
                     APIERR_GENERAL,
                     elemguid);
        }
        const GS::Pair<GS::UniString, GS::UniString> &keys = p.keys[n_layer];
        if (!p.subpos.ContainsKey (keys.first)) {
            return false;
        }
        const Subpos &s = p.subpos.Get (keys.first);
        if (keys.second.BeginsWith ("10@") &&
            (rawname.Contains ("{@listdata:arm.") || rawname.Contains ("{@listdata:elem."))) {
            if (!s.arm.ContainsKey (keys.second)) {
                return false;
            }
            const Arm &arm = s.arm.Get (keys.second);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.pos", arm.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.unit", arm.unit, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.klass", arm.klass, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.ves_t", arm.ves_t, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.dlin", arm.dlin, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.diam", arm.diam, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "arm.naen", arm.naen, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.pos", arm.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.unit", arm.unit, true);
            if (arm.isPm) {
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "arm.qty", arm.dlin, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "arm.ves", arm.ves_t, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.qty", arm.dlin, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.ves", arm.ves_t, true);
            } else {
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "arm.qty", arm.qty, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "arm.ves", arm.ves, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.qty", arm.qty, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.ves", arm.ves, true);
            }
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.type", "arm", true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.naen", arm.naen, true);
            return true;
        }
        if (keys.second.BeginsWith ("20@") &&
            (rawname.Contains ("{@listdata:prokat.") || rawname.Contains ("{@listdata:elem."))) {
            if (!s.prokat.ContainsKey (keys.second)) {
                return false;
            }
            const Prokat &prokat = s.prokat.Get (keys.second);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.pos", prokat.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.tip_konstr", prokat.tip_konstr, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.obozn_mater", prokat.obozn_mater, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.mater", prokat.mater, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.obozn", prokat.obozn, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.tip_profile", prokat.tip_profile, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.unit", prokat.unit, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.naen", prokat.naen, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.ves_t", prokat.ves_t, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "prokat.dlin", prokat.dlin, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.type", "prokat", true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.pos", prokat.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.obozn", prokat.obozn, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.unit", prokat.unit, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.naen", prokat.naen, true);
            if (prokat.isPm) {
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "prokat.ves", prokat.ves_t, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.ves", prokat.ves_t, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "prokat.qty", prokat.dlin, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.qty", prokat.dlin, true);
            } else {
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "prokat.ves", prokat.ves, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.ves", prokat.ves, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "prokat.qty", prokat.qty, true);
                ParamHelpers::AddDoubleValueToParamDictValue (
                    params, elemguid, LISTDATANAMEPREFIX, "elem.qty", prokat.qty, true);
            }
            return true;
        }
        if (keys.second.BeginsWith ("30@") &&
            (rawname.Contains ("{@listdata:mat.") || rawname.Contains ("{@listdata:elem."))) {
            if (!s.mat.ContainsKey (keys.second)) {
                return false;
            }
            const Mat &mat = s.mat.Get (keys.second);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.pos", mat.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.tip_konstr", mat.tip_konstr, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.obozn", mat.obozn, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.naen", mat.naen, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.unit", mat.unit, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.qty", mat.qty, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "mat.ves", mat.ves, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.pos", mat.pos, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.obozn", mat.obozn, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.naen", mat.naen, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.unit", mat.unit, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.qty", mat.qty, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.ves", mat.ves, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.type", "mat.object", true);
            return true;
        }
        if (rawname.Contains ("{@listdata:subpos.")) {
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.obozn", s.obozn, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.naen", s.naen, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.unit", s.unit, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.ves", s.ves, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.pos", s.pos, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "subpos.qty", s.qty, true);
            ParamHelpers::AddStringValueToParamDictValue (
                params, elemguid, LISTDATANAMEPREFIX, "elem.type", "subpos", true);
            return true;
        }
        return false;
    }

} // namespace ListData
