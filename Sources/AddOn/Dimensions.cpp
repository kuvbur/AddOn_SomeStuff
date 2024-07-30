//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Dimensions.hpp"

#define DIM_NOCHANGE 0
#define DIM_HIGHLIGHT_ON 1
#define DIM_HIGHLIGHT_OFF 2
#define DIM_CHANGE_ON 1
#define DIM_CHANGE_FORCE 2
#define DIM_CHANGE_OFF 3

// TODO Попробовать добавить все подходящие слои в правила на этапе чтения настроек
// -----------------------------------------------------------------------------
// Чтение настроек из информации о проекте
//	Имя свойства: "Addon_Dimenstions"
//	Формат записи: ПЕРО_РАЗМЕРА - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА", либо
//					"Слой" - КРАТНОСТЬ_ММ, ПЕРО_ТЕКСТА_ИЗМЕНЁННОЕ, ФЛАГ_ИЗМЕНЕНИЯ_СОДЕРЖИМОГО, "ФОРМУЛА"
// -----------------------------------------------------------------------------
GSErrCode DimReadPref (DimRules& dimrules)
{
    GS::Array<GS::ArrayFB<GS::UniString, 3> >	autotexts;
    API_AutotextType	type = APIAutoText_Custom;
    DBPrintf ("== SMSTF == DimReadPref start\n");
    GSErrCode	err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_AutoText_GetAutoTexts (&autotexts, type);
#else
    err = ACAPI_Goodies (APIAny_GetAutoTextsID, &autotexts, (void*) (GS::IntPtr) type);
#endif
    if (err != NoError) {
        msg_rep ("DimReadPref", "ACAPI_Goodies", err, APINULLGuid);
        return err;
    }
    for (UInt32 i = 0; i < autotexts.GetSize (); i++) {
        if (autotexts[i][0].Contains ("Addon_Dimenstions") || autotexts[i][0].Contains ("Addon_Dimensions")) {
            bool hasexpression = false; // Нужно ли нам читать список свойств
            if (autotexts[i][2].Contains (";")) {
                GS::Array<GS::UniString> partstring;
                StringSplt (autotexts[i][2], ";", partstring);
                for (UInt32 k = 0; k < partstring.GetSize (); k++) {
                    DimRule dimrule;
                    if (DimParsePref (partstring[k], dimrule, hasexpression)) {
                        GS::UniString kstr;
                        if (dimrule.layer.IsEmpty ()) {
                            kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
                        } else {
                            kstr = dimrule.layer;
                        }
                        dimrules.Add (kstr, dimrule);
                    }
                }
            } else {
                DimRule dimrule;
                if (DimParsePref (autotexts[i][2], dimrule, hasexpression)) {
                    GS::UniString kstr;
                    if (dimrule.layer.IsEmpty ()) {
                        kstr = GS::UniString::Printf ("%d", dimrule.pen_original);
                    } else {
                        kstr = dimrule.layer;
                    }
                    dimrules.Add (kstr, dimrule);
                }
            }
            DBPrintf ("== SMSTF == DimReadPref end!!\n");
            return err;
        }
    }
    DBPrintf ("== SMSTF == DimReadPref end\n");
    return err;
}

// -----------------------------------------------------------------------------
// Обработка одного правила
// -----------------------------------------------------------------------------
bool DimParsePref (GS::UniString& rawrule, DimRule& dimrule, bool& hasexpression)
{
    GS::Array<GS::UniString> partstring_1;
    if (StringSplt (rawrule, "-", partstring_1) == 2) {

        //Проверяем - что указано в правиле: слой или номер пера
        // Слой указываем в кавычках, в regexp формате
        if (partstring_1[0].Contains ('"')) {
            GS::UniString layer = partstring_1[0];
            layer.ReplaceAll ('"', ' ');
            layer.Trim ();
            dimrule.layer = layer;
        } else {
            dimrule.pen_original = std::atoi (partstring_1[0].ToCStr ());
        }
        GS::Array<GS::UniString> partstring_2;
        if (StringSplt (partstring_1[1], ",", partstring_2) > 1) {
            dimrule.round_value = std::atoi (partstring_2[0].ToCStr ());
            dimrule.pen_rounded = std::atoi (partstring_2[1].ToCStr ());
            if (partstring_2.GetSize () == 3) {
                if (partstring_2[2].Contains ("{") && partstring_2[2].Contains ("}")) {
                    ParamDictValue paramDict;
                    GS::UniString expression = partstring_2[2];
                    expression.ReplaceAll ("<MeasuredValue>", "{MeasuredValue}");
                    ParamHelpers::ParseParamName (expression, paramDict);
                    dimrule.paramDict = paramDict;
                    dimrule.expression = expression;
                    if (!hasexpression) hasexpression = !paramDict.IsEmpty ();
                } else {
                    dimrule.flag_change = (std::atoi (partstring_2[2].ToCStr ()) > 0);
                }
            }
            if (partstring_2.GetSize () == 4) {
                if (partstring_2[3].Contains ("{") && partstring_2[3].Contains ("}")) {
                    ParamDictValue paramDict;
                    GS::UniString expression = partstring_2[3];
                    expression.ReplaceAll ("<MeasuredValue>", "{MeasuredValue}");
                    ParamHelpers::ParseParamName (expression, paramDict);
                    dimrule.paramDict = paramDict;
                    dimrule.expression = expression;
                    if (!hasexpression) hasexpression = !paramDict.IsEmpty ();
                }
            }
            return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
GSErrCode DimAutoRound (const API_Guid& elemGuid, DimRules& dimrules, ParamDictValue& propertyParams)
{
    API_ElemTypeID elementType;
    if (GetTypeByGUID (elemGuid, elementType) != NoError) return NoError;
    if (elementType != API_DimensionID) return NoError;
    GSErrCode err = NoError;
    API_Element element;
    BNZeroMemory (&element, sizeof (API_Element));
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("DimAutoRound", "ACAPI_Element_Get", err, elemGuid);
        return err;
    }

    // Если нет элементов - выходим
    if (!element.header.hasMemo) return err;
    short pen_dimenstion = element.dimension.linPen;
    short pen_original = element.dimension.defNote.notePen;
    short pen_rounded = 0;
    bool flag_change_rule = false;
    bool find_rule = false;
    GS::UniString kstr = GS::UniString::Printf ("%d", pen_dimenstion);
    if (pen_dimenstion > 0 && dimrules.ContainsKey (kstr)) find_rule = true;
    if (!find_rule) {
        API_Attribute layer;
        BNZeroMemory (&layer, sizeof (API_Attribute));
        layer.header.typeID = API_LayerID;
        layer.header.index = element.header.layer;
        if (ACAPI_Attribute_Get (&layer) != NoError) return err;
        GS::UniString layert = GS::UniString::Printf ("%s", layer.header.name);
        for (GS::HashTable<GS::UniString, DimRule>::ConstPairIterator cIt = dimrules.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28)
            const GS::UniString& regexpstring = cIt->key;
#else
            const GS::UniString& regexpstring = *cIt->key;
#endif
            if (layert.Contains (regexpstring)) {
                kstr = regexpstring;
                find_rule = true;
                break;
            }
        }
    }

    // Нет подходящего привали - выходим
    if (!find_rule) return err;
    DimRule dimrule = dimrules[kstr];
    pen_rounded = dimrule.pen_rounded;
    flag_change_rule = dimrule.flag_change;
    short pen = pen_rounded;
    pen_original = pen_dimenstion; // Быстрофикс
    bool flag_write = false;
    API_ElementMemo memo;
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (element.header.guid, &memo);
    if (err != NoError) {
        msg_rep ("DimAutoRound", "ACAPI_Element_GetMemo", err, elemGuid);
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    API_Guid bef_elemGuid = (*memo.dimElems)[0].base.base.guid;
    for (Int32 k = 1; k < element.dimension.nDimElem; k++) {
        UInt32 flag_change = DIM_NOCHANGE;
        UInt32 flag_highlight = DIM_NOCHANGE;
        auto& dimElem = (*memo.dimElems)[k];
        API_ElemTypeID elementType;
#if defined AC_26 || defined AC_27 || defined AC_28
        elementType = dimElem.base.base.type.typeID;
#else
        elementType = dimElem.base.base.typeID;
#endif // AC_26

        // TODO Баг в архикаде - при обработке размеров, привязанных к колонне - они слетают.
        if (dimElem.dimVal == 0 && elementType == API_ColumnID) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return err;
        };
        if (dimElem.dimVal == 0) continue;
        GS::UniString content = GS::UniString::Printf ("%s", dimElem.note.content);
        API_Guid ref_elemGuid = dimElem.base.base.guid;
        if (ref_elemGuid != bef_elemGuid) ref_elemGuid = APINULLGuid;
        API_NoteContentType contentType = dimElem.note.contentType;
        if (DimParse (dimElem.dimVal, ref_elemGuid, contentType, content, flag_change, flag_highlight, dimrule, propertyParams)) {
            if (!flag_change_rule && flag_change != DIM_CHANGE_FORCE) flag_change = DIM_CHANGE_OFF;
            if (flag_change == DIM_CHANGE_ON || flag_change == DIM_CHANGE_FORCE) {
                flag_write = true;
                (*memo.dimElems)[k].note.contentType = API_NoteContent_Custom;
                if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                    delete (*memo.dimElems)[k].note.contentUStr;
                (*memo.dimElems)[k].note.contentUStr = new GS::UniString (content);
            }
            if (flag_change == DIM_CHANGE_OFF && dimElem.note.contentType != API_NoteContent_Measured) {
                flag_write = true;
                (*memo.dimElems)[k].note.contentType = API_NoteContent_Measured;
                if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                    delete (*memo.dimElems)[k].note.contentUStr;
                (*memo.dimElems)[k].note.contentUStr = new GS::UniString ("");
            }
            if (flag_highlight == DIM_HIGHLIGHT_ON) pen = pen_rounded;
            if (flag_highlight == DIM_HIGHLIGHT_OFF) pen = pen_original;
            if (flag_highlight != DIM_NOCHANGE && dimElem.note.notePen != pen) {
                flag_write = true;
                (*memo.dimElems)[k].note.notePen = pen;
            }
        }
        bef_elemGuid = dimElem.base.base.guid;
    }
    if (flag_write) {
        API_Element mask;
        ACAPI_ELEMENT_MASK_SETFULL (mask);
        API_Guid elemGuid_n = elemGuid;
        err = ACAPI_CallUndoableCommand ("Change dimension text", [&]() -> GSErrCode {
            return ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_All, true);

            //return ACAPI_Element_ChangeMemo(elemGuid_n, APIMemoMask_AdditionalPolygon, &memo);
        });
        if (err == APIERR_REFUSEDCMD) { // Я сказал надо!
            if (!ACAPI_Element_Filter (elemGuid, APIFilt_InMyWorkspace)) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return err;
            }
            if (!ACAPI_Element_Filter (elemGuid, APIFilt_HasAccessRight)) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return err;
            }
            if (!ACAPI_Element_Filter (elemGuid, APIFilt_IsEditable)) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return err;
            }

            //err = ACAPI_Element_ChangeMemo(elemGuid_n, APIMemoMask_AdditionalPolygon, &memo);
            err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_All, true);
        }
    }
    if (err != NoError) msg_rep ("DimAutoRound", "ACAPI_Element_Change_1", err, elemGuid);
    ACAPI_DisposeElemMemoHdls (&memo);
    return err;
}

// -----------------------------------------------------------------------------
// Обрабатывает размер и решает - что с ним делать
//	flag_change - менять текст размера, сбросить или не менять (DIM_CHANGE_ON, DIM_CHANGE_OFF, DIM_NOCHANGE)
//	flag_highlight - изменять перо текста, сбросить на оригинальное или не менять (DIM_HIGHLIGHT_ON, DIM_HIGHLIGHT_OFF, DIM_NOCHANGE)
// -----------------------------------------------------------------------------
bool DimParse (const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, DimRule& dimrule, ParamDictValue& propertyParams)
{
    flag_change = DIM_NOCHANGE;
    flag_highlight = DIM_NOCHANGE;
    Int32 round_value = dimrule.round_value;
    if (round_value < 1) round_value = 1;
    double dimVal_r = round (dimVal * 1000.0);
    Int32 dimValmm_round = ceil_mod ((GS::Int32) dimVal_r, round_value);
    double dx = abs (dimVal_r - dimValmm_round * 1.0); // Разница в размерах в мм
    GS::UniString custom_txt = GS::UniString::Printf ("%d", dimValmm_round);
    bool flag_expression = false; //В описании найдена формула
    if (!dimrule.expression.IsEmpty ()) {
        if (!ParamHelpers::hasProperyDefinition (propertyParams)) ParamHelpers::AllPropertyDefinitionToParamDict (propertyParams);
        ParamDictValue pdictvalue = dimrule.paramDict;

        // Добавляем в словарь округлённое значение
        if (pdictvalue.ContainsKey ("{@gdl:measuredvalue}")) {
            ParamValue pvalue;
            ParamHelpers::ConvertIntToParamValue (pvalue, "MeasuredValue", dimValmm_round);
            pdictvalue.Get ("{@gdl:measuredvalue}").val = pvalue.val;
            pdictvalue.Get ("{@gdl:measuredvalue}").isValid = true;
        }
        if (elemGuid != APINULLGuid) {
            ClassificationFunc::SystemDict systemdict;
            ParamHelpers::Read (elemGuid, pdictvalue, propertyParams, systemdict);
        }//Получим значения, если размер привязан к элементу
        GS::UniString expression = dimrule.expression;

        // Заменяем вычисленное
        if (ParamHelpers::ReplaceParamInExpression (pdictvalue, expression)) {

            // Вычисляем значения
            flag_expression = true;
            if (expression.Contains ("<") && expression.Contains (">")) {
                flag_expression = EvalExpression (expression);
            }
            ReplaceCR (expression);
            custom_txt = expression;
        }
    }

    //Если указано округление до нуля - просто подсветим кривые размеры
    if (round_value < 1) {
        if (contentType == API_NoteContent_Custom) flag_change = DIM_CHANGE_OFF;
        if (contentType == API_NoteContent_Measured) flag_change = DIM_NOCHANGE;
        if (dx > 0.099) {
            flag_highlight = DIM_HIGHLIGHT_ON;
        } else {
            flag_highlight = DIM_HIGHLIGHT_OFF;
        }
    } else {

        // Если стоит пользовательский текст - сверим с вычисленным значением
        if (contentType == API_NoteContent_Custom) {
            if (flag_change == DIM_NOCHANGE && flag_expression == false && dx < 1.0) {
                flag_change = DIM_CHANGE_OFF;
                flag_highlight = DIM_HIGHLIGHT_OFF;
            }
            if (flag_change == DIM_NOCHANGE && custom_txt != content) {
                flag_highlight = DIM_HIGHLIGHT_ON;
                flag_change = DIM_CHANGE_ON;
            }
        }

        // Если стоит автотекст и формул нет - снимем выделение
        if (contentType == API_NoteContent_Measured) {
            if (flag_change == DIM_NOCHANGE && (flag_expression == true || dx >= 1.0)) {
                flag_change = DIM_CHANGE_ON;
                flag_highlight = DIM_HIGHLIGHT_ON;
            } else {
                flag_highlight = DIM_HIGHLIGHT_OFF;
            }
        }
    }
    if (flag_expression && flag_change == DIM_CHANGE_ON) flag_change = DIM_CHANGE_FORCE;
    if (flag_change == DIM_CHANGE_ON || flag_change == DIM_CHANGE_FORCE) content = custom_txt;
    return (flag_change != DIM_NOCHANGE || flag_highlight != DIM_NOCHANGE);
}

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// -----------------------------------------------------------------------------
void DimRoundAll (const SyncSettings& syncSettings)
{
    (void) syncSettings;
    DoneElemGuid doneelemguid;
    DimRules dimrules;
    DBPrintf ("== SMSTF == DimRoundAll start\n");
    const GSErrCode err = DimReadPref (dimrules);
    if (dimrules.GetSize () == 0 || err != NoError) return;
    bool flag_chanel = false;
    ParamDictValue propertyParams;
    if (!flag_chanel) flag_chanel = DimRoundByType (API_DimensionID, doneelemguid, dimrules, propertyParams);

    //if (!flag_chanel) flag_chanel = DimRoundByType(API_RadialDimensionID, doneelemguid, dimrules, propertyParams);
    //if (!flag_chanel) flag_chanel = DimRoundByType(API_LevelDimensionID, doneelemguid, dimrules, propertyParams);
    DBPrintf ("== SMSTF == DimRoundAll end\n");
}

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// -----------------------------------------------------------------------------
bool DimRoundByType (const API_ElemTypeID& typeID, DoneElemGuid& doneelemguid, DimRules& dimrules, ParamDictValue& propertyParams)
{
    GSErrCode	err = NoError;
    GS::Array<API_Guid>	guidArray;
    err = ACAPI_Element_GetElemList (typeID, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (guidArray.GetSize () == 0 || err != NoError) return false;
    if (err == NoError) {
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
            if (!doneelemguid.ContainsKey (guidArray.Get (i))) {
                err = DimAutoRound (guidArray.Get (i), dimrules, propertyParams);
                if (err == NoError) doneelemguid.Add (guidArray.Get (i), false);
            }
#if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return true;
#else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return true;
#endif
        }
    } else {
        msg_rep ("DimAutoRound", "ACAPI_Element_GetElemList", err, APINULLGuid);
    }
    return false;
}