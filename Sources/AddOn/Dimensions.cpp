//------------ kuvbur 2022 ------------`
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Dimensions.hpp"
#include	"Propertycache.hpp"

#define DIM_NOCHANGE 0
#define DIM_HIGHLIGHT_ON 1
#define DIM_HIGHLIGHT_OFF 2
#define DIM_CHANGE_ON 1
#define DIM_CHANGE_FORCE 2
#define DIM_CHANGE_OFF 3


// -----------------------------------------------------------------------------
// Обработка одного размера
// -----------------------------------------------------------------------------
GSErrCode DimAutoRound (const API_Guid& elemGuid, const SyncSettings& syncSettings, bool isUndo)
{
    if (!PROPERTYCACHE ().hasDimAutotext) return NoError;
    if (!ACAPI_Element_Filter (elemGuid, APIFilt_InMyWorkspace)) {
        return NoError;
    }
    if (!ACAPI_Element_Filter (elemGuid, APIFilt_HasAccessRight)) {
        return NoError;
    }
    if (!ACAPI_Element_Filter (elemGuid, APIFilt_IsEditable)) {
        return NoError;
    }
    if (!ACAPI_Element_Filter (elemGuid, APIFilt_IsVisibleByRenovation)) {
        return NoError;
    }
    if (!ACAPI_Element_Filter (elemGuid, APIFilt_IsInStructureDisplay)) {
        return NoError;
    }

    API_ElemTypeID elementType;
    if (GetTypeByGUID (elemGuid, elementType) != NoError) return NoError;
    if (elementType != API_DimensionID) return NoError;
    GSErrCode err = NoError;
    if (syncSettings.syncMon) {
        err = AttachObserver (elemGuid, syncSettings);
        if (err == APIERR_LINKEXIST)
            err = NoError;
        if (err != NoError) {
            msg_rep ("DimAutoRound", "AttachObserver", err, elemGuid);
            return err;
        }
    }
    API_Element element = {}; BNZeroMemory (&element, sizeof (API_Element));
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
    GS::UniString kstr = GS::UniString::Printf ("%d", pen_dimenstion);
    GS::Array<DimRule> rules = {}; //Массив найденных правил

    const DimRules& dimrules = PROPERTYCACHE ().dimrules;
    if (pen_dimenstion > 0 && dimrules.ContainsKey (kstr)) {
        DimRule d = dimrules.Get (kstr);
        rules.Push (d);
    }
    // Если в файле только одно правило, и оно уже найдено - не смыслы запрашивать имя слоя
    if (!(!rules.IsEmpty () && dimrules.GetSize () == 1)) {
        API_Attribute layer;
        BNZeroMemory (&layer, sizeof (API_Attribute));
        layer.header.typeID = API_LayerID;
        layer.header.index = element.header.layer;
        if (ACAPI_Attribute_Get (&layer) != NoError) return err;
        GS::UniString layert = GS::UniString::Printf ("%s", layer.header.name);
        for (GS::HashTable<GS::UniString, DimRule>::ConstPairIterator cIt = dimrules.EnumeratePairs (); cIt != NULL; ++cIt) {
            #if defined(AC_28) || defined(AC_29)
            const GS::UniString& regexpstring = cIt->key;
            #else
            const GS::UniString& regexpstring = *cIt->key;
            #endif
            if (layert.Contains (regexpstring)) {
                DimRule d = dimrules.Get (regexpstring);
                rules.Push (d);
            }
        }
    }
    // Нет подходящего привали - выходим
    if (rules.IsEmpty ()) return err;
    API_ElementMemo memo = {};
    BNZeroMemory (&memo, sizeof (API_ElementMemo));
    err = ACAPI_Element_GetMemo (element.header.guid, &memo);
    if (err != NoError) {
        msg_rep ("DimAutoRound", "ACAPI_Element_GetMemo", err, elemGuid);
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    bool flag_write = false;
    for (const auto& dimrule : rules) {
        pen_rounded = dimrule.pen_rounded;
        flag_change_rule = dimrule.flag_change;
        short pen = pen_rounded;
        pen_original = pen_dimenstion; // Быстрофикс
        API_Guid bef_elemGuid = (*memo.dimElems)[0].base.base.guid;
        API_ElemTypeID elementType;
        #if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
        elementType = (*memo.dimElems)[0].base.base.type.typeID;
        #else
        elementType = (*memo.dimElems)[0].base.base.typeID;
        #endif // AC_26
        if (elementType == API_OpeningID) {
            ACAPI_DisposeElemMemoHdls (&memo);
            return err;
        }
        for (Int32 k = 1; k < element.dimension.nDimElem; k++) {
            UInt32 flag_change = DIM_NOCHANGE;
            UInt32 flag_highlight = DIM_NOCHANGE;
            #if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
            elementType = (*memo.dimElems)[k].base.base.type.typeID;
            #else
            elementType = (*memo.dimElems)[k].base.base.typeID;
            #endif // AC_26

            // TODO Баг в архикаде - при обработке размеров, привязанных к колонне - они слетают.
            if ((*memo.dimElems)[k].dimVal == 0 && elementType == API_ColumnID) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return err;
            };
            if (elementType == API_OpeningID) {
                ACAPI_DisposeElemMemoHdls (&memo);
                return err;
            }
            if ((*memo.dimElems)[k].dimVal == 0) {
                continue;
            }
            GS::UniString content = GS::UniString::Printf ("%s", (*memo.dimElems)[k].note.content);
            API_Guid ref_elemGuid = (*memo.dimElems)[k].base.base.guid;
            bool is_sameGUID = (ref_elemGuid == bef_elemGuid);
            if (!is_sameGUID) ref_elemGuid = APINULLGuid;
            bool is_wall = (elementType == API_WallID);
            bool flag_deletewall = is_wall && is_sameGUID && dimrule.flag_deletewall;
            API_NoteContentType contentType = (*memo.dimElems)[k].note.contentType;
            if (!flag_deletewall && !dimrule.flag_reset && DimParse ((*memo.dimElems)[k].dimVal, ref_elemGuid, contentType, content, flag_change, flag_highlight, dimrule)) {
                if (!flag_change_rule && flag_change != DIM_CHANGE_FORCE) flag_change = DIM_CHANGE_OFF;
                if (flag_change == DIM_CHANGE_ON || flag_change == DIM_CHANGE_FORCE) {
                    flag_write = true;
                    (*memo.dimElems)[k].note.contentType = API_NoteContent_Custom;
                    if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                        delete (*memo.dimElems)[k].note.contentUStr;
                    (*memo.dimElems)[k].note.contentUStr = new GS::UniString (content);
                }
                if (flag_change == DIM_CHANGE_OFF && (*memo.dimElems)[k].note.contentType != API_NoteContent_Measured && flag_change_rule) {
                    flag_write = true;
                    (*memo.dimElems)[k].note.contentType = API_NoteContent_Measured;
                    if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                        delete (*memo.dimElems)[k].note.contentUStr;
                    (*memo.dimElems)[k].note.contentUStr = new GS::UniString ("");
                }
                if (flag_highlight == DIM_HIGHLIGHT_ON) pen = pen_rounded;
                if (flag_highlight == DIM_HIGHLIGHT_OFF) pen = pen_original;
                if (flag_highlight != DIM_NOCHANGE && (*memo.dimElems)[k].note.notePen != pen) {
                    flag_write = true;
                    (*memo.dimElems)[k].note.notePen = pen;
                }
            }
            // Удаление толщин стен
            if (flag_deletewall && !content.IsEmpty ()) {
                flag_write = true;
                (*memo.dimElems)[k].note.contentType = API_NoteContent_Custom;
                if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                    delete (*memo.dimElems)[k].note.contentUStr;
                (*memo.dimElems)[k].note.contentUStr = new GS::UniString ("");
            }
            // Сброс пользовательского текста
            if (dimrule.flag_reset && (*memo.dimElems)[k].note.contentType != API_NoteContent_Measured) {
                flag_write = true;
                (*memo.dimElems)[k].note.notePen = pen_original;
                (*memo.dimElems)[k].note.contentType = API_NoteContent_Measured;
                if ((*memo.dimElems)[k].note.contentUStr != nullptr)
                    delete (*memo.dimElems)[k].note.contentUStr;
                (*memo.dimElems)[k].note.contentUStr = new GS::UniString ("");
            }
            // Проверка перебитых размеров
            if (dimrule.flag_custom) {
                if (contentType == API_NoteContent_Custom && (*memo.dimElems)[k].note.notePen != pen_rounded) {
                    flag_write = true;
                    (*memo.dimElems)[k].note.notePen = pen_rounded;
                }
                if (contentType == API_NoteContent_Measured && (*memo.dimElems)[k].note.notePen != pen_original && !flag_write) {
                    flag_write = true;
                    (*memo.dimElems)[k].note.notePen = pen_original;
                }
            }
            bef_elemGuid = (*memo.dimElems)[k].base.base.guid;
        }
    }
    if (flag_write) {
        API_Element mask = {};
        ACAPI_ELEMENT_MASK_CLEAR (mask);
        if (!isUndo) {
            err = ACAPI_CallUndoableCommand ("Change dimension text", [&]() -> GSErrCode {
                return ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_All, true);
            });
        } else {
            err = ACAPI_Element_Change (&element, &mask, &memo, APIMemoMask_All, true);
        }
        if (err == APIERR_REFUSEDCMD) { // Я сказал надо!
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
bool DimParse (const double& dimVal, const API_Guid& elemGuid, API_NoteContentType& contentType, GS::UniString& content, UInt32& flag_change, UInt32& flag_highlight, const DimRule& dimrule)
{
    flag_change = DIM_NOCHANGE;
    flag_highlight = DIM_NOCHANGE;
    Int32 round_value = dimrule.round_value;
    if (round_value < 1) round_value = 1;
    double dimVal_r = round (dimVal * 1000.0);
    Int32 dimValmm_round = 0;
    if (dimrule.classic_round_mode) {
        dimValmm_round = ceil_mod_classic ((GS::Int32) dimVal_r, round_value);
    } else {
        dimValmm_round = ceil_mod ((GS::Int32) dimVal_r, round_value);
    }
    double dx = fabs (dimVal_r - dimValmm_round * 1.0); // Разница в размерах в мм
    GS::UniString custom_txt = GS::UniString::Printf ("%d", dimValmm_round);
    bool flag_expression = false; //В описании найдена формула
    if (!dimrule.expression.IsEmpty ()) {
        ParamDictValue pdictvalue = dimrule.paramDict;
        // Добавляем в словарь округлённое значение
        if (pdictvalue.ContainsKey ("{@gdl:measuredvalue}")) {
            ParamValue pvalue;
            ParamHelpers::ConvertIntToParamValue (pvalue, "MeasuredValue", dimValmm_round);
            pdictvalue.Get ("{@gdl:measuredvalue}").val = pvalue.val;
            pdictvalue.Get ("{@gdl:measuredvalue}").isValid = true;
        }
        if (elemGuid != APINULLGuid) {
            ParamHelpers::Read (elemGuid, pdictvalue);
        }//Получим значения, если размер привязан к элементу
        GS::UniString expression = dimrule.expression;

        // Заменяем вычисленное
        if (ParamHelpers::ReplaceParamInExpression (pdictvalue, expression)) {

            // Вычисляем значения
            flag_expression = true;
            if (expression.Contains (str_formula_start) && expression.Contains (str_formula_end)) {
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

void DimRoundOne (const API_Guid& elemGuid, const SyncSettings& syncSettings, bool isUndo)
{
    (void) syncSettings;
    if (!PROPERTYCACHE ().hasDimAutotext) return;
    DoneElemGuid doneelemguid = {};
    #if defined(TESTING)
    DBprnt ("DimRoundAll start");
    #endif
    bool flag_chanel = false;
    DimAutoRound (elemGuid, syncSettings, isUndo);
}

// -----------------------------------------------------------------------------
// Округление всего доступного согласно настроек
// -----------------------------------------------------------------------------
void DimRoundAll (const SyncSettings& syncSettings, bool isUndo)
{
    DoneElemGuid doneelemguid = {};
    if (!PROPERTYCACHE ().hasDimAutotext) return;
    #if defined(TESTING)
    DBprnt ("DimRoundAll start");
    #endif
    bool flag_chanel = false;
    if (!flag_chanel) flag_chanel = DimRoundByType (API_DimensionID, doneelemguid, syncSettings, isUndo);
    #if defined(TESTING)
    DBprnt ("DimRoundAll end");
    #endif
}

// -----------------------------------------------------------------------------
// Округление одного типа размеров
// -----------------------------------------------------------------------------
bool DimRoundByType (const API_ElemTypeID& typeID, DoneElemGuid& doneelemguid, const SyncSettings& syncSettings, bool isUndo)
{
    GSErrCode err = NoError;
    GS::Array<API_Guid>	guidArray = {};
    err = ACAPI_Element_GetElemList (typeID, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace);
    if (err != NoError) msg_rep ("DimAutoRound", "ACAPI_Element_GetElemList", err, APINULLGuid);
    if (guidArray.IsEmpty ()) return false;
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        if (!doneelemguid.ContainsKey (guidArray.Get (i))) {
            err = DimAutoRound (guidArray.Get (i), syncSettings, isUndo);
            if (err == NoError) doneelemguid.Add (guidArray.Get (i), false);
        }
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ()) return true;
        #else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return true;
        #endif
    }
    return false;
}
