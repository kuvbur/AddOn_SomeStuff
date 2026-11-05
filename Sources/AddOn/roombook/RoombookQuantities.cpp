// SDK подключается раньше стандартных заголовков.
// clang-format off
#include "ACAPinc.h"
// clang-format on

#include <algorithm>
#include <cmath>
#include <map>
#include <string>

#include "api_headers/APIEnvir.h"

#include "CommonFunction.hpp"
#include "Helpers.hpp"
#include "roombook/Roombook.hpp"
#include "third_party/alphanum.h"

#ifdef ServerMainVers_2400
namespace Roombook {

    void OtdData_CalcForRoom (const ColumnFormatDict &columnFormat,
                              const OtdRoom &otd,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              OtdMaterialAreaDictByOtdType &dct_bytype);
    void OtdData_WriteToRoom (const ColumnFormatDict &columnFormat,
                              const API_Guid &zone_guid,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              const OtdMaterialAreaDictByType &dct,
                              const GS::HashTable<TypeOtd, GS::UniString> &paramnamebytype);

    static void OtdData_GetColumnFormat (GS::UniString descripton,
                                         const GS::UniString &rawname,
                                         ColumnFormatDict &columnFormat);
    static void OtdData_AddValueToDict (OtdMaterialAreaDictByType &dct,
                                        const TypeOtd &type,
                                        const GS::UniString &mat,
                                        double area);

    // Длины подготовленных плинтусов; результат не зависит от записи native-элементов.
    // void OtdRoom_CalculateSkirtingLengths (OtdRoom &otd) {
    //    otd.length_beam_floor = 0;
    //    otd.length_beam_ceil = 0;
    //    if (!otd.isValid)
    //        return;
    //    for (const auto *beams : {&otd.otdbeam_floor, &otd.otdbeam_ceil}) {
    //        for (const auto &beam : *beams) {
    //            if (beam.draw.type != Skirting_Down && beam.draw.type != Skirting_Up)
    //                continue;
    //            const double length = std::hypot (std::hypot (beam.geom.seg.endC.x - beam.geom.seg.begC.x,
    //                                                          beam.geom.seg.endC.y - beam.geom.seg.begC.y),
    //                                              beam.geom.seg.endC.z - beam.geom.seg.begC.z);
    //            (beam.draw.type == Skirting_Down ? otd.length_beam_floor : otd.length_beam_ceil) += length;
    //        }
    //    }
    //}

    // Дополняет сводку одной комнаты и подготавливает её свойства; не выбирает зоны на создание.
    // Невалидная комната, отсутствующие параметры или обязательные имена результата не дают вклада.
    void OtdData_AddRoomToMaterialSummary (const OtdRoom &otd,
                                           ParamDictElement &paramToRead,
                                           MaterialSummary &summary) {
        auto &paramToWrite = summary.paramToWrite;
        auto &columnFormat = summary.columnFormat;
        auto &dct_bytype = summary.dct_bytype;
        auto &zones_bytype = summary.zones_bytype;
        if (!otd.isValid)
            return;
        const ParamDictValue *roomparams = paramToRead.GetPtr (otd.zone_guid);
        if (roomparams == nullptr)
            return;
        GS::Array<GS::UniString> rawnames;
        GS::UniString msg;
        if (!otd.om_up.rawname.IsEmpty ())
            rawnames.Push (otd.om_up.rawname);
        if (!otd.om_main.rawname.IsEmpty ())
            rawnames.Push (otd.om_main.rawname);
        if (!otd.om_down.rawname.IsEmpty ())
            rawnames.Push (otd.om_down.rawname);
        if (!otd.om_column.rawname.IsEmpty ())
            rawnames.Push (otd.om_column.rawname);
        if (!otd.om_reveals.rawname.IsEmpty ())
            rawnames.Push (otd.om_reveals.rawname);
        if (!otd.om_floor.rawname.IsEmpty ())
            rawnames.Push (otd.om_floor.rawname);
        if (!otd.om_ceil.rawname.IsEmpty ())
            rawnames.Push (otd.om_ceil.rawname);
        if (!otd.om_up.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_up.rawname_bytype);
        if (!otd.om_main.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_main.rawname_bytype);
        if (!otd.om_down.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_down.rawname_bytype);
        if (!otd.om_column.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_column.rawname_bytype);
        if (!otd.om_reveals.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_reveals.rawname_bytype);
        if (!otd.om_floor.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_floor.rawname_bytype);
        if (!otd.om_ceil.rawname_bytype.IsEmpty ())
            rawnames.Push (otd.om_ceil.rawname_bytype);
        if (otd.om_main.rawname.IsEmpty () && otd.om_main.rawname_bytype.IsEmpty ())
            msg += "'some_stuff_fin_main_result' ";
        if (otd.om_down.rawname.IsEmpty () && otd.om_down.rawname_bytype.IsEmpty ())
            msg += "'some_stuff_fin_down_result' ";
        if (otd.om_column.rawname.IsEmpty () && otd.om_column.rawname_bytype.IsEmpty ())
            msg += "'some_stuff_fin_column_result' ";
        if (otd.om_ceil.rawname.IsEmpty () && otd.om_ceil.rawname_bytype.IsEmpty ())
            msg += "'some_stuff_fin_ceil_result' ";
        if (!msg.IsEmpty ()) {
            msg_rep ("RoomBook",
                     "Properties for recording finish layers to the Zone "
                     "were not "
                     "found.\nRecording will not be performed. Missing " +
                         msg,
                     NoError,
                     APINULLGuid);
            return;
        }
        // Получение форматов столбцов
        for (const GS::UniString &rawname : rawnames) {
            if (rawname.IsEmpty ())
                continue;
            if (columnFormat.ContainsKey (rawname))
                continue;
            if (!roomparams->ContainsKey (rawname))
                continue;
            OtdData_GetColumnFormat (roomparams->Get (rawname).definition.description, rawname, columnFormat);
        }
        // Расчёт площадей
        OtdData_CalcForRoom (columnFormat, otd, paramToWrite, paramToRead, dct_bytype);
        zones_bytype.Push (otd.zone_guid);
    }

    // Подготавливает значения свойств зон по общей сводке материалов для их tip_otd, используя имена rawname_bytype.
    void OtdData_WriteRoomMaterialData (OtdRooms &roomsinfo, ParamDictElement &paramToRead, MaterialSummary &summary) {
        // Общая сводка по типу помещения добавляется в summary.paramToWrite, без записи в модель.
        for (const API_Guid &subguid : summary.zones_bytype) {
            OtdRoom &otd = roomsinfo.Get (subguid);
            if (!summary.dct_bytype.ContainsKey (otd.tip_otd))
                continue;
            OtdMaterialAreaDictByType &dct = summary.dct_bytype.Get (otd.tip_otd);
            GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
            paramnamebytype.Add (Wall_Up,
                                 otd.om_up.rawname_bytype); // Отделка стен выше потолка
            paramnamebytype.Add (Wall_Main,
                                 otd.om_main.rawname_bytype); // Отделка стен основная
            paramnamebytype.Add (Wall_Down,
                                 otd.om_down.rawname_bytype); // Отделка низа стен
            paramnamebytype.Add (Column,
                                 otd.om_column.rawname_bytype); // Отделка колонн
            paramnamebytype.Add (Reveal_Main,
                                 otd.om_reveals.rawname_bytype);      // Отделка откосов
            paramnamebytype.Add (Floor, otd.om_floor.rawname_bytype); // Отделка пола
            paramnamebytype.Add (Ceil,
                                 otd.om_ceil.rawname_bytype); // Отделка потолка
            OtdData_WriteToRoom (
                summary.columnFormat, otd.zone_guid, summary.paramToWrite, paramToRead, dct, paramnamebytype);
        }
    }

    // Формат столбцов: ширина текста и пробелов в миллиметрах.
    // Настройки для форматирования текста в таблицу
    static void OtdData_GetColumnFormat (GS::UniString descripton,
                                         const GS::UniString &rawname,
                                         ColumnFormatDict &columnFormat) {
        GS::UniString fontname = "GOST 2.304 type A";
        ColumnFormat c;
        if (descripton.Contains (BRACESTART) && descripton.Contains (BRACEEND) && descripton.Contains (SEMICOLON)) {
            descripton = descripton.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
            GS::Array<GS::UniString> partstring;
            UInt32 n = StringSplt (descripton, SEMICOLON, partstring, false);
            if (n > 0) {
                fontname = "Arial";
                if (!UniStringToDouble (partstring[0], c.width_mat))
                    c.width_mat = 50;
            }
            if (n > 1) {
                if (!UniStringToDouble (partstring[1], c.width_area))
                    c.width_area = 20;
            }
            if (n > 2) {
                if (!UniStringToDouble (partstring[2], c.fontsize))
                    c.fontsize = 2.5;
            }
            if (n > 3) {
                fontname = partstring[3];
            }
            if (n > 4) {
                GS::UniString space = UniStringToLower (partstring[4]);
                if (space.Contains ("nbs"))
                    space = reinterpret_cast<const char *> (u8"\u2007");
                if (space.Contains ("ns"))
                    space = reinterpret_cast<const char *> (u8"\u202F");
                if (space.Contains ("s"))
                    space = SPACESTRING;
                c.no_breake_space = space;
            }
            if (n > 5) {
                GS::UniString space = UniStringToLower (partstring[5]);
                if (space.Contains ("nbs"))
                    space = reinterpret_cast<const char *> (u8"\u2007");
                if (space.Contains ("ns"))
                    space = reinterpret_cast<const char *> (u8"\u202F");
                if (space.Contains ("s"))
                    space = SPACESTRING;
                c.narow_space = space;
            }
        }
        c.font = GetFontIndex (fontname);
        if (c.font == 1) {
            fontname = "Arial";
            c.font = GetFontIndex (fontname);
        }
        Int32 addspace = 0;
        GS::UniString delim = "-";
        c.width_no_breake_space = GetTextWidth (c.font, c.fontsize, c.no_breake_space);
        if (c.width_no_breake_space < 0.001 || is_equal (c.width_no_breake_space, 0)) {
            c.no_breake_space = SPACESTRING;
            c.width_no_breake_space = GetTextWidth (c.font, c.fontsize, c.no_breake_space);
        }
        if (c.width_no_breake_space < 0.001) {
            c.width_no_breake_space = 0.5;
        }
        c.width_narow_space = GetTextWidth (c.font, c.fontsize, c.narow_space);
        if (c.width_narow_space < 0.001 || is_equal (c.width_narow_space, 0)) {
            c.narow_space = SPACESTRING;
            c.width_narow_space = GetTextWidth (c.font, c.fontsize, c.narow_space);
        }
        if (c.width_narow_space < 0.001) {
            c.width_narow_space = 0.5;
        }
        addspace = DoubleToInt32 (c.width_area / c.width_narow_space, "Roombook", "число пробелов по ширине");
        if (fabs (addspace * c.width_narow_space - c.width_area) > c.width_narow_space)
            addspace -= 1;
        c.space_line = EMPTYSTRING;
        for (Int32 j = 0; j < addspace; j++) {
            c.space_line.Append (c.narow_space);
        }
        c.width_area = GetTextWidth (c.font, c.fontsize, c.space_line);
        double width_delim = GetTextWidth (c.font, c.fontsize, delim);
        addspace =
            DoubleToInt32 ((c.width_mat + c.width_area) / width_delim, "Roombook", "число разделителей по ширине");
        if (fabs (addspace * width_delim - c.width_mat - c.width_area) > c.width_narow_space)
            addspace -= 1;
        c.delim_line = EMPTYSTRING;
        for (Int32 j = 0; j < addspace; j++) {
            c.delim_line.Append (delim);
        }
        c.width_mat = GetTextWidth (c.font, c.fontsize, c.delim_line) - c.width_area;
        c.delim_line.Append (SPACESTRING);
        c.space_line.Append (SPACESTRING);
        columnFormat.Add (rawname, c);
    }

    // Площади готовой отделки и распределение по материалам. Геометрия здесь не изменяется.
    // Накапливает площади материалов валидных стен и перекрытий комнаты в словаре по типам отделки.
    // Имена слоёв разделяет по точке с запятой; переданный словарь не очищает.
    static void OtdData_CollectMaterialAreasForRoom (const OtdRoom &otd, OtdMaterialAreaDictByType &dct) {
        // Разбивка по отделочным слоям, вычисление площадей и добавление их в
        // словарь по типам отделки
        for (const OtdWall &otdw : otd.otdwall) {
            if (!otdw.isValid)
                continue;
            const TypeOtd &t = otdw.draw.type;
            const double area = OtdWall_GetArea (otdw);
            for (UInt32 j = 0; j < otdw.base_composite.GetSize (); j++) {
                GS::UniString mat = otdw.base_composite[j].val;
                if (mat.Contains (SEMICOLON)) {
                    GS::Array<GS::UniString> partstring;
                    UInt32 n = StringSpltUnic (mat, SEMICOLON, partstring);
                    if (n < 1) {
                        OtdData_AddValueToDict (dct, t, mat, area);
                    } else {
                        for (const GS::UniString &mat_ : partstring) {
                            OtdData_AddValueToDict (dct, t, mat_, area);
                        }
                    }
                } else {
                    OtdData_AddValueToDict (dct, t, mat, area);
                }
            }
        }

        for (const OtdSlab &otdslab : otd.otdslab) {
            if (!otdslab.isValid)
                continue;
            const TypeOtd &t = otdslab.draw.type;
            const double area =
                otdslab.poly.CalcArea () * (otdslab.draw.base_type == API_MeshID
                                                ? std::hypot (1.0, std::hypot (otdslab.slope_x, otdslab.slope_y))
                                                : 1.0);
            for (UInt32 j = 0; j < otdslab.base_composite.GetSize (); j++) {
                GS::UniString mat = otdslab.base_composite[j].val;
                if (mat.Contains (SEMICOLON)) {
                    GS::Array<GS::UniString> partstring;
                    UInt32 n = StringSpltUnic (mat, SEMICOLON, partstring);
                    if (n < 1) {
                        OtdData_AddValueToDict (dct, t, mat, area);
                    } else {
                        for (const GS::UniString &mat_ : partstring) {
                            OtdData_AddValueToDict (dct, t, mat_, area);
                        }
                    }
                } else {
                    OtdData_AddValueToDict (dct, t, mat, area);
                }
            }
        }
    }

    // Собирает площади материалов комнаты, подготавливает её свойства результата и добавляет площади в сводку по
    // tip_otd.
    void OtdData_CalcForRoom (const ColumnFormatDict &columnFormat,
                              const OtdRoom &otd,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              OtdMaterialAreaDictByOtdType &dct_bytype) {
        OtdMaterialAreaDictByType dct; // Основной словарь
        OtdData_CollectMaterialAreasForRoom (otd, dct);
        // Записываем результаты в результаты для каждого помещения
        GS::HashTable<TypeOtd, GS::UniString> paramnamebytype;
        paramnamebytype.Add (Wall_Up,
                             otd.om_up.rawname); // Отделка стен выше потолка
        paramnamebytype.Add (Wall_Main,
                             otd.om_main.rawname);                 // Отделка стен основная
        paramnamebytype.Add (Wall_Down, otd.om_down.rawname);      // Отделка низа стен
        paramnamebytype.Add (Column, otd.om_column.rawname);       // Отделка колонн
        paramnamebytype.Add (Reveal_Main, otd.om_reveals.rawname); // Отделка откосов
        paramnamebytype.Add (Floor, otd.om_floor.rawname);         // Отделка пола
        paramnamebytype.Add (Ceil, otd.om_ceil.rawname);           // Отделка потолка
        OtdData_WriteToRoom (columnFormat, otd.zone_guid, paramToWrite, paramToRead, dct, paramnamebytype);
        // Формируем словарь для разбивки по типам

        if (!dct_bytype.ContainsKey (otd.tip_otd)) {
            dct_bytype.Add (otd.tip_otd, dct);
            return;
        }
        OtdMaterialAreaDictByType &dct_to = dct_bytype.Get (otd.tip_otd);
        for (auto &cItt : dct) {
    #ifdef ServerMainVers_2800
            const OtdMaterialAreaDict dct_t = cItt.value;
            const TypeOtd typeotd = cItt.key;
    #else
            const OtdMaterialAreaDict dct_t = *cItt.value;
            const TypeOtd typeotd = *cItt.key;
    #endif
            for (auto &cIt : dct_t) {
    #ifdef ServerMainVers_2800
                const double area = cIt.value;
                const GS::UniString mat = cIt.key;
    #else
                const double area = *cIt.value;
                const GS::UniString mat = *cIt.key;
    #endif
                OtdData_AddValueToDict (dct_to, typeotd, mat, area);
            }
        }
    }

    // Для одного типа копирует его площади материалов; для нескольких суммирует найденные площади в переданный словарь.
    static void OtdData_CollectAreasForFinishTypes (const GS::Array<TypeOtd> &typeotd,
                                                    const OtdMaterialAreaDictByType &dct,
                                                    OtdMaterialAreaDict &dcta) {
        if (typeotd.GetSize () == 1) {
            if (dct.ContainsKey (typeotd[0]))
                dcta = dct.Get (typeotd[0]);
        } else {
            // В одно свойство записывается несколько типов. Суммируем
            // материалы.
            for (auto &t : typeotd) {
                if (!dct.ContainsKey (t))
                    continue;
                const OtdMaterialAreaDict &d = dct.Get (t);
                for (auto &m : d) {
    #ifdef ServerMainVers_2800
                    const GS::UniString mat = m.key;
                    const double area = m.value;
    #else
                    const GS::UniString mat = *m.key;
                    const double area = *m.value;
    #endif
                    if (dcta.ContainsKey (mat)) {
                        double area_sum = dcta.Get (mat) + area;
                        dcta.Set (mat, area_sum);
                    } else {
                        dcta.Add (mat, area);
                    }
                }
            }
        }
    }

    // Текст результата: объединение типов, порядок материалов, выравнивание и изменённые свойства.
    // Объединяет типы отделки с одинаковым именем свойства, сортирует материалы и форматирует площади в текстовую
    // таблицу. Добавляет в paramToWrite только изменённые значения; пустой результат заменяет на «-».
    void OtdData_WriteToRoom (const ColumnFormatDict &columnFormat,
                              const API_Guid &zone_guid,
                              ParamDictElement &paramToWrite,
                              const ParamDictElement &paramToRead,
                              const OtdMaterialAreaDictByType &dct,
                              const GS::HashTable<TypeOtd, GS::UniString> &paramnamebytype) {
        // Мы получили словарь по типам отделки
        // Проблема в том, что для разных типов имя свойство может быть одно
        // Это означает, что нужно просуммировать отделку для каждого имени свойства
        GS::HashTable<GS::UniString, GS::Array<TypeOtd>> exsists_rawname; // словарь существующих свойств для типов
                                                                          // отделки
        for (auto &cItt : paramnamebytype) {
    #ifdef ServerMainVers_2800
            const GS::UniString rawname = cItt.value;
            const TypeOtd typeotd = cItt.key;
    #else
            const GS::UniString rawname = *cItt.value;
            const TypeOtd typeotd = *cItt.key;
    #endif
            if (rawname.IsEmpty ())
                continue;
            if (!paramToRead.Get (zone_guid).ContainsKey (rawname))
                continue;
            if (!exsists_rawname.ContainsKey (rawname)) {
                GS::Array<TypeOtd> t;
                exsists_rawname.Add (rawname, t);
            }
            exsists_rawname.Get (rawname).Push (typeotd);
        }

        for (auto &cItt : exsists_rawname) {
    #ifdef ServerMainVers_2800
            GS::UniString rawname = cItt.key;
            GS::Array<TypeOtd> typeotd = cItt.value;
    #else
            GS::UniString rawname = *cItt.key;
            GS::Array<TypeOtd> typeotd = *cItt.value;
    #endif
            // Проверяем - есть ли считанное свойство в зоне для записи
            ParamValue paramtow = paramToRead.Get (zone_guid).Get (rawname);
            GS::UniString old_val = paramtow.val.uniStringValue;
            GS::UniString new_val;
            // Если такой тип отделки есть в словаре - записываем послойно материалы
            // и площади
            OtdMaterialAreaDict dcta;
            OtdData_CollectAreasForFinishTypes (typeotd, dct, dcta);
            if (!dcta.IsEmpty ()) {
                ColumnFormat c;
                if (columnFormat.ContainsKey (rawname))
                    c = columnFormat.Get (rawname);
                // Сортировка по алфавиту
                std::map<std::string, GS::UniString, doj::alphanum_less<std::string>> abc_material;
                for (auto &cIt : dcta) {
    #ifdef ServerMainVers_2800
                    GS::UniString mat = cIt.key;
    #else
                    GS::UniString mat = *cIt.key;
    #endif
                    GSCharCode chcode = GetCharCode (mat);
                    std::string s = mat.ToCStr (0, MaxUSize, chcode).Get ();
                    abc_material[s] = mat;
                }
                for (std::map<std::string, GS::UniString, doj::alphanum_less<std::string>>::iterator k =
                         abc_material.begin ();
                     k != abc_material.end ();
                     ++k) {
                    GS::UniString mat = k->second;
                    const double *dcta_area = dcta.GetPtr (mat);
                    if (dcta_area == nullptr)
                        continue;
                    double area = *dcta_area;
                    mat.Trim ();
                    mat.ReplaceAll ("  ", SPACESTRING);
                    mat.ReplaceAll ("0&#& ", EMPTYSTRING);
                    mat.ReplaceAll (SPACESTRING, c.no_breake_space);
                    GS::UniString area_sring = GS::UniString::Printf ("%.2f", area);
                    double w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                    if (w_area < c.width_area) {
                        Int32 addspace =
                            DoubleToInt32 ((c.width_area - w_area) / c.width_narow_space, "Roombook", "число пробелов");
                        if (fabs (addspace * c.width_narow_space - c.width_area + w_area) > 0.01)
                            addspace -= 1;
                        if (addspace > 1) {
                            for (Int32 j = 0; j < addspace; j++) {
                                area_sring = c.narow_space + area_sring;
                            }
                        }
                        w_area = GetTextWidth (c.font, c.fontsize, area_sring);
                    }
                    if (w_area - c.width_area > 0.1)
                        area_sring = c.narow_space + area_sring;
                    area_sring.Append (SPACESTRING);
                    if (!new_val.IsEmpty ())
                        new_val.Append (c.delim_line);
                    GS::Array<GS::UniString> lines_mat =
                        DelimTextLine (c.font, c.fontsize, c.width_mat, mat, c.no_breake_space, c.narow_space);
                    for (UInt32 j = 0; j < lines_mat.GetSize (); j++) {
                        new_val.Append (lines_mat[j]);
                        if (j == lines_mat.GetSize () - 1) {
                            new_val.Append (area_sring);
                        } else {
                            new_val.Append (c.space_line);
                        }
                    }
                }
            }
            if (new_val.IsEmpty ())
                new_val = "-";
            if (!old_val.IsEqual (new_val)) {
                paramtow.val.uniStringValue = new_val;
                paramtow.isValid = true;
                ParamHelpers::AddParamValue2ParamDictElement (zone_guid, paramtow, paramToWrite);
            }
        }
    }

    // Добавляет площадь к материалу заданного типа отделки, создавая отсутствующие словари.
    // Пропускает площади меньше 0.000001 и имена, содержащие IGNORENAME.
    static void OtdData_AddValueToDict (OtdMaterialAreaDictByType &dct,
                                        const TypeOtd &type,
                                        const GS::UniString &mat,
                                        double area) {
        if (area < 0.000001)
            return;
        if (mat.Contains (IGNORENAME))
            return;
        OtdMaterialAreaDict *innerDictPtr = dct.GetPtr (type);

        if (innerDictPtr == nullptr) {
            // Если такого типа еще нет, создаем и добавляем
            OtdMaterialAreaDict newDict;
            newDict.Add (mat, area);
            dct.Add (type, std::move (newDict));
            return;
        }

        // ищем площадь конкретного материала
        if (double *existingAreaPtr = innerDictPtr->GetPtr (mat)) {
            *existingAreaPtr += area;
        } else {
            innerDictPtr->Add (mat, area);
        }
    }

    // -----------------------------------------------------------------------------
    // Получение площади отделочной стены с учётом отверстий, м²
    // -----------------------------------------------------------------------------
    double OtdWall_GetArea (const OtdWall &otdw) {
        double dx = -otdw.geom.seg.endC.x + otdw.geom.seg.begC.x;
        double dy = -otdw.geom.seg.endC.y + otdw.geom.seg.begC.y;
        double dr = sqrt (dx * dx + dy * dy);
        double area = dr * otdw.geom.height;
        if (otdw.surface_geometry) {
            for (const auto &opening : otdw.openings)
                area -= opening.width * opening.height;
            return std::max (0.0, area) * std::sqrt (1 + otdw.rise_x * otdw.rise_x + otdw.rise_y * otdw.rise_y);
        }
        // Если задана ширина - это откос
        if (!is_equal (otdw.geom.width, 0)) {
            if (!is_equal (otdw.geom.length, 0)) {
                area = otdw.geom.width * otdw.geom.length;
            } else {
                area = otdw.geom.width * otdw.geom.height;
            }
            // В откосах проёмов быть не может - выходим
            if (area < 0)
                area = 0;
            return area;
        }
        if (is_equal (dr, 0))
            return 0;
        if (is_equal (area, 0))
            return 0;
        if (!otdw.openings.IsEmpty ()) {
            double area_op = 0;
            for (const OtdOpening &op : otdw.openings) {
                area_op += op.height * op.width;
            }
            area -= area_op;
            if (area < 0)
                area = 0;
        }
        return area;
    }

} // namespace Roombook
#endif
