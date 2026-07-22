//------------ kuvbur 2022 ------------
#include "ACAPinc.h"
#include "APIEnvir.h"
#include <cmath>
#include <limits>
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    #include "MEPv1.hpp"
#endif // AC_27
#ifdef TESTING
    #include "TestFunc.hpp"
#endif
#include "HashSet.hpp"
#include "Helpers.hpp"
#include "Model3D/MeshBody.hpp"
#include "Model3D/model.h"
#include "ProfileAdditionalInfo.hpp"
#include "ProfileVectorImage.hpp"
#include "ProfileVectorImageOperations.hpp"
#include "Propertycache.hpp"
#include "VectorImageIterator.hpp"

int IsDummyModeOn () { return DUMMY_MODE_OFF; }

namespace FormatStringFunc {
    FormatString GetFormatStringFromFormula (const GS::UniString &formula,
                                             const GS::UniString &part,
                                             GS::UniString &stringformat) {
        FormatString f = ParseFormatString (DEFULTREALFSTRING);
        if (!formula.Contains (DOT))
            return f;
        GS::UniString texpression = formula;
        GS::UniString texpression_ = formula;
        FormatStringFunc::ReplaceMeters (texpression);
        if (!texpression.Contains (METERS))
            return f;
        GS::UniString tpart = part;
        texpression.ReplaceAll (BRACESTART, EMPTYSTRING);
        texpression.ReplaceAll (BRACEEND, EMPTYSTRING);
        texpression_.ReplaceAll (BRACESTART, EMPTYSTRING);
        texpression_.ReplaceAll (BRACEEND, EMPTYSTRING);
        tpart.ReplaceAll (BRACESTART, EMPTYSTRING);
        tpart.ReplaceAll (BRACEEND, EMPTYSTRING);
        UInt32 n_start = texpression.FindFirst (tpart) + tpart.GetLength (); // Индекс начала поиска строки-формата
        GS::UniString stringformat_ =
            texpression.GetSubstring (CHARFORMULAEND, CHARMETERS, n_start) + METERS; // Предположительно, строка-формат
        if (stringformat_.IsEmpty ())
            stringformat_ = texpression.GetSubstring (CHARDQUT, CHARMETERS, n_start) + METERS;
        if (stringformat_.Contains (DOT) && !stringformat_.Contains (SPACESTRING)) {
            // Проверим, не обрезали ли лишнюю m
            n_start = texpression.FindFirst (stringformat_) - 1;
            UInt32 n_end = n_start + stringformat_.GetLength ();
            if (n_end + 1 < texpression.GetLength ()) {
                GS::UniString endm = texpression.ToLowerCase ().GetSubstring (n_end + 1, 1);
                if (endm.IsEqual (METERS) || endm.IsEqual (DOTSET) || endm.IsEqual (RDSET) || endm.IsEqual (FSET)) {
                    n_end = n_end + 1;
                }
            }
            stringformat = texpression_.GetSubstring (n_start + 1, n_end - n_start);
#ifdef TESTING
            DBtest (!stringformat.Contains (CHARDQUT),
                    "GetFormatStringFromFormula : stringformat.Contains('\"') " + stringformat,
                    false);
            DBtest (!stringformat.Contains (CHARFORMULAEND),
                    "GetFormatStringFromFormula : stringformat.Contains(CHARFORMULAEND) " + stringformat,
                    false);
            DBtest (!stringformat.Contains (CHARPROC),
                    "GetFormatStringFromFormula : stringformat.Contains(CHARPROC) " + stringformat,
                    false);
            DBtest (!stringformat.Contains (CHARBRACEEND),
                    "GetFormatStringFromFormula : stringformat.Contains(CHARBRACEEND) " + stringformat,
                    false);
#endif
            stringformat.Trim (CHARDQUT);
            stringformat.Trim (CHARFORMULAEND);
            stringformat.Trim (CHARPROC);
            stringformat.Trim (CHARBRACEEND);
            stringformat.Trim ();
            f = FormatStringFunc::ParseFormatString (stringformat);
        }
        return f;
    }

    // -----------------------------------------------------------------------------
    // Обработка количества нулей и единиц измерения в имени свойства
    // Удаляет из имени paramName найденные единицы измерения
    // Возвращает строку для скармливания функции NumToStig
    // -----------------------------------------------------------------------------
    GS::UniString GetFormatString (GS::UniString &paramName) {
        GS::UniString formatstring = "";
        if (!paramName.Contains (DOT))
            return formatstring;
        auto &cache = PROPERTYCACHE ();
        if (!paramName.Contains (cache.meterString) && !paramName.Contains (METERS))
            return formatstring;
        GS::Array<GS::UniString> partstring = {};
        UInt32 n = StringSplt (paramName, DOT, partstring, true);
        if (n > 1) {
            formatstring = partstring[n - 1];
            if (formatstring.Contains (METERS) || formatstring.Contains (cache.meterString)) {
                if (formatstring.Contains (CharENTER)) {
                    UIndex attribinx = formatstring.FindLast (CharENTER);
                    formatstring = formatstring.GetSubstring (0, attribinx);
                }

                if (IsValid (formatstring)) {
                    paramName.ReplaceAll (DOT + formatstring, EMPTYSTRING);
                    ReplaceMeters (formatstring);
                } else {
                    formatstring = EMPTYSTRING;
                }
            } else {
                // Если .м найдена не в последнем блоке - то это не строка-формат
                formatstring = EMPTYSTRING;
            }
        }
        return formatstring;
    }

    bool IsValid (GS::UniString formatstring) {
        ReplaceMeters (formatstring);
        if (PROPERTYCACHE ().parsedformatstring.ContainsKey (formatstring))
            return true;
        if (!formatstring.Contains (METERS))
            return false;
        formatstring.ReplaceAll (METERS, EMPTYSTRING);
        formatstring.ReplaceAll (DOT, EMPTYSTRING);
        formatstring.ReplaceAll (ZEROSTRING, EMPTYSTRING);
        formatstring.ReplaceAll (SPACESTRING, EMPTYSTRING);
        if (formatstring.IsEmpty ())
            return true;
        if (formatstring.Contains ("1"))
            formatstring.ReplaceAll ("1", EMPTYSTRING);
        if (formatstring.Contains ("2"))
            formatstring.ReplaceAll ("2", EMPTYSTRING);
        if (formatstring.Contains ("3"))
            formatstring.ReplaceAll ("3", EMPTYSTRING);
        if (formatstring.IsEmpty ())
            return true;
        if (formatstring.Contains (CSTRING))
            formatstring.ReplaceAll (CSTRING, EMPTYSTRING);
        if (formatstring.Contains (DSTRING))
            formatstring.ReplaceAll (DSTRING, EMPTYSTRING);
        if (formatstring.Contains (DOTSET))
            formatstring.ReplaceAll (DOTSET, EMPTYSTRING);
        if (formatstring.Contains (RDSET))
            formatstring.ReplaceAll (RDSET, EMPTYSTRING);
        if (formatstring.Contains (FSET))
            formatstring.ReplaceAll (FSET, EMPTYSTRING);
        if (formatstring.Contains (GSTRING))
            formatstring.ReplaceAll (GSTRING, EMPTYSTRING);
        if (formatstring.Contains (KSTRING))
            formatstring.ReplaceAll (KSTRING, EMPTYSTRING);
        if (formatstring.Contains ("4"))
            formatstring.ReplaceAll ("3", EMPTYSTRING);
        if (formatstring.IsEmpty ())
            return true;
        for (UInt32 i = 4; i < 10; i++) {
            formatstring.ReplaceAll (GS::UniString::Printf ("%d", i), EMPTYSTRING);
            if (formatstring.IsEmpty ())
                return true;
        }
        if (formatstring.IsEmpty ())
            return true;
        return false;
    }

    void ReplaceMeters (GS::UniString &formatstring) {
        if (formatstring.IsEmpty ())
            return;
        auto &cache = PROPERTYCACHE ();
        if (!cache.isEng_OK)
            cache.ReadisEng ();
        if (formatstring.Contains (cache.meterString))
            formatstring.ReplaceAll (cache.meterString, METERS);
        if (formatstring.Contains (cache.santimeterString))
            formatstring.ReplaceAll (cache.santimeterString, DSTRING);
        if (formatstring.Contains (cache.decimeterString))
            formatstring.ReplaceAll (cache.decimeterString, CSTRING);
    }

    // -----------------------------------------------------------------------------
    // Извлекает из строки информацио о единицах измерении и округлении
    // -----------------------------------------------------------------------------
    FormatString ParseFormatString (const GS::UniString &stringformat) {
        auto &cache = PROPERTYCACHE ();
        if (const auto *cached = cache.parsedformatstring.GetPtr (stringformat)) {
            return *cached;
        }
        int n_zero = 3;
        Int32 krat = 0;         // Крутность округления
        double koeff = 1;       // Коэфф. увеличения
        bool trim_zero = true;  // Требуется образать нули после запятой
        bool needround = false; // Требуется округлить численное значение для вычислений
        bool forceRaw = false;  // Использовать неокруглённое значение для записи
        bool delimetr_iscomma = true;
        FormatString format = {};
        format.stringformat = stringformat;
        format.isEmpty = true;
        if (!stringformat.IsEmpty ()) {
            GS::UniString outstringformat = stringformat;
            if (stringformat.Contains (DOT)) {
                outstringformat.ReplaceAll (DOT, EMPTYSTRING);
                format.stringformat.ReplaceAll (DOT, EMPTYSTRING);
            }
            ReplaceMeters (outstringformat);
            if (outstringformat.Contains (METERS)) {
                if (outstringformat.Contains (MMETERS)) {
                    n_zero = 0;
                    koeff = 1000;
                    outstringformat.ReplaceAll (MMETERS, EMPTYSTRING);
                } else {
                    if (outstringformat.Contains (CMETERS)) {
                        n_zero = 1;
                        koeff = 100;
                        outstringformat.ReplaceAll (CMETERS, EMPTYSTRING);
                    } else {
                        if (outstringformat.Contains (DMETERS)) {
                            n_zero = 2;
                            koeff = 10;
                            outstringformat.ReplaceAll (DMETERS, EMPTYSTRING);
                        } else {
                            if (outstringformat.Contains (GMETERS)) {
                                koeff = 1 / 100;
                                outstringformat.ReplaceAll (GMETERS, EMPTYSTRING);
                            } else {
                                if (outstringformat.Contains (KMETERS)) {
                                    koeff = 1 / 1000;
                                    outstringformat.ReplaceAll (KMETERS, EMPTYSTRING);
                                } else {
                                    koeff = 1;
                                    n_zero = 3;
                                    outstringformat.ReplaceAll (METERS, EMPTYSTRING);
                                }
                            }
                        }
                    }
                }
            }
            if (!outstringformat.IsEmpty ()) {
                if (outstringformat.Contains (DOTSET)) {
                    delimetr_iscomma = false;
                    outstringformat.ReplaceAll (DOTSET, EMPTYSTRING);
                }
            }
            if (!outstringformat.IsEmpty ()) {
                if (outstringformat.Contains (RDSET)) {
                    needround = true;
                    outstringformat.ReplaceAll (RDSET, EMPTYSTRING);
                }
            }
            if (!outstringformat.IsEmpty ()) {
                if (outstringformat.Contains (FSET)) {
                    forceRaw = true;
                    outstringformat.ReplaceAll (FSET, EMPTYSTRING);
                }
            }
            // Принудительный вывод заданного кол-ва нулей после запятой
            if (!outstringformat.IsEmpty ()) {
                if (outstringformat.Contains (ZEROSTRING)) {
                    outstringformat.ReplaceAll (ZEROSTRING, EMPTYSTRING);
                    outstringformat.Trim ();
                    if (!outstringformat.IsEmpty ())
                        trim_zero = false;
                }
            }
            if (!outstringformat.IsEmpty ()) {
                n_zero = std::atoi (outstringformat.ToCStr ());
            }
            format.isEmpty = false;
            format.isRead = true;
        }
        format.forceRaw = forceRaw;
        format.needRound = needround;
        if (delimetr_iscomma) {
            format.delimetr = COMMA;
        } else {
            format.delimetr = DOT;
        }
        format.n_zero = n_zero;
        format.krat = krat;
        format.koeff = koeff;
        format.trim_zero = trim_zero;
        cache.parsedformatstring.Put (stringformat, format);
        return format;
    }

    // -----------------------------------------------------------------------------
    // Переводит число в строку согласно настройкам строки-формата
    // -----------------------------------------------------------------------------
    GS::UniString NumToString (double var, const FormatString &stringformat) {
        if (fabs (var) < 0.00000001)
            return ZEROSTRING;
        double outvar = var * stringformat.koeff;
        outvar = round_nzero (outvar, stringformat.n_zero);
        if (stringformat.krat > 0) {
            outvar = ceil_mod ((GS::Int32)outvar, stringformat.krat);
        }
        int precision = (stringformat.n_zero >= 0 && stringformat.n_zero < 18) ? stringformat.n_zero : 6;
        char buf[128];
        int len = std::snprintf (buf, sizeof (buf), "%.*f", precision, outvar);
        if (len <= 0 || len >= (int)sizeof (buf)) {
            return ZEROSTRING;
        }

        int sep_idx = -1;
        for (int i = 0; i < len; ++i) {
            if (buf[i] == '.' || buf[i] == ',') {
                sep_idx = i;
                break;
            }
        }

        if (sep_idx != -1 && stringformat.trim_zero) {
            int back = len - 1;
            while (back > sep_idx && buf[back] == '0') {
                buf[back] = '\0';
                back--;
            }
            len = back + 1;
            if (buf[back] == '.' || buf[back] == ',') {
                buf[back] = '\0';
                len = back;
                sep_idx = -1;
            }
        }
        // Пишем нужный разделитель сразу — без нормализации в точку и без ReplaceAll
        if (sep_idx != -1)
            buf[sep_idx] = !stringformat.delimetr.IsEmpty () ? (char)stringformat.delimetr.GetChar (0) : '.';
        return GS::UniString (buf);
    }
} // namespace FormatStringFunc

// -------------------------------------------------------------------------------
// Функция для получения правил из массива элементов (по имени правила и наличию скобок в описании)
// Если в массиве один элемент - будет произведён поиск по классификации
// В этом случае в массив элементов будут добавлены элементы, у которых видны правила
// -------------------------------------------------------------------------------
bool GetRuleFromSelected (GS::Array<API_Guid> &guidArray,
                          GS::HashTable<API_Guid, API_PropertyDefinition> &definitions,
                          const GS::UniString &name,
                          bool check_bracket) {
    if (guidArray.GetSize () == 1) {
        GetRuleFromSelected (guidArray[0], definitions, name, check_bracket);
        if (definitions.IsEmpty ())
            return false;
        guidArray.Clear ();
        GetElementForPropertyDefinition (definitions, guidArray);
    } else {
        GetUnicGuid (guidArray);
        for (const API_Guid &elemGuid : guidArray) {
            GetRuleFromSelected (elemGuid, definitions, name, check_bracket);
        }
    }
    return (!definitions.IsEmpty ());
}

// -------------------------------------------------------------------------------
// Функция для выбора элементов, в которых видимо выбранное свойство
// -------------------------------------------------------------------------------
void GetElementForPropertyDefinition (const GS::HashTable<API_Guid, API_PropertyDefinition> &definitions,
                                      GS::Array<API_Guid> &guidArray) {
#if defined(AC_22)
    return;
#else
    UnicGuid unguid;
    UnicGuid unguid_by_class;
    ParamValue pvalue;
    GSErrCode err = NoError;
    GS::Array<API_Guid> elemGuids = {};
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    GS::Array<API_Property> properties;
    for (const auto &cIt : definitions) {
    #if defined(AC_28) || defined(AC_29)
        const API_PropertyDefinition &definition = cIt.value;
    #else
        const API_PropertyDefinition &definition = *cIt.value;
    #endif
        bool has_elems = false;
        unguid_by_class.Clear ();
        for (const API_Guid &classificationItemGuid : definition.availability) {
            elemGuids.Clear ();
            err = ACAPI_Element_GetElementsWithClassification (classificationItemGuid, elemGuids);
            if (err != NoError) {
                msg_rep (
                    "GetElementForPropertyDefinition", "ACAPI_Element_GetElementsWithClassification", err, APINULLGuid);
                continue;
            }
            if (!elemGuids.IsEmpty ())
                has_elems = true;
            for (const API_Guid &elemGuid : elemGuids) {
                if (!unguid_by_class.ContainsKey (elemGuid))
                    unguid_by_class.Put (elemGuid, true);
            }
        }
        // Отсеиваем элементы с невалидным свойством
        if (has_elems) {
            propertyDefinitions.Clear ();
            properties.Clear ();
            propertyDefinitions.Push (definition);
            guidArray.SetCapacity (guidArray.GetCapacity () + unguid_by_class.GetSize () + 1);
            for (const auto &el : unguid_by_class) {
    #if defined(AC_28) || defined(AC_29)
                const API_Guid &elemGuid = el.key;
    #else
                const API_Guid &elemGuid = *el.key;
    #endif
                err = ACAPI_Element_GetPropertyValues (elemGuid, propertyDefinitions, properties);
                if (err != NoError) {
                    msg_rep ("GetElementForPropertyDefinition", "ACAPI_Element_GetPropertyValues", err, elemGuid);
                    continue;
                }
                for (const auto &prop : properties) {
                    pvalue.Сlear ();
                    if (!ParamHelpers::ConvertToParamValue (pvalue, prop)) {
                        continue;
                    }
                    if (!pvalue.isValid) {
                        continue;
                    }
                    if (!unguid.ContainsKey (elemGuid)) {
                        unguid.Put (elemGuid, true);
                        guidArray.Push (std::move (elemGuid));
                    }
                }
            }
        }
    }
#endif
}

// --------------------------------------------------------------------
// Функция для сбора правил с элемента
// ---------------------------------------------------------------------
bool GetRuleFromSelected (const API_Guid &elemguid,
                          GS::HashTable<API_Guid, API_PropertyDefinition> &definitions,
                          const GS::UniString &name,
                          bool check_bracket) {
#if defined(AC_22)
    return false;
#else
    GS::Array<API_PropertyDefinition> definitions_;
    GSErrCode err =
        ACAPI_Element_GetPropertyDefinitions (elemguid, API_PropertyDefinitionFilter_UserDefined, definitions_);
    if (err != NoError) {
        msg_rep ("GetRuleFromSelected", "ACAPI_Element_GetPropertyDefinitions", err, elemguid);
        return false;
    }
    if (definitions_.IsEmpty ()) {
        msg_rep ("GetRuleFromSelected", "PropertyDefinition empty", err, elemguid);
        return false;
    }
    for (const auto &definition : definitions_) {
        if (definition.description.IsEmpty ())
            continue;
        if (!definition.description.Contains (name))
            continue;
        if (check_bracket) {
            if (!definition.description.Contains (BRACESTART)) {
                msg_rep ("GetRuleFromSelected",
                         definition.name + " Check the opening bracket, there should be {",
                         APIERR_GENERAL,
                         APINULLGuid);
                continue;
            }
            if (!definition.description.Contains (BRACEEND)) {
                msg_rep ("GetRuleFromSelected",
                         definition.name + " Check the closing bracket, there should be }",
                         APIERR_GENERAL,
                         APINULLGuid);
                continue;
            }
        }
        if (definitions.ContainsKey (definition.guid))
            continue;
        definitions.Put (definition.guid, definition);
    }
    return (!definitions.IsEmpty ());
#endif
}

// -----------------------------------------------------------------------------
// Добавление отслеживания (для разных версий)
// -----------------------------------------------------------------------------
GSErrCode AttachObserver (const API_Guid &objectId, const SyncSettings &syncSettings) {
    GSErrCode err = NoError;
#ifdef AC_22
    API_Elem_Head elemHead;
    elemHead.guid = objectId;
    err = ACAPI_Element_AttachObserver (&elemHead, 0);
#else
    err = ACAPI_Element_AttachObserver (objectId);
#endif
    return err;
}

// --------------------------------------------------------------------
// Проверяет - попадает ли тип элемента в под настройки синхронизации
// --------------------------------------------------------------------
bool CheckElementType (const API_ElemTypeID &elementType, const SyncSettings &syncSettings) {
    switch (elementType) {
    case API_WallID:
    case API_ColumnID:
    case API_BeamID:
    case API_SlabID:
    case API_RoofID:
    case API_MeshID:
    case API_ShellID:
    case API_MorphID:
    case API_BeamSegmentID:
    case API_ColumnSegmentID:
        return syncSettings.wallS;

    case API_StairID:
    case API_RiserID:
    case API_TreadID:
    case API_StairStructureID:
    case API_ObjectID:
    case API_ZoneID:
    case API_LampID:
        return syncSettings.objS;

    case API_RailingID:
    case API_RailingToprailID:
    case API_RailingHandrailID:
    case API_RailingRailID:
    case API_RailingPostID:
    case API_RailingInnerPostID:
    case API_RailingBalusterID:
    case API_RailingPanelID:
    case API_RailingSegmentID:
    case API_RailingNodeID:
    case API_RailingBalusterSetID:
    case API_RailingPatternID:
    case API_RailingToprailEndID:
    case API_RailingHandrailEndID:
    case API_RailingRailEndID:
    case API_RailingToprailConnectionID:
    case API_RailingHandrailConnectionID:
    case API_RailingRailConnectionID:
    case API_RailingEndFinishID:
        return syncSettings.cwallS;

#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    case API_ExternalElemID:
        return syncSettings.objS;
#endif
    case API_CurtainWallSegmentID:
    case API_CurtainWallFrameID:
    case API_CurtainWallJunctionID:
    case API_CurtainWallAccessoryID:
    case API_CurtainWallID:
    case API_CurtainWallPanelID:
        return syncSettings.cwallS;

    case API_WindowID:
    case API_DoorID:
    case API_SkylightID:
    case API_OpeningID:
        return syncSettings.widoS;
    default:
        return false;
    }
    return false;
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid &objectId, const SyncSettings &syncSettings, const bool needCheckElementType) {
    API_ElemTypeID eltype;
    bool res = IsElementEditable (objectId, syncSettings, needCheckElementType, eltype);
    UNUSED_VARIABLE (eltype);
    return res;
}

bool IsElementEditable (const API_Elem_Head &tElemHead,
                        const SyncSettings &syncSettings,
                        const bool needCheckElementType) {
    if (tElemHead.guid == APINULLGuid)
        return false;
    // Проверяем - на находится ли объект в модуле
    if (tElemHead.hotlinkGuid != APINULLGuid)
        return false;
    if (needCheckElementType) {
        API_ElemTypeID eltype = GetElemTypeID (tElemHead);
        if (!CheckElementType (eltype, syncSettings))
            return false;
    }
    // Проверяем - зарезервирован ли объект
    return ACAPI_Element_Filter (tElemHead.guid, APIFilt_IsEditable | APIFilt_InMyWorkspace | APIFilt_HasAccessRight);
}

// -----------------------------------------------------------------------------
// Проверяет возможность редактирования объекта (не находится в модуле, разблокирован, зарезервирован)
// Возвращает тип элемента
// -----------------------------------------------------------------------------
bool IsElementEditable (const API_Guid &objectId,
                        const SyncSettings &syncSettings,
                        const bool needCheckElementType,
                        API_ElemTypeID &eltype) {
    // Проверяем - зарезервирован ли объект
    if (objectId == APINULLGuid)
        return false;
    if (!ACAPI_Element_Filter (objectId, APIFilt_IsEditable | APIFilt_InMyWorkspace | APIFilt_HasAccessRight))
        return false;
    // Проверяем - на находится ли объект в модуле
    API_Elem_Head tElemHead = {};
    tElemHead.guid = objectId;
    if (ACAPI_Element_GetHeader (&tElemHead) != NoError)
        return false;
    if (tElemHead.hotlinkGuid != APINULLGuid)
        return false;
    eltype = GetElemTypeID (tElemHead);
    if (needCheckElementType && !CheckElementType (eltype, syncSettings))
        return false;
    return true;
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Настройки будут считаны при вызове функции
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/,
                                         bool onlyEditable /*= true*/,
                                         const SyncSettings &syncSettings,
                                         bool addSubelement) {
    bool addZone = false;
    bool addConnect = false;
    if (addSubelement) {
        addZone = true;
        addConnect = true;
    }
    return GetSelectedElements (assertIfNoSel, onlyEditable, syncSettings, addSubelement, addZone, addConnect);
}

GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/,
                                         bool onlyEditable /*= true*/,
                                         bool addSubelement /*= true*/) {
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings, true);
    bool addZone = false;
    bool addConnect = false;
    if (addSubelement) {
        addZone = true;
        addConnect = true;
    }
    return GetSelectedElements (assertIfNoSel, onlyEditable, syncSettings, addSubelement, addZone, addConnect);
}

GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/,
                                         bool onlyEditable /*= true*/,
                                         bool addSubelement /*= true*/,
                                         bool addZone,
                                         bool addConnect) {
    SyncSettings syncSettings (false, false, true, true, true, true, false);
    LoadSyncSettingsFromPreferences (syncSettings, true);
    return GetSelectedElements (assertIfNoSel, onlyEditable, syncSettings, addSubelement, addZone, addConnect);
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов в соответсвии с настройками обработки
// -----------------------------------------------------------------------------
GS::Array<API_Guid> GetSelectedElements (bool assertIfNoSel /* = true*/,
                                         bool onlyEditable /*= true*/,
                                         const SyncSettings &syncSettings,
                                         bool addSubelement,
                                         bool addZone,
                                         bool addConnect) {
    GSErrCode err;
    API_SelectionInfo selectionInfo = {};
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString errorString = RSGetIndString (iseng, ErrorSelectID, ACAPI_GetOwnResModule ());
#ifdef AC_22
    API_Neig **selNeigs;
#else
    GS::Array<API_Neig> selNeigs;
#endif
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, onlyEditable);
    BMKillHandle ((GSHandle *)&selectionInfo.marquee.coords);
    if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
        if (assertIfNoSel) {
            DGAlert (DG_ERROR, "Error", errorString, EMPTYSTRING, "Ok");
        }
    }
    if (err != NoError) {
#ifdef AC_22
        BMKillHandle ((GSHandle *)&selNeigs);
#endif // AC_22
        return GS::Array<API_Guid> ();
    }
    GS::Array<API_Guid> guidArray;
#ifdef AC_22
    USize nSel = BMGetHandleSize ((GSHandle)selNeigs) / sizeof (API_Neig);
    for (USize i = 0; i < nSel; i++) {
        guidArray.Push ((*selNeigs)[i].guid);
    }
    BMKillHandle ((GSHandle *)&selNeigs);
#else
    guidArray.SetCapacity (selNeigs.GetSize ());
    for (const API_Neig &neig : selNeigs) {
        API_Guid elemguid = neig.guid;
        guidArray.Push (elemguid);
        if (addSubelement) {
            API_ElemTypeID elementType;
            API_NeigID neigID = neig.neigID;
            GSErrCode err = NoError;
    #if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
            API_ElemType elemType26;
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_Element_NeigIDToElemType (neigID, elemType26);
        #else
            err = ACAPI_Goodies_NeigIDToElemType (neigID, elemType26);
        #endif
            elementType = elemType26.typeID;
    #else
            err = ACAPI_Goodies (APIAny_NeigIDToElemTypeID, &neigID, &elementType);
    #endif // AC_26
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            if (err != NoError && neig.guid != APINULLGuid) { // На МЕР элементах функция ACAPI_Element_NeigIDToElemType
                                                              // не работает(
                err = GetTypeByGUID (neig.guid, elementType);
            }
    #endif // AC_27
            if (err == NoError)
                GetRelationsElement (elemguid, elementType, syncSettings, guidArray, addZone, addConnect);
        }
    }
#endif     // AC_22
    GetUnicGuid (guidArray);
    return guidArray;
}

// -----------------------------------------------------------------------------
// Возвращает GUID родительского элемента для API_SectElemType
// -----------------------------------------------------------------------------
void GetParentGUIDSectElem (const API_Guid &sectElemguid, API_Guid &parentguid, API_ElemTypeID &parentType) {
    API_Element elem = {};
    elem.header.guid = sectElemguid;
    GSErrCode err = ACAPI_Element_Get (&elem);
    if (err != NoError || elem.sectElem.parentGuid == APINULLGuid) {
        msg_rep ("GetParentGUIDSectElem", "ACAPI_Element_Get", err, sectElemguid);
    } else {
        parentguid = elem.sectElem.parentGuid;
#if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
        parentType = elem.sectElem.parentType.typeID;
#else
        parentType = elem.sectElem.parentID;
#endif
    }
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid SyncSettings
// -----------------------------------------------------------------------------
void CallOnSelectedElemSettings (void (*function) (const API_Guid &, const SyncSettings &),
                                 bool assertIfNoSel /* = true*/,
                                 bool onlyEditable /* = true*/,
                                 const SyncSettings &syncSettings,
                                 GS::UniString &funcname,
                                 bool addSubelement) {
    GS::Array<API_Guid> guidArray = GetSelectedElements (assertIfNoSel, onlyEditable, addSubelement);
    if (guidArray.IsEmpty ())
        return;
    GS::UniString subtitle ("working...");
    GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    bool showPercent = true;
    Int32 maxval = guidArray.GetSize ();
#endif
    ProcessWindowGuard pwGuard (funcname, nPhase);
    long time_start = clock ();
    for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
        function (guidArray[i], syncSettings);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (i % 10 == 0)
            ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
        if (i % 10 == 0)
            ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        if (ACAPI_ProcessWindow_IsProcessCanceled ())
            return;
#else
        if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
            return;
#endif
    }
    long time_end = clock ();
    GS::UniString time = GS::UniString::Printf (" %.3f s", (time_end - time_start) / 1000);
    GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
    msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// -----------------------------------------------------------------------------
void CallOnSelectedElem (void (*function) (const API_Guid &),
                         bool assertIfNoSel /* = true*/,
                         bool onlyEditable /* = true*/,
                         GS::UniString &funcname,
                         bool addSubelement) {
    GS::Array<API_Guid> guidArray = GetSelectedElements (assertIfNoSel, onlyEditable, addSubelement);
    if (!guidArray.IsEmpty ()) {
        long time_start = clock ();
        GS::UniString subtitle ("working...");
        GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
#endif
        ProcessWindowGuard pwGuard (funcname, nPhase);
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            if (i % 10 == 0)
                ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            if (i % 10 == 0)
                ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
            function (guidArray[i]);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            if (ACAPI_ProcessWindow_IsProcessCanceled ())
                return;
#else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr))
                return;
#endif
        }
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %d ms", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
        msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
    } else if (!assertIfNoSel) {
        function (APINULLGuid);
    }
}

// --------------------------------------------------------------------
// Поиск связанных элементов
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid &elemGuid,
                          const SyncSettings &syncSettings,
                          GS::Array<API_Guid> &subelemGuid) {
    GetRelationsElement (elemGuid, syncSettings, subelemGuid, true, true);
}

void GetRelationsElement (const API_Guid &elemGuid,
                          const SyncSettings &syncSettings,
                          GS::Array<API_Guid> &subelemGuid,
                          bool addZone,
                          bool addConnect) {
    API_ElemTypeID elementType = API_ZombieElemID;
    if (GetTypeByGUID (elemGuid, elementType) != NoError)
        return;
    GetRelationsElement (elemGuid, elementType, syncSettings, subelemGuid, addZone, addConnect);
}

// --------------------------------------------------------------------
// Поиск связанных элементов для определённого типа
// --------------------------------------------------------------------
void GetRelationsElement (const API_Guid &elemGuid,
                          const API_ElemTypeID &elementType,
                          const SyncSettings &syncSettings,
                          GS::Array<API_Guid> &subelemGuid,
                          bool addZone,
                          bool addConnect) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    if (syncSettings.objS && elementType == API_ExternalElemID) {
        MEPv1::GetSubElement (elemGuid, subelemGuid);
        return;
    }
#endif

    GSErrCode err = NoError;
    API_Element element = {};
    API_ElementMemo memo = {};

#ifndef AC_22
    auto GetHierarchicalOwners =
        [&] (API_Guid &owner, API_Guid &ownerRoot, API_HierarchicalElemType &type, API_HierarchicalElemType &typeRoot) {
            API_Guid elemGuid_t = elemGuid;
            API_HierarchicalOwnerType ownerType = API_ParentHierarchicalOwner;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (&elemGuid_t, &ownerType, &type, &owner);
            ownerType = API_RootHierarchicalOwner;
            ACAPI_HierarchicalEditing_GetHierarchicalElementOwner (&elemGuid_t, &ownerType, &typeRoot, &ownerRoot);
    #else
            ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &ownerType, &type, &owner);
            ownerType = API_RootHierarchicalOwner;
            ACAPI_Goodies (APIAny_GetHierarchicalElementOwnerID, &elemGuid_t, &ownerType, &typeRoot, &ownerRoot);
    #endif
        };
#endif

    switch (elementType) {
#ifndef AC_22
    case API_ColumnID:
        if (syncSettings.cwallS) {
            element.header.guid = elemGuid;
            err = ACAPI_Element_Get (&element);
            if (err != NoError) {
                msg_rep ("GetRelationsElement", "ACAPI_Element_Get", err, elemGuid);
                return;
            }
            if (element.column.nSegments == 0)
                return;
            err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment);
            if (err == NoError && memo.columnSegments != nullptr) {
                subelemGuid.SetCapacity (subelemGuid.GetSize () + element.column.nSegments);
                for (UInt32 i = 0; i < element.column.nSegments; i++) {
                    subelemGuid.Push (memo.columnSegments[i].head.guid);
                }
            }
            ACAPI_DisposeElemMemoHdls (&memo);
        }
        break;
    case API_BeamID:
        if (syncSettings.cwallS) {
            element.header.guid = elemGuid;
            err = ACAPI_Element_Get (&element);
            if (err != NoError) {
                msg_rep ("GetRelationsElement", "ACAPI_Element_Get", err, elemGuid);
                return;
            }
            if (element.beam.nSegments == 0)
                return;
            err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment);
            if (err == NoError && memo.beamSegments != nullptr) {
                subelemGuid.SetCapacity (subelemGuid.GetSize () + element.beam.nSegments);
                for (UInt32 i = 0; i < element.beam.nSegments; i++) {
                    subelemGuid.Push (memo.beamSegments[i].head.guid);
                }
            }
            ACAPI_DisposeElemMemoHdls (&memo);
        }
        break;
#endif
    case API_WallID:
        if (syncSettings.widoS && addConnect) {
            GS::Array<API_Guid> connectedElements;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_Grouping_GetConnectedElements (elemGuid,
                                                       API_WindowID,
                                                       &connectedElements,
                                                       APIFilt_IsEditable | APIFilt_HasAccessRight |
                                                           APIFilt_InMyWorkspace,
                                                       APINULLGuid);
#else
            err = ACAPI_Element_GetConnectedElements (elemGuid,
                                                      API_WindowID,
                                                      &connectedElements,
                                                      APIFilt_IsEditable | APIFilt_HasAccessRight |
                                                          APIFilt_InMyWorkspace);
#endif
            subelemGuid.SetCapacity (subelemGuid.GetSize () + connectedElements.GetSize ());
            for (const auto &guid : connectedElements)
                subelemGuid.Push (guid);
            connectedElements.Clear ();
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_Grouping_GetConnectedElements (elemGuid,
                                                       API_DoorID,
                                                       &connectedElements,
                                                       APIFilt_IsEditable | APIFilt_HasAccessRight |
                                                           APIFilt_InMyWorkspace,
                                                       APINULLGuid);
#else
            err = ACAPI_Element_GetConnectedElements (elemGuid,
                                                      API_DoorID,
                                                      &connectedElements,
                                                      APIFilt_IsEditable | APIFilt_HasAccessRight |
                                                          APIFilt_InMyWorkspace);
#endif
            subelemGuid.SetCapacity (subelemGuid.GetSize () + connectedElements.GetSize ());
            for (const auto &guid : connectedElements)
                subelemGuid.Push (guid);
            break;
        }
    case API_RailingID:
        if (syncSettings.cwallS)
            err = GetRElementsForRailing (elemGuid, subelemGuid);
        break;
    case API_CurtainWallID:
        if (syncSettings.cwallS)
            err = GetRElementsForCWall (elemGuid, subelemGuid);
        break;
    case API_CurtainWallSegmentID:
    case API_CurtainWallFrameID:
    case API_CurtainWallJunctionID:
    case API_CurtainWallAccessoryID:
    case API_CurtainWallPanelID:
        if (syncSettings.cwallS) {
            subelemGuid.SetCapacity (subelemGuid.GetSize () + addZone * 2 + 2);
            if (addZone) {
                API_CWPanelRelation crelData = {};
                err = ACAPI_Element_GetRelations (elemGuid, API_ZoneID, &crelData);
                if (err == NoError) {
                    if (crelData.fromRoom != APINULLGuid)
                        subelemGuid.Push (crelData.fromRoom);
                    if (crelData.toRoom != APINULLGuid)
                        subelemGuid.Push (crelData.toRoom);
                }
            }
#ifndef AC_22
            API_Guid owner = APINULLGuid, ownerRoot = APINULLGuid;
            API_HierarchicalElemType type = API_SingleElem, typeRoot = API_SingleElem;
            GetHierarchicalOwners (owner, ownerRoot, type, typeRoot);
            if (owner != APINULLGuid && type == API_ChildElemInMultipleElem)
                subelemGuid.Push (owner);
            if (ownerRoot != owner && ownerRoot != APINULLGuid && typeRoot == API_ChildElemInMultipleElem)
                subelemGuid.Push (ownerRoot);
#endif
        }
        break;
    case API_DoorID:
    case API_WindowID:
        if (syncSettings.widoS) {
            subelemGuid.SetCapacity (subelemGuid.GetSize () + 4);
            API_DoorRelation drelData = {};
            err = ACAPI_Element_GetRelations (elemGuid, API_ZoneID, &drelData);
            if (err == NoError) {
                if (drelData.fromRoom != APINULLGuid)
                    subelemGuid.Push (drelData.fromRoom);
                if (drelData.toRoom != APINULLGuid)
                    subelemGuid.Push (drelData.toRoom);
            }
#ifndef AC_22
            API_Guid owner = APINULLGuid, ownerRoot = APINULLGuid;
            API_HierarchicalElemType type = API_SingleElem, typeRoot = API_SingleElem;
            GetHierarchicalOwners (owner, ownerRoot, type, typeRoot);
            if (owner != APINULLGuid && type == API_ChildElemInMultipleElem)
                subelemGuid.Push (owner);
            if (ownerRoot != owner && ownerRoot != APINULLGuid && typeRoot == API_ChildElemInMultipleElem)
                subelemGuid.Push (ownerRoot);
#endif
        }
        break;
        break;
    case API_ZoneID:
        if (syncSettings.objS && addZone) {
            API_RoomRelation relData = {};
            GS::Array<API_ElemTypeID> typeinzone;
            err = ACAPI_Element_GetRelations (elemGuid, API_ZombieElemID, &relData);
            if (err == NoError) {
#if defined(AC_23) || defined(AC_22)
                for (Int32 i = 0; i < relData.nObject; i++) {
                    API_Guid elGuid = (*relData.objects)[i];
                    subelemGuid.Push (elGuid);
                }
                if (syncSettings.widoS) {
                    for (Int32 i = 0; i < relData.nWindow; i++) {
                        API_Guid elGuid = (*relData.windows)[i];
                        subelemGuid.Push (elGuid);
                    }
                    for (Int32 i = 0; i < relData.nDoor; i++) {
                        API_Guid elGuid = (*relData.doors)[i];
                        subelemGuid.Push (elGuid);
                    }
                    for (Int32 i = 0; i < relData.nSkylight; i++) {
                        API_Guid elGuid = (*relData.skylights)[i];
                        subelemGuid.Push (elGuid);
                    }
                }
                if (syncSettings.wallS) {
                    for (Int32 i = 0; i < relData.nColumn; i++) {
                        API_Guid elGuid = (*relData.columns)[i];
                        subelemGuid.Push (elGuid);
                    }
                    for (Int32 i = 0; i < relData.nWallPart; i++) {
                        API_Guid elGuid = (*relData.wallPart)[i].guid;
                        subelemGuid.Push (elGuid);
                    }
                    for (Int32 i = 0; i < relData.nBeamPart; i++) {
                        API_Guid elGuid = (*relData.beamPart)[i].guid;
                        subelemGuid.Push (elGuid);
                    }
                    for (Int32 i = 0; i < relData.nMorph; i++) {
                        API_Guid elGuid = (*relData.morphs)[i];
                        subelemGuid.Push (elGuid);
                    }
                }
#else
                static const API_ElemTypeID baseTypes[] = {
                    API_ObjectID, API_LampID, API_StairID, API_RiserID, API_TreadID, API_StairStructureID};
                for (const auto &type : baseTypes) {
                    if (const auto *arrayPtr = relData.elementsGroupedByType.GetPtr (type)) {
                        for (const auto &id : *arrayPtr)
                            subelemGuid.Push (id);
                    }
                }
                if (syncSettings.wallS) {
                    static const API_ElemTypeID wallTypes[] = {
                        API_WallID, API_SlabID, API_ColumnID, API_BeamID, API_RoofID, API_ShellID, API_MorphID};
                    for (const auto &type : wallTypes) {
                        if (const auto *arrayPtr = relData.elementsGroupedByType.GetPtr (type)) {
                            for (const auto &id : *arrayPtr)
                                subelemGuid.Push (id);
                        }
                    }
                }
                if (syncSettings.widoS) {
                    static const API_ElemTypeID widoTypes[] = {API_WindowID, API_DoorID, API_SkylightID};
                    for (const auto &type : widoTypes) {
                        if (const auto *arrayPtr = relData.elementsGroupedByType.GetPtr (type)) {
                            for (const auto &id : *arrayPtr)
                                subelemGuid.Push (id);
                        }
                    }
                }
                if (syncSettings.cwallS) {
                    if (const auto *arrayPtr = relData.elementsGroupedByType.GetPtr (API_CurtainWallID)) {
                        for (const auto &id : *arrayPtr)
                            subelemGuid.Push (id);
                    }
                }
#endif
            }
            ACAPI_DisposeRoomRelationHdls (&relData);
        }
        break;
    default:
        break;
    }
}

// -----------------------------------------------------------------------------
// Получение размеров Морфа
// Формирует словарь ParamDictValue& pdictvalue со значениями
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMorphParam (const API_Guid &guid, ParamDictValue &pdictvaluemorph) {
#if defined(TESTING)
    DBprnt ("      ReadMorphParam");
#endif
    API_ElementMemo memo = {};
    GSErrCode err = ACAPI_Element_GetMemo (guid, &memo);
    if (err != NoError || memo.morphBody == nullptr) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return false;
    }
    double L = 0;
    double Lx = 0;
    double Ly = 0;
    double Lz = 0;
    double Max_x = 0;
    double Max_y = 0;
    double Max_z = 0;
    double Min_x = std::numeric_limits<double>::max ();
    double Min_y = std::numeric_limits<double>::max ();
    double Min_z = std::numeric_limits<double>::max ();
    double A = 0;
    double B = 0;
    double ZZYZX = 0;
    if (memo.morphBody->IsWireBody () && !memo.morphBody->IsSolidBody ()) {
        Int32 edgeCnt = memo.morphBody->GetEdgeCount ();
        for (Int32 iEdge = 0; iEdge < edgeCnt; iEdge++) {
            const EDGE &edge = memo.morphBody->GetConstEdge (iEdge);
            const VERT &vtx1 = memo.morphBody->GetConstVertex (edge.vert1);
            const VERT &vtx2 = memo.morphBody->GetConstVertex (edge.vert2);
            double x1 = vtx1.x;
            double x2 = vtx2.x;
            double y1 = vtx1.y;
            double y2 = vtx2.y;
            double z1 = vtx1.z;
            double z2 = vtx2.z;
            double dx = (x2 - x1) * (x2 - x1);
            double dy = (y2 - y1) * (y2 - y1);
            double dz = (z2 - z1) * (z2 - z1);
            double dl = DoubleM2IntMM (sqrt (dx + dy + dz)) / 1000.0;
            double dlx = DoubleM2IntMM (sqrt (dy + dx)) / 1000.0;
            double dly = DoubleM2IntMM (sqrt (dx + dz)) / 1000.0;
            double dlz = DoubleM2IntMM (sqrt (dx + dy)) / 1000.0;
            L = L + dl;
            Lx = Lx + dlx;
            Ly = Ly + dly;
            Lz = Lz + dlz;
            Max_x = fmax (Max_x, x1);
            Max_x = fmax (Max_x, x2);
            Max_y = fmax (Max_y, y1);
            Max_y = fmax (Max_y, y2);
            Max_z = fmax (Max_z, z1);
            Max_z = fmax (Max_z, z2);
            Min_x = fmin (Min_x, x1);
            Min_x = fmin (Min_x, x2);
            Min_y = fmin (Min_y, y1);
            Min_y = fmin (Min_y, y2);
            Min_z = fmin (Min_z, z1);
            Min_z = fmin (Min_z, z2);
        }
        Max_x = DoubleM2IntMM (Max_x) / 1000.0;
        Max_y = DoubleM2IntMM (Max_y) / 1000.0;
        Max_z = DoubleM2IntMM (Max_z) / 1000.0;
        Min_x = DoubleM2IntMM (Min_x) / 1000.0;
        Min_y = DoubleM2IntMM (Min_y) / 1000.0;
        Min_z = DoubleM2IntMM (Min_z) / 1000.0;
        A = Max_x - Min_x;
        B = Max_y - Min_y;
        ZZYZX = Max_z - Min_z;
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "l", L, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "lx", Lx, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "ly", Ly, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "lz", Lz, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "max_x", Max_x, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "min_x", Min_x, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "max_y", Max_y, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "min_y", Min_y, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "max_z", Max_z, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "min_z", Min_z, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "a", A, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "b", B, true);
        ParamHelpers::AddLengthValueToParamDictValue (pdictvaluemorph, guid, MORPHNAMEPREFIX, "zzyzx", ZZYZX, true);
        ACAPI_DisposeElemMemoHdls (&memo);
        return true;
    } else {
        ACAPI_DisposeElemMemoHdls (&memo);
        return false;
    }
}

GS::UniString ParamHelpers::GetRawnamePrefixByTypeInx (const short &inx) {
    switch (inx) {
    case PROPERTYTYPEINX:
        return PROPERTYNAMEPREFIX;
        break;
    case GDLTYPEINX:
        return GDLNAMEPREFIX;
        break;
    case GDLDESCTYPEINX:
        return GDLDESCNAMEPREFIX;
        break;
    case COORDTYPEINX:
        return COORDNAMEPREFIX;
        break;
    case GLOBTYPEINX:
        return GLOBNAMEPREFIX;
        break;
    case INFOTYPEINX:
        return INFONAMEPREFIX;
        break;
    case MEPTYPEINX:
        return MEPNAMEPREFIX;
        break;
    case FORMULATYPEINX:
        return FORMULANAMEPREFIX;
        break;
    case IDTYPEINX:
        return IDNAMEPREFIX;
        break;
    case IFCTYPEINX:
        return IFCNAMEPREFIX;
        break;
    case MORPHTYPEINX:
        return MORPHNAMEPREFIX;
        break;
    case ATTRIBTYPEINX:
        return ATTRIBNAMEPREFIX;
        break;
    case LISTDATATYPEINX:
        return LISTDATANAMEPREFIX;
        break;
    case MATERIALTYPEINX:
        return MATERIALNAMEPREFIX;
        break;
    case CLASSTYPEINX:
        return CLASSNAMEPREFIX;
        break;
    case ELEMENTTYPEINX:
        return ELEMENTNAMEPREFIX;
        break;
    case FILETYPEINX:
        return FILENAMEPREFIX;
        break;
    default:
#if defined(TESTING)
        DBprnt (inx, "ERROR GetRawnamePrefixByTypeInx");
#endif
        return EMPTYSTRING;
        break;
    }
#if defined(TESTING)
    DBprnt (inx, "ERROR GetRawnamePrefixByTypeInx");
#endif
    return EMPTYSTRING;
}

short ParamHelpers::GetTypeInxByRawnamePrefix (const GS::UniString &rawName) {
    if (rawName.BeginsWith (PROPERTYNAMEPREFIX))
        return PROPERTYTYPEINX;
    if (rawName.BeginsWith (GDLNAMEPREFIX))
        return GDLTYPEINX;
    if (rawName.BeginsWith (GDLDESCNAMEPREFIX))
        return GDLDESCTYPEINX;
    if (rawName.BeginsWith (COORDNAMEPREFIX))
        return COORDTYPEINX;
    if (rawName.BeginsWith (GLOBNAMEPREFIX))
        return GLOBTYPEINX;
    if (rawName.BeginsWith (INFONAMEPREFIX))
        return INFOTYPEINX;
    if (rawName.BeginsWith (MEPNAMEPREFIX))
        return MEPTYPEINX;
    if (rawName.BeginsWith (FORMULANAMEPREFIX))
        return FORMULATYPEINX;
    if (rawName.BeginsWith (IDNAMEPREFIX))
        return IDTYPEINX;
    if (rawName.BeginsWith (IFCNAMEPREFIX))
        return IFCTYPEINX;
    if (rawName.BeginsWith (MORPHNAMEPREFIX))
        return MORPHTYPEINX;
    if (rawName.BeginsWith (ATTRIBNAMEPREFIX))
        return ATTRIBTYPEINX;
    if (rawName.BeginsWith (LISTDATANAMEPREFIX))
        return LISTDATATYPEINX;
    if (rawName.BeginsWith (MATERIALNAMEPREFIX))
        return MATERIALTYPEINX;
    if (rawName.BeginsWith (CLASSNAMEPREFIX))
        return CLASSTYPEINX;
    if (rawName.BeginsWith (ELEMENTNAMEPREFIX))
        return ELEMENTTYPEINX;
    if (rawName.BeginsWith (FILENAMEPREFIX))
        return FILETYPEINX;
#if defined(TESTING)
    DBprnt ("ERROR GetTypeInxByRawnamePrefix " + rawName);
#endif
    return 0;
}

// -----------------------------------------------------------------------------
// Назначает флаги источника чтения по rawName параметра
// -----------------------------------------------------------------------------
void ParamHelpers::SetParamValueSourseByName (ParamValue &pvalue) {
    if (pvalue.fromCoord || pvalue.fromGDLparam || pvalue.fromGDLdescription || pvalue.fromProperty ||
        pvalue.fromMaterial || pvalue.fromInfo || pvalue.fromIFCProperty || pvalue.fromMorph ||
        pvalue.fromPropertyDefinition || pvalue.fromClassification || pvalue.fromGDLArray ||
        pvalue.fromAttribDefinition || pvalue.fromAttribElement || pvalue.fromListData || pvalue.fromElement ||
        pvalue.fromMEP || pvalue.fromID)
        return;
    short inx = ParamHelpers::GetTypeInxByRawnamePrefix (pvalue.rawName);
    pvalue.typeinx = inx;
    switch (inx) {
    case PROPERTYTYPEINX:
        pvalue.fromPropertyDefinition = true;
        if (pvalue.rawName.BeginsWith ("{@property:buildingmaterialproperties/"))
            pvalue.fromAttribDefinition = true;
        break;
    case GDLTYPEINX:
        pvalue.fromGDLparam = true;
        break;
    case GDLDESCTYPEINX:
        pvalue.fromGDLparam = true;
        pvalue.fromGDLdescription = true;
        break;
    case COORDTYPEINX:
        pvalue.fromCoord = true;
        break;
    case GLOBTYPEINX:
        pvalue.fromGlob = true;
        break;
    case INFOTYPEINX:
        pvalue.fromInfo = true;
        break;
    case MEPTYPEINX:
        pvalue.fromMEP = true;
        break;
    case FORMULATYPEINX:
        pvalue.val.hasFormula = true;
        break;
    case IDTYPEINX:
        pvalue.fromID = true;
        break;
    case IFCTYPEINX:
        pvalue.fromIFCProperty = true;
        break;
    case MORPHTYPEINX:
        pvalue.fromMorph = true;
        break;
    case ATTRIBTYPEINX:
        pvalue.fromAttribElement = true;
        break;
    case LISTDATATYPEINX:
        pvalue.fromListData = true;
        break;
    case MATERIALTYPEINX:
        pvalue.fromMaterial = true;
        break;
    case CLASSTYPEINX:
        pvalue.fromClassification = true;
        break;
    case ELEMENTTYPEINX:
        pvalue.fromElement = true;
        break;
    case FILETYPEINX:
        pvalue.fromFile = true;
        break;
    default:
        break;
    }
    if (pvalue.fromGDLparam || pvalue.fromGDLdescription || pvalue.fromListData)
        ParamHelpers::SetArrayByRawname (pvalue);
    if (pvalue.fromGDLparam && (!pvalue.rawName_row_start.IsEmpty () || !pvalue.rawName_row_end.IsEmpty () ||
                                !pvalue.rawName_col_start.IsEmpty () || !pvalue.rawName_col_end.IsEmpty ()))
        pvalue.needPreRead = true;
}

void ParamHelpers::SetArrayByRawname (ParamValue &pvalue) {
    if (!pvalue.rawName.Contains ("@arr_"))
        return;
    GS::Array<GS::UniString> partstring;
    UInt32 n = StringSpltFilter (pvalue.rawName, "@arr_", partstring, BRACEEND);
    if (n == 0)
        return;
    GS::UniString arr = partstring.Get (0);
    arr.ReplaceAll (BRACEEND, EMPTYSTRING);
    partstring.Clear ();
    n = StringSplt (arr, "_", partstring, true);
    if (n == 0)
        return;
    for (UInt32 i = 0; i < n; i++) {
        double inx = 0;
        if (UniStringToDouble (partstring[i], inx)) {
            if (inx > 0.01) {
                switch (i) {
                case 0:
                    pvalue.val.array_row_start = (int)inx;
                    break;
                case 1:
                    pvalue.val.array_row_end = (int)inx;
                    break;
                case 2:
                    pvalue.val.array_column_start = (int)inx;
                    break;
                case 3:
                    pvalue.val.array_column_end = (int)inx;
                    break;
                case 4:
                    pvalue.val.array_format_out = (int)inx;
                    break;
                default:
                    break;
                }
            }
        }
    }
    pvalue.fromGDLArray = true;
}

GS::UniString ParamHelpers::NameToRawName (const GS::UniString &name, FormatString &formatstring) {
    if (name.IsEmpty ())
        return EMPTYSTRING;
    GS::UniString rawname_prefix = "";
    GS::UniString name_ = name.ToLowerCase ();
    if (name_.Contains (BRACESTART) && name_.Contains (BRACESTART))
        name_ = name_.GetSubstring (CHARBRACESTART, CHARBRACEEND, 0);
    // Ищём строку с указанием формата вывода (метры/миллиметры)
    GS::UniString stringformat = FormatStringFunc::GetFormatString (name_);
    formatstring = FormatStringFunc::ParseFormatString (stringformat);
    name_ = GetPropertyENGName (name_).ToLowerCase ();

    // Проверяем - есть ли указатель на тип параметра (GDL, Property, IFC)
    if (name_.Contains (":")) {
        GS::Array<GS::UniString> partstring;
        UInt32 n = StringSplt (name_, ":", partstring, true);
        if (n > 1) {
            rawname_prefix = partstring[0] + ":";
            name_ = partstring[1].ToLowerCase ();
        }
    }
    if (rawname_prefix.IsEmpty () && name_.IsEqual ("id"))
        rawname_prefix = "@id:";
    if (rawname_prefix.IsEmpty ())
        rawname_prefix = "@gdl:";
    if (!rawname_prefix.Contains (ATSIGN)) {
        rawname_prefix = ATSIGN + rawname_prefix;
    }
    name_.ReplaceAll (SLASHEKR, SLASH);
    GS::UniString rawName = BRACESTART + rawname_prefix + name_ + BRACEEND;
    return rawName;
}

// -----------------------------------------------------------------------------
// Добавление пустого значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddValueToParamDictValue (ParamDictValue &params, const GS::UniString &name) {
    if (name.IsEmpty ())
        return;
    FormatString formatstring = {};
    GS::UniString rawName = ParamHelpers::NameToRawName (name, formatstring);
    if (params.ContainsKey (rawName))
        return;
    ParamValue pvalue = {};
    GS::UniString name_ = name.ToLowerCase ();
    pvalue.rawName = rawName;
    pvalue.name = name_;
    pvalue.val.formatstring = formatstring;
    ParamHelpers::SetParamValueSourseByName (pvalue);
    params.Put (rawName, std::move (pvalue));
}

// -----------------------------------------------------------------------------
// Проверяет необходимость добавления в словарь параметров
// Если в имени параметра содержится информация о номере аттрибута и имя такого параметра есть в словаре - вернёт истину
// -----------------------------------------------------------------------------
bool ParamHelpers::needAdd (const ParamDictValue &params, const GS::UniString &rawName) {
    bool addNew = false;
    if (rawName.Contains (CharENTER)) {
        UInt32 n = rawName.FindFirst (CharENTER);
        GS::UniString rawName_ = rawName.GetSubstring (0, n) + BRACEEND;
        addNew = params.ContainsKey (rawName_);
    }
    return addNew;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь ParamDict, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDict (const API_Guid &elemGuid, ParamValue &param, ParamDictValue &paramToRead) {
    GS::UniString &rawName = param.rawName;
    if (!paramToRead.ContainsKey (rawName)) {
        if (param.fromGuid == APINULLGuid)
            param.fromGuid = elemGuid;
        paramToRead.Put (rawName, param);
    }
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement (const ParamValue &param, ParamDictElement &paramToRead) {
    ParamHelpers::AddParamValue2ParamDictElement (param.fromGuid, param, paramToRead);
}

// --------------------------------------------------------------------
// Содержит ли значения элемент из списка игнорируемых
// --------------------------------------------------------------------
bool ParamHelpers::CheckIgnoreVal (const SkipValues &ignorevals, const ParamValue &param) {
    GS::UniString emp = param.val.uniStringValue;
    emp.Trim ();
    if (param.val.type != API_PropertyBooleanValueType) {
        if (param.val.type == API_PropertyStringValueType) {
            if (ignorevals.skip_empty && param.val.uniStringValue.IsEmpty ())
                return true;
            if (ignorevals.skip_trim_empty && emp.IsEmpty ())
                return true;
        } else {
            if ((ignorevals.skip_empty || ignorevals.skip_trim_empty) && is_equal (param.val.doubleValue, 0))
                return true;
        }
    }
    if (ignorevals.ignorevals.IsEmpty ())
        return false;

    for (const auto &ign : ignorevals.ignorevals) {
        if (emp.IsEqual (ign))
            return true;
        if (ign.BeginsWith ("*")) {
            GS::UniString ig = ign;
            ig.ReplaceAll ("*", EMPTYSTRING);
            if (emp.EndsWith (ig))
                return true;
            continue;
        }
        if (ign.EndsWith ("*")) {
            GS::UniString ig = ign;
            ig.ReplaceAll ("*", EMPTYSTRING);
            if (emp.BeginsWith (ig))
                return true;
            continue;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Сопоставляет параметры
//
// is_ignore - параметр содержит игнорируемое значение
// is_eq - параметры сопоставимы и равны, оба валидны
// --------------------------------------------------------------------
bool ParamHelpers::CompareParamValue (ParamValue &paramFrom,
                                      ParamValue &paramTo,
                                      FormatString stringformat,
                                      const SkipValues &ignorevals,
                                      bool &is_ignore,
                                      bool &is_eq) {
#ifdef TESTING
    if (!paramTo.isValid) {
        if (!paramTo.fromProperty && !paramTo.fromPropertyDefinition)
            DBtest (paramTo.isValid, paramTo.rawName + " paramTo.isValid");
    } else {
        if (!paramTo.val.hasrawDouble && paramTo.val.type != API_PropertyStringValueType &&
            !paramFrom.fromClassification)
            DBtest (paramTo.val.hasrawDouble, paramTo.rawName + " CompareParamValue::paramTo.val.hasrawDouble");
    }
    if (!paramFrom.isValid) {
        DBtest (paramFrom.isValid, paramFrom.rawName + " paramFrom.isValid");
    } else {
        if (!paramFrom.val.hasrawDouble && paramFrom.val.type != API_PropertyStringValueType)
            DBtest (paramFrom.val.hasrawDouble, paramFrom.rawName + " CompareParamValue::paramFrom.val.hasrawDouble");
    }
    if (paramFrom.fromClassification && paramTo.fromClassification && paramFrom.val.guidval != APINULLGuid) {
        DBtest (paramFrom.val.guidval == APINULLGuid,
                paramFrom.rawName + " paramTo.fromClassification && paramFrom.val.guidval == APINULLGuid");
    }
#endif
    if (!paramFrom.isValid) {
        if (ignorevals.reset_to_def)
            is_ignore = true;
        return false;
    }
    if (paramFrom.fromClassification && paramTo.fromClassification && paramFrom.val.guidval == APINULLGuid) {
        return false;
    }
    if (paramFrom.fromClassification && paramTo.fromClassification) {
        if (paramFrom.val.guidval != paramTo.val.guidval) {
            paramTo.val = paramFrom.val;
            paramTo.isValid = true;
            return true;
        } else {
            return false;
        }
    }
    if (paramTo.isValid || paramTo.fromProperty || paramTo.fromPropertyDefinition) {
        if (stringformat.isEmpty)
            stringformat = paramTo.val.formatstring;
        if (stringformat.isEmpty)
            stringformat = paramFrom.val.formatstring;
        // Приводим к единому виду перед проверкой
        if (!stringformat.isEmpty) {
            // Если в правиле задана более высокая точность - нужно использовать значение до предыдущего округления.
            if (paramTo.val.formatstring.n_zero < stringformat.n_zero ||
                (!stringformat.needRound && paramTo.val.formatstring.needRound) ||
                paramTo.val.formatstring.forceRaw != stringformat.forceRaw) {
                paramTo.val.doubleValue = paramTo.val.rawDoubleValue;
            }
            if (paramFrom.val.formatstring.n_zero < stringformat.n_zero ||
                (!stringformat.needRound && paramFrom.val.formatstring.needRound) ||
                paramFrom.val.formatstring.forceRaw != stringformat.forceRaw) {
                paramFrom.val.doubleValue = paramFrom.val.rawDoubleValue;
            }
            paramTo.val.formatstring = stringformat;
            paramFrom.val.formatstring = stringformat;
            ParamHelpers::ConvertByFormatString (paramTo);
            ParamHelpers::ConvertByFormatString (paramFrom);
        }
        // Проверяем игнорируемый список
        if (ParamHelpers::CheckIgnoreVal (ignorevals, paramFrom)) {
            is_ignore = true;
            return false;
        }
        // Сопоставляем и записываем, если значения отличаются
        if (paramFrom != paramTo) {
            paramTo.val = paramFrom.val; // Записываем только значения
            paramTo.isValid = true;
            return true;
        } else {
            if (paramTo.isValid)
                is_eq = true;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Запись параметра ParamValue в словарь элементов ParamDictElement, если его там прежде не было
// --------------------------------------------------------------------
void ParamHelpers::AddParamValue2ParamDictElement (const API_Guid &elemGuid,
                                                   const ParamValue &param,
                                                   ParamDictElement &paramToRead) {
    const GS::UniString &rawName = param.rawName;
#if defined(TESTING)
    if (elemGuid == APINULLGuid) {
        DBprnt ("AddParamValue2ParamDictElement err", "elemGuid == APINULLGuid");
    }
#endif
    ParamDictValue *pPtr = paramToRead.GetPtr (elemGuid);
    if (pPtr != nullptr) {
        pPtr->Put (rawName, param);
    } else {
        ParamDictValue newParams;
        newParams.Put (rawName, param);
        paramToRead.Put (elemGuid, std::move (newParams));
    }
}

// --------------------------------------------------------------------
// Запись словаря ParamDictValue в словарь элементов ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::AddParamDictValue2ParamDictElement (const API_Guid &elemGuid,
                                                       ParamDictValue &param,
                                                       ParamDictElement &paramToRead) {
    ParamDictValue *pPtr = paramToRead.GetPtr (elemGuid);
    if (pPtr == nullptr) {
        paramToRead.Put (elemGuid, param);
        return;
    }
    for (auto cIt = param.EnumeratePairs (); cIt != nullptr; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        const GS::UniString &rawName = cIt->key;
#else
        const GS::UniString &rawName = *cIt->key;
#endif
        if (pPtr->ContainsKey (rawName))
            continue;
#if defined(AC_28) || defined(AC_29)
        const ParamValue &sourceParam = cIt->value;
#else
        const ParamValue &sourceParam = *cIt->value;
#endif
        ParamValue paramToCopy = sourceParam;
        if (paramToCopy.fromGuid == APINULLGuid) {
            paramToCopy.fromGuid = elemGuid;
#if defined(TESTING)
        } else {
            if (paramToCopy.fromGuid != elemGuid) {
                DBprnt ("err AddParamDictValue2ParamDictElement - different GUID ", rawName);
            }
#endif
        }
        pPtr->Put (rawName, paramToCopy);
    }
}

// -----------------------------------------------------------------------------
// Добавление массива свойств в словарь
// -----------------------------------------------------------------------------
bool ParamHelpers::AddProperty (ParamDictValue &params, GS::Array<API_Property> &properties, const API_Guid &elemguid) {
    bool flag_find = false;
    for (API_Property &property : properties) {
        ParamValue pvalue = {};
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
        const GS::UniString &rawName = pvalue.rawName;
        ParamValue *existingValue = params.GetPtr (rawName);
        bool hasname = (existingValue != nullptr);
        bool needAdd = ParamHelpers::needAdd (params, rawName);
        if (!hasname && !needAdd) {
#if defined(TESTING)
            DBprnt ("AddProperty err not added", rawName);
#endif
            continue;
        }
        if (hasname) {
            // Заполняем stringformat
            FormatString fstring = {};
            fstring = existingValue->val.formatstring;
            if (!fstring.isEmpty)
                pvalue.val.formatstring = fstring;
        }
        if (!ParamHelpers::ConvertToParamValue (pvalue, property)) {
#if defined(TESTING)
            DBprnt ("AddProperty err convert to pvalue", rawName);
#endif
            continue;
        }
        if (hasname) {
            if (pvalue.fromClassification && !pvalue.val.uniStringValue.IsEmpty () &&
                ClassificationFunc::ReadSystemDict ()) {
                GS::UniString systemname = pvalue.val.uniStringValue.ToLowerCase ();
                API_Guid classguid = ClassificationFunc::FindClass (pvalue.name, systemname);
                if (classguid != APINULLGuid && pvalue.val.guidval == APINULLGuid) {
                    pvalue.val.guidval = classguid;
                } else {
#if defined(TESTING)
                    if (pvalue.val.guidval != APINULLGuid)
                        DBprnt ("ERROR Compare classification - double class");
                    if (classguid == APINULLGuid)
                        DBprnt ("ERROR Compare classification - empty class");
#endif
                }
            }
            if (pvalue.fromGuid == APINULLGuid) {
                if (elemguid == APINULLGuid) {
                    pvalue.fromGuid = elemguid;
                } else {
                    pvalue.fromGuid = existingValue->fromGuid;
                }
            }
            pvalue.toQRCode = existingValue->toQRCode;
            params.Put (rawName, std::move (pvalue));
            flag_find = true;
        } else {
            if (needAdd) {
                if (elemguid != APINULLGuid)
                    pvalue.fromGuid = elemguid;
                params.Put (rawName, std::move (pvalue));
                flag_find = true;
            }
        }
    }
    return flag_find;
}

template <typename Converter>
void AddValueToParamDictValueImpl (ParamDictValue &params,
                                   const GS::UniString &rawName_prefix,
                                   const GS::UniString &name,
                                   const bool addInNotEx,
                                   const bool isLen,
                                   Converter &&convertValue) {
    GS::UniString rawName = rawName_prefix;
    rawName += name;
    rawName += BRACEEND;
    rawName.SetToLowerCase ();
    ParamValue *existingParam = params.GetPtr (rawName);
    if (existingParam != nullptr) {
        if (!addInNotEx) {
#if defined(TESTING)
            DBprnt ("AddToParamDictValue err", rawName + " ContainsKey");
#endif
            return;
        }
        ParamValue pvalue = {};
        pvalue.rawName = std::move (rawName);
        pvalue.name = name;
        ParamHelpers::SetParamValueSourseByName (pvalue);
        if (isLen)
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
        convertValue (pvalue); // Вызываем кастомную конвертацию через лямбду
        pvalue.fromGuid = existingParam->fromGuid;
        pvalue.toQRCode = existingParam->toQRCode;
        *existingParam = std::move (pvalue);
    } else {
        if (!addInNotEx) {
            ParamValue pvalue = {};
            pvalue.rawName = rawName;
            pvalue.name = name;
            ParamHelpers::SetParamValueSourseByName (pvalue);
            if (isLen)
                pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
            convertValue (pvalue);
            params.Add (std::move (rawName), std::move (pvalue));
        }
    }
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddBoolValueToParamDictValue (ParamDictValue &params,
                                                 const API_Guid &elemGuid,
                                                 const GS::UniString &rawName_prefix,
                                                 const GS::UniString &name,
                                                 const bool val,
                                                 const bool addInNotEx) {
    AddValueToParamDictValueImpl (params, rawName_prefix, name, addInNotEx, false, [val] (ParamValue &pvalue) {
        ParamHelpers::ConvertBoolToParamValue (pvalue, EMPTYSTRING, val);
    });
}

// -----------------------------------------------------------------------------
// Добавление значения длины в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddLengthValueToParamDictValue (ParamDictValue &params,
                                                   const API_Guid &elemGuid,
                                                   const GS::UniString &rawName_prefix,
                                                   const GS::UniString &name,
                                                   const double val,
                                                   const bool addInNotEx) {
    AddValueToParamDictValueImpl (params, rawName_prefix, name, addInNotEx, true, [val] (ParamValue &pvalue) {
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, val);
    });
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// rawName_prefix без скобок
// -----------------------------------------------------------------------------
void ParamHelpers::AddDoubleValueToParamDictValue (ParamDictValue &params,
                                                   const API_Guid &elemGuid,
                                                   const GS::UniString &rawName_prefix,
                                                   const GS::UniString &name,
                                                   const double val,
                                                   const bool addInNotEx) {
    AddValueToParamDictValueImpl (params, rawName_prefix, name, addInNotEx, false, [val] (ParamValue &pvalue) {
        ParamHelpers::ConvertDoubleToParamValue (pvalue, EMPTYSTRING, val);
    });
}

// -----------------------------------------------------------------------------
// Добавление значения в словарь ParamDictValue
// -----------------------------------------------------------------------------
void ParamHelpers::AddStringValueToParamDictValue (ParamDictValue &params,
                                                   const API_Guid &elemGuid,
                                                   const GS::UniString &rawName_prefix,
                                                   const GS::UniString &name,
                                                   const GS::UniString val,
                                                   const bool addInNotEx) {
    AddValueToParamDictValueImpl (params, rawName_prefix, name, addInNotEx, false, [val] (ParamValue &pvalue) {
        ParamHelpers::ConvertStringToParamValue (pvalue, EMPTYSTRING, val);
    });
}

// -----------------------------------------------------------------------------
// Получение координат объекта
// symb_pos_x , symb_pos_y, symb_pos_z
// Для панелей навесной стены возвращает центр панели
// Для колонны или объекта - центр колонны и отм. низа
// Для зоны - центр зоны (без отметки, symb_pos_z = 0)
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadCoords (const API_Element &element, ParamDictValue &pdictvaluecoord) {
#if defined(TESTING)
    DBprnt ("      ReadCoords");
#endif
    bool isFliped = false;
    double x = 0;
    double y = 0;
    double z = 0;
    double angz = 0;
    double sx = 0;
    double sy = 0;
    double ww = 0;
    double hw = 0; // Размеры проёма
    double ex = 0;
    double ey = 0;
    double dx = 0;
    double dy = 0;
    // Координаты относительно пользовательского начала
    double xu = 0;
    double yu = 0;
    double zu = 0;
    double sxu = 0;
    double syu = 0;
    double exu = 0;
    double eyu = 0;
    // Координаты относительно точки привязки проекта
    double xsp = 0;
    double ysp = 0;
    double zsp = 0;
    double sxsp = 0;
    double sysp = 0;
    double exsp = 0;
    double eysp = 0;

    double zw = 0; // Высота стены, в которой размещён проём
    double lox = 0;
    double loy = 0;
    double loz = 0;
    double offx = 0;
    double offy = 0;
    ParamValue cacheparam = {};
    GS::UniString locorig = "{@glob:locorigin_x}";
    if (GetParamValueFromCache (locorig, cacheparam))
        lox = cacheparam.val.rawDoubleValue;
    locorig = "{@glob:locorigin_y}";
    if (GetParamValueFromCache (locorig, cacheparam))
        loy = cacheparam.val.rawDoubleValue;
    locorig = "{@glob:locorigin_z}";
    if (GetParamValueFromCache (locorig, cacheparam))
        loz = cacheparam.val.rawDoubleValue;
    locorig = "{@glob:offsetorigin_x}";
    if (GetParamValueFromCache (locorig, cacheparam))
        offx = cacheparam.val.rawDoubleValue;
    locorig = "{@glob:offsetorigin_y}";
    if (GetParamValueFromCache (locorig, cacheparam))
        offy = cacheparam.val.rawDoubleValue;
    if (is_equal (offx, 0) && is_equal (offy, 0)) {
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "has_distant_element", false, true);
    } else {
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "has_distant_element", true, true);
    }
    double tolerance_coord = 0.001;
    double tolerance_coord_hard = 0.000001;
    double tolerance_ang = 0.00001;
    bool hasSymbpos = false;
    bool hasLine = false;
    double angznorth = -1.0;
    double north = -1.0;
    bool skip_north = false;

    double symb_rotangle_fraction = 0.0;
    bool bsymb_rotangle_correct = false;
    bool bsymb_rotangle_correct_1000 = false;

    double slantDirectionAngle = 0;
    double axisRotationAngle = 0;
    GS::UniString angznorthtxt = "UNDEF";
    GS::UniString angznorthtxteng = "UNDEF";
    GS::UniString globnorthkey = "{@glob:glob_north_dir}";
    if (!GetParamValueFromCache (globnorthkey, cacheparam)) {
        skip_north = true;
    } else {
        north = cacheparam.val.doubleValue;
    }
    bool bsync_coord_correct = true;
    const GS::UniString sync_coord_correctkey = "{@property:sync_correct_flag}";
    if (auto *flag = pdictvaluecoord.GetPtr (sync_coord_correctkey)) {
        if (flag->isValid)
            bsync_coord_correct = flag->val.boolValue;
    }
    API_ElemTypeID eltype = GetElemTypeID (element);
    API_Element owner = {};
    auto &cache = PROPERTYCACHE ();
    // Обработка навесной стены- случай особый, т.к. у неё может быть несколько сегментов
    if (eltype == API_CurtainWallID && element.header.hasMemo) {
        double aang = fabs (fmod (element.curtainWall.angle, 180.0));
        if (aang > 90.0)
            aang = 180.0 - aang;
        if (aang < 5.0)
            skip_north = true;
        hasLine = true;
        isFliped = element.curtainWall.flipped;
        double sx_ = 0;
        double sy_ = 0;
        double ex_ = 0;
        double ey_ = 0;
        double sxu_ = 0;
        double syu_ = 0;
        double exu_ = 0;
        double eyu_ = 0;
        GS::UniString angznorthtxteng_ = "";
        bool bsymb_pos_sx_correct_ = false;
        bool bsymb_pos_sy_correct_ = false;
        bool bsymb_pos_ex_correct_ = false;
        bool bsymb_pos_ey_correct_ = false;
        bool bsymb_pos_e_correct_ = false;
        bool bsymb_pos_s_correct_ = false;
        bool bsymb_pos_correct_ = false;
        bool bsymb_rotangle_correct_ = false;
        bool bsymb_rotangle_correct_1000_ = false;
        bool bsymb_pos_sx_correctu_ = false;
        bool bsymb_pos_sy_correctu_ = false;
        bool bsymb_pos_ex_correctu_ = false;
        bool bsymb_pos_ey_correctu_ = false;
        bool bsymb_pos_e_correctu_ = false;
        bool bsymb_pos_s_correctu_ = false;
        bool bsymb_pos_correctu_ = false;
        API_ElementMemo memo = {};
        if (ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_CWallSegments) == NoError) {
            Int32 size = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.cWallSegments)) / sizeof (API_CWSegmentType);
            for (Int32 inx_segment = 0; inx_segment < size; ++inx_segment) {
                sx = memo.cWallSegments[inx_segment].begC.x + offx;
                sy = memo.cWallSegments[inx_segment].begC.y + offy;
                ex = memo.cWallSegments[inx_segment].endC.x + offx;
                ey = memo.cWallSegments[inx_segment].endC.y + offy;
                sxu = sx - lox;
                syu = sy - loy;
                exu = ex - lox;
                eyu = ey - loy;
                if (inx_segment == 0) {
                    sx_ = sx;
                    sy_ = sy;
                    ex_ = ex;
                    ey_ = ey;
                    sxu_ = sxu;
                    syu_ = syu;
                    exu_ = exu;
                    eyu_ = eyu;
                } else {
                    ex_ = ex;
                    ey_ = ey;
                    exu_ = exu;
                    eyu_ = eyu;
                }
                CoordRotAngle (sx, sy, ex, ey, isFliped, angz);
                bsymb_rotangle_correct =
                    CoordCorrectAngle (angz, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
                if (!skip_north)
                    CoordNorthAngle (north, angz, angznorth, angznorthtxt, angznorthtxteng);
                if (angznorthtxteng_.IsEmpty () && !angznorthtxteng.IsEmpty ())
                    angznorthtxteng_ = angznorthtxteng;
                if (!angznorthtxteng.IsEqual (angznorthtxteng_) && !angznorthtxteng.IsEmpty ()) {
                    skip_north = true; // Несколько сегментов с разным направлением. Определить север не получится
                }
                bool bsymb_pos_sx_correct = check_accuracy (sx, tolerance_coord);
                bool bsymb_pos_sy_correct = check_accuracy (sy, tolerance_coord);
                bool bsymb_pos_ex_correct = check_accuracy (ex, tolerance_coord);
                bool bsymb_pos_ey_correct = check_accuracy (ey, tolerance_coord);
                bool bsymb_pos_e_correct = bsymb_pos_sx_correct && bsymb_pos_sy_correct;
                bool bsymb_pos_s_correct = bsymb_pos_ex_correct && bsymb_pos_ey_correct;
                bool bsymb_pos_correct = bsymb_pos_e_correct && bsymb_pos_s_correct;

                bool bsymb_pos_sx_correctu = check_accuracy (sxu, tolerance_coord);
                bool bsymb_pos_sy_correctu = check_accuracy (syu, tolerance_coord);
                bool bsymb_pos_ex_correctu = check_accuracy (exu, tolerance_coord);
                bool bsymb_pos_ey_correctu = check_accuracy (eyu, tolerance_coord);
                bool bsymb_pos_e_correctu = bsymb_pos_sx_correctu && bsymb_pos_sy_correctu;
                bool bsymb_pos_s_correctu = bsymb_pos_ex_correctu && bsymb_pos_ey_correctu;
                bool bsymb_pos_correctu = bsymb_pos_e_correctu && bsymb_pos_s_correctu;

                if (bsymb_rotangle_correct)
                    bsymb_rotangle_correct_ = true;
                if (bsymb_rotangle_correct_1000)
                    bsymb_rotangle_correct_1000_ = true;
                if (bsymb_pos_sx_correct)
                    bsymb_pos_sx_correct_ = true;
                if (bsymb_pos_sy_correct)
                    bsymb_pos_sy_correct_ = true;
                if (bsymb_pos_ex_correct)
                    bsymb_pos_ex_correct_ = true;
                if (bsymb_pos_ey_correct)
                    bsymb_pos_ey_correct_ = true;
                if (bsymb_pos_e_correct)
                    bsymb_pos_e_correct_ = true;
                if (bsymb_pos_s_correct)
                    bsymb_pos_s_correct_ = true;
                if (bsymb_pos_correct)
                    bsymb_pos_correct_ = true;

                if (bsymb_pos_sx_correctu)
                    bsymb_pos_sx_correctu_ = true;
                if (bsymb_pos_sy_correctu)
                    bsymb_pos_sy_correctu_ = true;
                if (bsymb_pos_ex_correctu)
                    bsymb_pos_ex_correctu_ = true;
                if (bsymb_pos_ey_correctu)
                    bsymb_pos_ey_correctu_ = true;
                if (bsymb_pos_e_correctu)
                    bsymb_pos_e_correctu_ = true;
                if (bsymb_pos_s_correctu)
                    bsymb_pos_s_correctu_ = true;
                if (bsymb_pos_correctu)
                    bsymb_pos_correctu_ = true;
            }
        }
        ACAPI_DisposeElemMemoHdls (&memo);
        if (skip_north) {
            angznorth = -1.0;
            angznorthtxt = "UNDEF";
            angznorthtxteng = "UNDEF";
        }
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir", angznorth, true);
        ParamHelpers::AddStringValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir_str", angznorthtxt, true);
        ParamHelpers::AddStringValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir_eng", angznorthtxteng, true);

        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x", sx_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y", sy_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx", sx_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy", sy_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ex", ex_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ey", ey_, true);

        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x", sxu_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y", syu_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx", sxu_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy", syu_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ex", exu_, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ey", eyu_, true);

        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle", angz, true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod5", fmod (angz, 5.0), true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod10", fmod (angz, 10.0), true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod45", fmod (angz, 45.0), true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod90", fmod (angz, 90.0), true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod180", fmod (angz, 180.0), true);
        if (bsync_coord_correct) {
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_s_correct",
                                                        bsymb_pos_s_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_e_correct",
                                                        bsymb_pos_e_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_x_correct",
                                                        bsymb_pos_sx_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_y_correct",
                                                        bsymb_pos_sy_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sx_correct",
                                                        bsymb_pos_sx_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sy_correct",
                                                        bsymb_pos_sy_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ex_correct",
                                                        bsymb_pos_ex_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ey_correct",
                                                        bsymb_pos_ey_correctu_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_correct",
                                                        bsymb_pos_correctu_,
                                                        true);

            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_s_correct",
                                                        bsymb_pos_s_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_e_correct",
                                                        bsymb_pos_e_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_x_correct",
                                                        bsymb_pos_sx_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_y_correct",
                                                        bsymb_pos_sy_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sx_correct",
                                                        bsymb_pos_sx_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sy_correct",
                                                        bsymb_pos_sy_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ex_correct",
                                                        bsymb_pos_ex_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ey_correct",
                                                        bsymb_pos_ey_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", bsymb_pos_correct_, true);
            ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord,
                                                          element.header.guid,
                                                          COORDNAMEPREFIX,
                                                          "symb_rotangle_fraction",
                                                          symb_rotangle_fraction,
                                                          true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_rotangle_correct",
                                                        bsymb_rotangle_correct_,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_rotangle_correct_1000",
                                                        bsymb_rotangle_correct_1000_,
                                                        true);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_e_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ex_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ey_correct", true, true);

            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_e_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ex_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ey_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct", true, true);

            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", true, true);
            ParamHelpers::AddDoubleValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_fraction", 0.0, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_correct_1000", true, true);
        }
        return true;
    }

    // Если нужно определить направление окон или дверей - запрашиваем родительский элемент
    if (eltype == API_WindowID || eltype == API_DoorID || (!skip_north && eltype == API_CurtainWallPanelID)) {
        BNZeroMemory (&owner, sizeof (API_Element));
        if (eltype == API_CurtainWallPanelID)
            owner.header.guid = element.cwPanel.owner;
        if (eltype == API_WindowID)
            owner.header.guid = element.window.owner;
        if (eltype == API_DoorID)
            owner.header.guid = element.door.owner;
        if (ACAPI_Element_Get (&owner) != NoError)
            return false;
        API_ElemTypeID ownereltype = GetElemTypeID (owner);
        if (ownereltype == API_WallID) {
            sx = owner.wall.begC.x + offx;
            sy = owner.wall.begC.y + offy;
            ex = owner.wall.endC.x + offx;
            ey = owner.wall.endC.y + offy;
            zw = owner.wall.height;
            isFliped = owner.wall.flipped;
            hasLine = true;
        }
#ifndef AC_22
        if (eltype == API_CurtainWallPanelID && ownereltype == API_CurtainWallID && owner.header.hasMemo) {
            double aang = fabs (fmod (owner.curtainWall.angle, 180.0));
            if (aang > 90.0)
                aang = 180.0 - aang;
            if (aang < 5.0)
                skip_north = true;
            Int32 inx_segment = element.cwPanel.segmentID;
            API_ElementMemo memo = {};
            if (ACAPI_Element_GetMemo (owner.header.guid, &memo, APIMemoMask_CWallSegments) == NoError) {
                Int32 size = BMGetPtrSize (reinterpret_cast<GSPtr> (memo.cWallSegments)) / sizeof (API_CWSegmentType);
                if (size >= inx_segment) {
                    sx = memo.cWallSegments[inx_segment].begC.x + offx;
                    sy = memo.cWallSegments[inx_segment].begC.y + offy;
                    ex = memo.cWallSegments[inx_segment].endC.x + offx;
                    ey = memo.cWallSegments[inx_segment].endC.y + offy;
                    hasLine = true;
                    isFliped = owner.curtainWall.flipped;
                }
            }
            ACAPI_DisposeElemMemoHdls (&memo);
        }
#endif
    } else {
        UNUSED_VARIABLE (owner);
    }
    switch (eltype) {
    case API_WindowID:
        x = element.window.objLoc;
        y = 0;
        z = element.window.lower;
        ww = element.window.openingBase.width;
        hw = element.window.openingBase.height;
        hasSymbpos = true;
        break;
    case API_DoorID:
        x = element.door.objLoc;
        y = 0;
        z = element.door.lower;
        ww = element.window.openingBase.width;
        hw = element.window.openingBase.height;
        hasSymbpos = true;
        break;
    case API_CurtainWallPanelID:
#ifndef AC_22
        x = element.cwPanel.centroid.x + offx;
        y = element.cwPanel.centroid.y + offy;
        z = element.cwPanel.centroid.z;
        hasSymbpos = true;
#endif
        break;
    case API_ObjectID:
        x = element.object.pos.x + offx;
        y = element.object.pos.y + offy;
        z = element.object.level;
        angz = element.object.angle;
        skip_north = true;
        hasSymbpos = true;
        break;
    case API_ZoneID:
        x = element.zone.pos.x + offx;
        y = element.zone.pos.y + offy;
        skip_north = true;
        hasSymbpos = true;
        break;
    case API_ColumnID:
        x = element.column.origoPos.x + offx;
        y = element.column.origoPos.y + offy;
        slantDirectionAngle = element.column.slantDirectionAngle;
#ifdef AC_22
        axisRotationAngle = element.column.angle;
#else
        axisRotationAngle = element.column.axisRotationAngle;
#endif
        if (slantDirectionAngle < 0)
            slantDirectionAngle = 2 * PI + slantDirectionAngle;
        if (axisRotationAngle < 0)
            axisRotationAngle = 2 * PI + axisRotationAngle;
        angz = slantDirectionAngle + axisRotationAngle;
        hasSymbpos = true;
        break;
    case API_WallID:
        sx = element.wall.begC.x + offx;
        sy = element.wall.begC.y + offy;
        ex = element.wall.endC.x + offx;
        ey = element.wall.endC.y + offy;
        isFliped = element.wall.flipped;
        hasLine = true;
        break;
    case API_BeamID:
        sx = element.beam.begC.x + offx;
        sy = element.beam.begC.y + offy;
        ex = element.beam.endC.x + offx;
        ey = element.beam.endC.y + offy;
        isFliped = element.beam.isFlipped;
        hasLine = true;
        break;
    default:
        x = 0;
        y = 0;
        z = 0;
        sx = 0;
        sy = 0;
        ex = 0;
        ey = 0;
        angz = 0;
        hasLine = true;
        hasSymbpos = true;
        skip_north = true;
        bsync_coord_correct = false;
        break;
    }
    if (eltype == API_ColumnID) {
        const double k = 100000.0; // Коэфф для округления
        if (fabs (slantDirectionAngle) > 0.0000001) {
            slantDirectionAngle = fmod (round ((slantDirectionAngle * 180.0 / PI) * k) / k, 360.0);
        } else {
            slantDirectionAngle = 0.0;
        }
        if (fabs (axisRotationAngle) > 0.0000001) {
            axisRotationAngle = fmod (round ((axisRotationAngle * 180.0 / PI) * k) / k, 360.0);
        } else {
            axisRotationAngle = 0.0;
        }
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_slant", slantDirectionAngle, true);
        bsymb_rotangle_correct =
            CoordCorrectAngle (slantDirectionAngle, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord,
                                                      element.header.guid,
                                                      COORDNAMEPREFIX,
                                                      "symb_rotangle_slant_fraction",
                                                      symb_rotangle_fraction,
                                                      true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_slant_correct",
                                                    bsymb_rotangle_correct,
                                                    true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_slant_correct_1000",
                                                    bsymb_rotangle_correct_1000,
                                                    true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_axis", axisRotationAngle, true);
        bsymb_rotangle_correct =
            CoordCorrectAngle (slantDirectionAngle, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord,
                                                      element.header.guid,
                                                      COORDNAMEPREFIX,
                                                      "symb_rotangle_axis_fraction",
                                                      symb_rotangle_fraction,
                                                      true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_axis_correct",
                                                    bsymb_rotangle_correct,
                                                    true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_axis_correct_1000",
                                                    bsymb_rotangle_correct_1000,
                                                    true);
    } else {
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_slant", 0, true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_slant_fraction", true, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_slant_correct", true, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_slant_correct_1000", true, true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_axis", 0, true);
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_axis_fraction", true, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_axis_correct", true, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_axis_correct_1000", true, true);
    }
    if (hasSymbpos) {
        if (fabs (angz) > 0.0000001) {
            double k = 100000.0;
            angz = fmod (round ((angz * 180.0 / PI) * k) / k, 360.0);
        } else {
            angz = 0.0;
        }
        xu = x - lox;
        yu = y - loy;
        zu = z - loz;
        if (!cache.isSurveyPointTransformationRead)
            cache.ReadSurveyPointTransformation ();
        if (cache.isSurveyPointTransformation_OK) {
            API_Coord3D vtx = {x, y, z};
            API_Coord3D v = GetWordCoord3DTM (vtx, cache.surv_point_tm);
            xsp = v.x;
            ysp = v.y;
            zsp = v.z;
        }
        if (eltype == API_WindowID || eltype == API_DoorID) {
            yu = 0;
            zu = 0;
            if (isFliped) {
                dx = ex - sx;
                dy = ey - sy;
            } else {
                dx = sx - ex;
                dy = sy - ey;
            }
            double l_wall = sqrt (dx * dx + dy * dy);
            double koeff = x / l_wall;
            double swx = sx + (ex - sx) * koeff;
            double swy = sy + (ey - sy) * koeff; // Абсолютные координаты середины проёма
            double swx_lo = swx - lox;
            double swy_lo = swy - loy; // Координаты относительно ПН середины проёма
            double swspx = 0;
            double swspy = 0;
            if (!cache.isSurveyPointTransformationRead)
                cache.ReadSurveyPointTransformation ();
            if (cache.isSurveyPointTransformation_OK) {
                API_Coord3D vtx = {swx, swy, 0};
                API_Coord3D v = GetWordCoord3DTM (vtx, cache.surv_point_tm);
                swspx = v.x;
                swspy = v.y;
            }
            bool windoor_in_wall = true;
            if (x + ww / 2 < tolerance_coord_hard)
                windoor_in_wall = false;
            if (x - ww / 2 - l_wall > tolerance_coord_hard)
                windoor_in_wall = false;
            if (z + hw < tolerance_coord_hard)
                windoor_in_wall = false;
            if (z - zw > tolerance_coord_hard)
                windoor_in_wall = false;
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "windoor_in_wall", windoor_in_wall, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx", swx, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy", swy, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx", swx_lo, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy", swy_lo, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx", swspx, true);
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy", swspy, true);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "windoor_in_wall", true, true);
        }
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x", x, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y", y, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_z", z, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x", xu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y", yu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_z", zu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x", xsp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y", ysp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_z", zsp, true);
        if (bsync_coord_correct) {
            bool bsymb_pos_x_correct = check_accuracy (x, tolerance_coord);
            bool bsymb_pos_y_correct = check_accuracy (y, tolerance_coord);
            bool bsymb_pos_correct = bsymb_pos_x_correct && bsymb_pos_y_correct;
            bool bsymb_pos_x_correct_hard = check_accuracy (x, tolerance_coord_hard);
            bool bsymb_pos_y_correct_hard = check_accuracy (y, tolerance_coord_hard);
            bool bsymb_pos_correct_hard = bsymb_pos_x_correct_hard && bsymb_pos_y_correct_hard;
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct", bsymb_pos_x_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct", bsymb_pos_y_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sx_correct",
                                                        bsymb_pos_x_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sy_correct",
                                                        bsymb_pos_y_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct", bsymb_pos_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", bsymb_pos_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_x_correct_hard",
                                                        bsymb_pos_x_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_y_correct_hard",
                                                        bsymb_pos_y_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sx_correct_hard",
                                                        bsymb_pos_x_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sy_correct_hard",
                                                        bsymb_pos_y_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_s_correct_hard",
                                                        bsymb_pos_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_correct_hard",
                                                        bsymb_pos_correct_hard,
                                                        true);
            bool bsymb_pos_x_correctu = check_accuracy (xu, tolerance_coord);
            bool bsymb_pos_y_correctu = check_accuracy (yu, tolerance_coord);
            bool bsymb_pos_correctu = bsymb_pos_x_correctu && bsymb_pos_y_correctu;
            bool bsymb_pos_x_correct_hardu = check_accuracy (xu, tolerance_coord_hard);
            bool bsymb_pos_y_correct_hardu = check_accuracy (yu, tolerance_coord_hard);
            bool bsymb_pos_correct_hardu = bsymb_pos_x_correct_hardu && bsymb_pos_y_correct_hardu;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_x_correct",
                                                        bsymb_pos_x_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_y_correct",
                                                        bsymb_pos_y_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sx_correct",
                                                        bsymb_pos_x_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sy_correct",
                                                        bsymb_pos_y_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_s_correct",
                                                        bsymb_pos_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct", bsymb_pos_correctu, true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_x_correct_hard",
                                                        bsymb_pos_x_correct_hardu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_y_correct_hard",
                                                        bsymb_pos_y_correct_hardu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sx_correct_hard",
                                                        bsymb_pos_x_correct_hardu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sy_correct_hard",
                                                        bsymb_pos_y_correct_hardu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_s_correct_hard",
                                                        bsymb_pos_correct_hardu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_correct_hard",
                                                        bsymb_pos_correct_hardu,
                                                        true);
            bool bsymb_pos_x_correctsp = check_accuracy (xsp, tolerance_coord);
            bool bsymb_pos_y_correctsp = check_accuracy (ysp, tolerance_coord);
            bool bsymb_pos_correctsp = bsymb_pos_x_correctsp && bsymb_pos_y_correctsp;
            bool bsymb_pos_x_correct_hardusp = check_accuracy (xsp, tolerance_coord_hard);
            bool bsymb_pos_y_correct_hardusp = check_accuracy (ysp, tolerance_coord_hard);
            bool bsymb_pos_correct_hardusp = bsymb_pos_x_correct_hardusp && bsymb_pos_y_correct_hardusp;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_x_correct",
                                                        bsymb_pos_x_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_y_correct",
                                                        bsymb_pos_y_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sx_correct",
                                                        bsymb_pos_x_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sy_correct",
                                                        bsymb_pos_y_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_s_correct",
                                                        bsymb_pos_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_correct",
                                                        bsymb_pos_correctsp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_x_correct_hard",
                                                        bsymb_pos_x_correct_hardusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_y_correct_hard",
                                                        bsymb_pos_y_correct_hardusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sx_correct_hard",
                                                        bsymb_pos_x_correct_hardusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sy_correct_hard",
                                                        bsymb_pos_y_correct_hardusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_s_correct_hard",
                                                        bsymb_pos_correct_hardusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_correct_hard",
                                                        bsymb_pos_correct_hardusp,
                                                        true);
        } else {
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_correct_hard", true, true);
        }
    }
    if (hasLine)
        CoordRotAngle (sx, sy, ex, ey, isFliped, angz);
    sxu = sx - lox;
    syu = sy - loy;
    exu = ex - lox;
    eyu = ey - loy;
    if (!cache.isSurveyPointTransformationRead)
        cache.ReadSurveyPointTransformation ();
    if (cache.isSurveyPointTransformation_OK) {
        API_Coord3D vtx = {sx, sy, 0};
        API_Coord3D v = GetWordCoord3DTM (vtx, cache.surv_point_tm);
        sxsp = v.x;
        sysp = v.y;
        vtx = {ex, ey, 0};
        v = GetWordCoord3DTM (vtx, cache.surv_point_tm);
        exsp = v.x;
        eysp = v.y;
    }
    if (hasLine && !hasSymbpos) {
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x", sx, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y", sy, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx", sx, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy", sy, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ex", ex, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ey", ey, true);

        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x", sxu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y", syu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx", sxu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy", syu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ex", exu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ey", eyu, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x", sxsp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y", sysp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx", sxsp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy", sysp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ex", exsp, true);
        ParamHelpers::AddLengthValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ey", eysp, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "windoor_in_wall", true, true);
        if (bsync_coord_correct) {
            if (isFliped) {
                dx = ex - sx;
                dy = ey - sy;
            } else {
                dx = sx - ex;
                dy = sy - ey;
            }
            double l = sqrt (dx * dx + dy * dy);
            bool bl_correct = check_accuracy (l, tolerance_coord);
            bool bsymb_pos_sx_correct = check_accuracy (sx, tolerance_coord);
            bool bsymb_pos_sy_correct = check_accuracy (sy, tolerance_coord);
            bool bsymb_pos_ex_correct = check_accuracy (ex, tolerance_coord);
            bool bsymb_pos_ey_correct = check_accuracy (ey, tolerance_coord);
            bool bsymb_pos_e_correct = bsymb_pos_sx_correct && bsymb_pos_sy_correct;
            bool bsymb_pos_s_correct = bsymb_pos_ex_correct && bsymb_pos_ey_correct;
            bool bsymb_pos_correct = bsymb_pos_e_correct && bsymb_pos_s_correct;
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "l_correct", bl_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct", bsymb_pos_s_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_e_correct", bsymb_pos_e_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_x_correct",
                                                        bsymb_pos_sx_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_y_correct",
                                                        bsymb_pos_sy_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sx_correct",
                                                        bsymb_pos_sx_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sy_correct",
                                                        bsymb_pos_sy_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ex_correct",
                                                        bsymb_pos_ex_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ey_correct",
                                                        bsymb_pos_ey_correct,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", bsymb_pos_correct, true);

            bool bsymb_pos_sx_correctu = check_accuracy (sxu, tolerance_coord);
            bool bsymb_pos_sy_correctu = check_accuracy (syu, tolerance_coord);
            bool bsymb_pos_ex_correctu = check_accuracy (exu, tolerance_coord);
            bool bsymb_pos_ey_correctu = check_accuracy (eyu, tolerance_coord);
            bool bsymb_pos_e_correctu = bsymb_pos_sx_correctu && bsymb_pos_sy_correctu;
            bool bsymb_pos_s_correctu = bsymb_pos_ex_correctu && bsymb_pos_ey_correctu;
            bool bsymb_pos_correctu = bsymb_pos_e_correctu && bsymb_pos_s_correctu;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_s_correct",
                                                        bsymb_pos_s_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_e_correct",
                                                        bsymb_pos_e_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_x_correct",
                                                        bsymb_pos_sx_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_y_correct",
                                                        bsymb_pos_sy_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sx_correct",
                                                        bsymb_pos_sx_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sy_correct",
                                                        bsymb_pos_sy_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ex_correct",
                                                        bsymb_pos_ex_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ey_correct",
                                                        bsymb_pos_ey_correctu,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct", bsymb_pos_correctu, true);

            bool bl_correct_hard = check_accuracy (l, tolerance_coord_hard);
            bool bsymb_pos_sx_correct_hard = check_accuracy (sx, tolerance_coord_hard);
            bool bsymb_pos_sy_correct_hard = check_accuracy (sy, tolerance_coord_hard);
            bool bsymb_pos_ex_correct_hard = check_accuracy (ex, tolerance_coord_hard);
            bool bsymb_pos_ey_correct_hard = check_accuracy (ey, tolerance_coord_hard);
            bool bsymb_pos_e_correct_hard = bsymb_pos_sx_correct_hard && bsymb_pos_sy_correct_hard;
            bool bsymb_pos_s_correct_hard = bsymb_pos_ex_correct_hard && bsymb_pos_ey_correct_hard;
            bool bsymb_pos_correct_hard = bsymb_pos_e_correct_hard && bsymb_pos_s_correct_hard;
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "l_correct_hard", bl_correct, true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_s_correct_hard",
                                                        bsymb_pos_s_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_e_correct_hard",
                                                        bsymb_pos_e_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_x_correct_hard",
                                                        bsymb_pos_sx_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_y_correct_hard",
                                                        bsymb_pos_sy_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sx_correct_hard",
                                                        bsymb_pos_sx_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sy_correct_hard",
                                                        bsymb_pos_sy_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ex_correct_hard",
                                                        bsymb_pos_ex_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_ey_correct_hard",
                                                        bsymb_pos_ey_correct_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_correct_hard",
                                                        bsymb_pos_correct_hard,
                                                        true);
            bool bsymb_pos_sx_correctu_hard = check_accuracy (sxu, tolerance_coord_hard);
            bool bsymb_pos_sy_correctu_hard = check_accuracy (syu, tolerance_coord_hard);
            bool bsymb_pos_ex_correctu_hard = check_accuracy (exu, tolerance_coord_hard);
            bool bsymb_pos_ey_correctu_hard = check_accuracy (eyu, tolerance_coord_hard);
            bool bsymb_pos_e_correctu_hard = bsymb_pos_sx_correctu_hard && bsymb_pos_sy_correctu_hard;
            bool bsymb_pos_s_correctu_hard = bsymb_pos_ex_correctu_hard && bsymb_pos_ey_correctu_hard;
            bool bsymb_pos_correctu_hard = bsymb_pos_e_correctu_hard && bsymb_pos_s_correctu_hard;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_s_correct_hard",
                                                        bsymb_pos_s_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_e_correct_hard",
                                                        bsymb_pos_e_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_x_correct_hard",
                                                        bsymb_pos_sx_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_y_correct_hard",
                                                        bsymb_pos_sy_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sx_correct_hard",
                                                        bsymb_pos_sx_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_sy_correct_hard",
                                                        bsymb_pos_sy_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ex_correct_hard",
                                                        bsymb_pos_ex_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_ey_correct_hard",
                                                        bsymb_pos_ey_correctu_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_lo_correct_hard",
                                                        bsymb_pos_correctu_hard,
                                                        true);

            bool bsymb_pos_sx_correctusp = check_accuracy (sxsp, tolerance_coord);
            bool bsymb_pos_sy_correctusp = check_accuracy (sysp, tolerance_coord);
            bool bsymb_pos_ex_correctusp = check_accuracy (exsp, tolerance_coord);
            bool bsymb_pos_ey_correctusp = check_accuracy (eysp, tolerance_coord);
            bool bsymb_pos_e_correctusp = bsymb_pos_sx_correctusp && bsymb_pos_sy_correctusp;
            bool bsymb_pos_s_correctusp = bsymb_pos_ex_correctusp && bsymb_pos_ey_correctusp;
            bool bsymb_pos_correctusp = bsymb_pos_e_correctusp && bsymb_pos_s_correctusp;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_s_correct",
                                                        bsymb_pos_s_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_e_correct",
                                                        bsymb_pos_e_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_x_correct",
                                                        bsymb_pos_sx_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_y_correct",
                                                        bsymb_pos_sy_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sx_correct",
                                                        bsymb_pos_sx_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sy_correct",
                                                        bsymb_pos_sy_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_ex_correct",
                                                        bsymb_pos_ex_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_ey_correct",
                                                        bsymb_pos_ey_correctusp,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_correct",
                                                        bsymb_pos_correctusp,
                                                        true);
            bool bsymb_pos_sx_correctusp_hard = check_accuracy (sxsp, tolerance_coord_hard);
            bool bsymb_pos_sy_correctusp_hard = check_accuracy (sysp, tolerance_coord_hard);
            bool bsymb_pos_ex_correctusp_hard = check_accuracy (exsp, tolerance_coord_hard);
            bool bsymb_pos_ey_correctusp_hard = check_accuracy (eysp, tolerance_coord_hard);
            bool bsymb_pos_e_correctusp_hard = bsymb_pos_sx_correctusp_hard && bsymb_pos_sy_correctusp_hard;
            bool bsymb_pos_s_correctusp_hard = bsymb_pos_ex_correctusp_hard && bsymb_pos_ey_correctusp_hard;
            bool bsymb_pos_correctusp_hard = bsymb_pos_e_correctusp_hard && bsymb_pos_s_correctusp_hard;
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_s_correct_hard",
                                                        bsymb_pos_s_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_e_correct_hard",
                                                        bsymb_pos_e_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_x_correct_hard",
                                                        bsymb_pos_sx_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_y_correct_hard",
                                                        bsymb_pos_sy_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sx_correct_hard",
                                                        bsymb_pos_sx_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_sy_correct_hard",
                                                        bsymb_pos_sy_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_ex_correct_hard",
                                                        bsymb_pos_ex_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_ey_correct_hard",
                                                        bsymb_pos_ey_correctusp_hard,
                                                        true);
            ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                        element.header.guid,
                                                        COORDNAMEPREFIX,
                                                        "symb_pos_sp_correct_hard",
                                                        bsymb_pos_correctusp_hard,
                                                        true);

        } else {
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "l_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_e_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ex_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ey_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "l_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_e_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ex_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_ey_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_e_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ex_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ey_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_e_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ex_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_ey_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_lo_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_s_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_e_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ex_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ey_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_correct_hard", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_s_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_e_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_x_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_y_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sx_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_sy_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ex_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_ey_correct", true, true);
            ParamHelpers::AddBoolValueToParamDictValue (
                pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_pos_sp_correct", true, true);
        }
    }
    if (!skip_north)
        CoordNorthAngle (north, angz, angznorth, angznorthtxt, angznorthtxteng);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir", angznorth, true);
    ParamHelpers::AddStringValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir_str", angznorthtxt, true);
    ParamHelpers::AddStringValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "north_dir_eng", angznorthtxteng, true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle", angz, true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod5", fmod (angz, 5.0), true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod10", fmod (angz, 10.0), true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod45", fmod (angz, 45.0), true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod90", fmod (angz, 90.0), true);
    ParamHelpers::AddDoubleValueToParamDictValue (
        pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_mod180", fmod (angz, 180.0), true);
    if (bsync_coord_correct) {
        bsymb_rotangle_correct =
            CoordCorrectAngle (angz, tolerance_ang, symb_rotangle_fraction, bsymb_rotangle_correct_1000);
        ParamHelpers::AddDoubleValueToParamDictValue (pdictvaluecoord,
                                                      element.header.guid,
                                                      COORDNAMEPREFIX,
                                                      "symb_rotangle_fraction",
                                                      symb_rotangle_fraction,
                                                      true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_correct",
                                                    bsymb_rotangle_correct,
                                                    true);
        ParamHelpers::AddBoolValueToParamDictValue (pdictvaluecoord,
                                                    element.header.guid,
                                                    COORDNAMEPREFIX,
                                                    "symb_rotangle_correct_1000",
                                                    bsymb_rotangle_correct_1000,
                                                    true);
    } else {
        ParamHelpers::AddDoubleValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_fraction", 0.0, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_correct", true, true);
        ParamHelpers::AddBoolValueToParamDictValue (
            pdictvaluecoord, element.header.guid, COORDNAMEPREFIX, "symb_rotangle_correct_1000", true, true);
    }
    return true;
}

// -----------------------------------------------------------------------------
// Проверяет наличие дробной части у угла с заданной точностью
// -----------------------------------------------------------------------------
bool CoordCorrectAngle (double angz,
                        double &tolerance_ang,
                        double &symb_rotangle_fraction,
                        bool &bsymb_rotangle_correct_1000) {
    symb_rotangle_fraction = 1000.0 * fabs (fabs (angz) - floor (fabs (angz))) / tolerance_ang;
    double angz_ = angz / 1000.0;
    bool bsymb_rotangle_correct = check_accuracy (angz_, tolerance_ang);
    bsymb_rotangle_correct_1000 = check_accuracy (angz_, 0.001);
    return bsymb_rotangle_correct;
}

// -----------------------------------------------------------------------------
// По заданному углу поворота и глобальному углу направления на север возвращает ориентацию объекта
// и текст с обозначением стороны света (RUS+ENG)
// -----------------------------------------------------------------------------
void CoordNorthAngle (
    double north, double angz, double &angznorth, GS::UniString &angznorthtxt, GS::UniString &angznorthtxteng) {
    double k = 100000.0;
    angznorth = fmod (angz - north + 90.0, 360.0);
    if (fabs (angznorth) < 0.0000001) {
        angznorth = 0.0;
    } else {
        if (angznorth < 0.0)
            angznorth = 360.0 + angznorth;
        angznorth = round (angznorth * k) / k;
    }
    double n = 0.0;    //"С"
    double nw = 45.0;  //"СЗ"
    double w = 90.0;   //"З"
    double sw = 135.0; //"ЮЗ"
    double s = 180.0;  //"Ю"
    double se = 225.0; //"ЮВ"
    double e = 270.0;  //"В";
    double ne = 315.0; //"СB";
    double nn = 360.0;
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    if (angznorth > nn - 22.5 || angznorth < n + 22.5)
        angznorthtxt = RSGetIndString (iseng, N_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > ne - 22.5 && angznorth < ne + 22.5)
        angznorthtxt = RSGetIndString (iseng, NE_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > e - 22.5 && angznorth < e + 22.5)
        angznorthtxt = RSGetIndString (iseng, E_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > se - 22.5 && angznorth < se + 22.5)
        angznorthtxt = RSGetIndString (iseng, SE_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > s - 22.5 && angznorth < s + 22.5)
        angznorthtxt = RSGetIndString (iseng, S_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > sw - 22.5 && angznorth < sw + 22.5)
        angznorthtxt = RSGetIndString (iseng, SW_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > w - 22.5 && angznorth < w + 22.5)
        angznorthtxt = RSGetIndString (iseng, W_StringID, ACAPI_GetOwnResModule ());
    if (angznorth > nw - 22.5 && angznorth < nw + 22.5)
        angznorthtxt = RSGetIndString (iseng, NW_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, n + 22.5))
        angznorthtxt = RSGetIndString (iseng, N_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, ne + 22.5))
        angznorthtxt = RSGetIndString (iseng, NE_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, e + 22.5))
        angznorthtxt = RSGetIndString (iseng, E_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, se + 22.5))
        angznorthtxt = RSGetIndString (iseng, SE_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, s + 22.5))
        angznorthtxt = RSGetIndString (iseng, S_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, sw + 22.5))
        angznorthtxt = RSGetIndString (iseng, SW_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, w + 22.5))
        angznorthtxt = RSGetIndString (iseng, W_StringID, ACAPI_GetOwnResModule ());
    if (is_equal (angznorth, nn - 22.5))
        angznorthtxt = RSGetIndString (iseng, NW_StringID, ACAPI_GetOwnResModule ());

    if (angznorth > nn - 22.5 || angznorth < n + 22.5)
        angznorthtxteng = "N";
    if (angznorth > ne - 22.5 && angznorth < ne + 22.5)
        angznorthtxteng = "NE";
    if (angznorth > e - 22.5 && angznorth < e + 22.5)
        angznorthtxteng = "E";
    if (angznorth > se - 22.5 && angznorth < se + 22.5)
        angznorthtxteng = "SE";
    if (angznorth > s - 22.5 && angznorth < s + 22.5)
        angznorthtxteng = "S";
    if (angznorth > sw - 22.5 && angznorth < sw + 22.5)
        angznorthtxteng = "SW";
    if (angznorth > w - 22.5 && angznorth < w + 22.5)
        angznorthtxteng = "W";
    if (angznorth > nw - 22.5 && angznorth < nw + 22.5)
        angznorthtxteng = "NW";
    if (is_equal (angznorth, n + 22.5))
        angznorthtxteng = "N";
    if (is_equal (angznorth, ne + 22.5))
        angznorthtxteng = "NE";
    if (is_equal (angznorth, e + 22.5))
        angznorthtxteng = "E";
    if (is_equal (angznorth, se + 22.5))
        angznorthtxteng = "SE";
    if (is_equal (angznorth, s + 22.5))
        angznorthtxteng = "S";
    if (is_equal (angznorth, sw + 22.5))
        angznorthtxteng = "SW";
    if (is_equal (angznorth, w + 22.5))
        angznorthtxteng = "W";
    if (is_equal (angznorth, nn - 22.5))
        angznorthtxteng = "NW";
}

// -----------------------------------------------------------------------------
// Вычисляет уголв поворота элемента по координатам его начала и конца
// -----------------------------------------------------------------------------
void CoordRotAngle (double sx, double sy, double ex, double ey, bool isFliped, double &angz) {
    double k = 1000000000.0;
    double dx;
    double dy;
    if (isFliped) {
        dx = ex - sx;
        dy = ey - sy;
    } else {
        dx = sx - ex;
        dy = sy - ey;
    }
    if (is_equal (dx, 0.0) && is_equal (dy, 0.0)) {
        angz = 0.0;
    } else {
        angz = atan2 (dy, dx) + PI;
    }
    if (fabs (angz) > 0.000000001) {
        angz = fmod (round ((angz * 180.0 / PI) * k) / k, 360.0);
    } else {
        angz = 0.0;
    }
}

// -----------------------------------------------------------------------------
// Получение имени внутренних свойств по русскому имени
// -----------------------------------------------------------------------------
GS::UniString GetPropertyENGName (GS::UniString &name) {
    auto &cache = PROPERTYCACHE ();
    if (!name.Contains (PROP_PREFIX))
        return name;
    if (name.Contains (PROP_SYNC_NAME))
        return name;
    if (name.IsEqual (PROP_ID))
        return MAT_BUILDING_MATERIAL_ID;
    if (name.IsEqual (PROP_N))
        return MAT_N;
    if (name.IsEqual (PROP_NS))
        return MAT_NS;
    if (name.IsEqual (PROP_LAYER_THICKNESS))
        return MAT_LAYER_THICKNESS;
    if (name.IsEqual (PROP_TH))
        return MAT_LAYER_THICKNESS;
    if (name.IsEqual (PROP_LAYER_MIN_THICKNESS))
        return MAT_LAYER_MIN_THICKNESS;
    if (name.IsEqual (PROP_TH_MIN))
        return MAT_LAYER_MIN_THICKNESS;
    if (name.IsEqual (PROP_BMAT_INX))
        return MAT_BMAT_INX;
    if (name.IsEqual (PROP_CUTFILL_INX))
        return MAT_CUTFILL_INX;
    if (name.IsEqual (PROP_SOME_STUFF_TH))
        return MAT_SOME_STUFF_TH;
    if (name.IsEqual (PROP_SOME_STUFF_UNITS))
        return MAT_SOME_STUFF_UNITS;
    if (name.IsEqual (PROP_UNIT))
        return MAT_SOME_STUFF_UNITS;
    if (name.IsEqual (PROP_KZAP))
        return MAT_SOME_STUFF_KZAP;
    if (name.IsEqual (PROP_AREA))
        return MAT_AREA;
    if (name.IsEqual (PROP_VOLUME))
        return MAT_VOLUME;
    if (name.IsEqual (PROP_QTY))
        return MAT_QTY;
    if (name.IsEqual (PROP_UNIT_PREFIX))
        return MAT_UNIT_PREFIX;
    if (name.IsEqual (PROP_LENGTH))
        return MAT_LENGTH;
    if (name.IsEqual (PROP_AREA_SECTION))
        return MAT_AREA_SECTION;
    if (name.IsEqual (PROP_WIDTH))
        return MAT_WIDTH;
    if (name.IsEqual (cache.buildingMaterialNameString))
        return MAT_BUILDING_MATERIAL_NAME;
    if (name.IsEqual (cache.buildingMaterialDescriptionString))
        return MAT_BUILDING_MATERIAL_DESCRIPTION;
    if (name.IsEqual (cache.buildingMaterialDensityString))
        return MAT_BUILDING_MATERIAL_DENSITY;
    if (name.IsEqual (cache.buildingMaterialManufacturerString))
        return MAT_BUILDING_MATERIAL_MANUFACTURER;
    if (name.IsEqual (cache.buildingMaterialCutFillString))
        return MAT_BUILDING_MATERIAL_CUTFILL;
    if (name.IsEqual (cache.thicknessString))
        return MAT_LAYER_THICKNESS;
    return name;
}

void ParamHelpers::ReplaceProcToBrace (GS::UniString &expression, bool fromMaterial) {
    if (expression.IsEmpty ())
        return;
    if (expression.FindFirst (CHARPROC) == MaxUIndex) {
        return;
    }
    GS::UniString result;
    result.SetCapacity (expression.GetLength () + 32);
    UIndex lastPos = 0;
    const UIndex len = expression.GetLength ();
    while (lastPos < len) {
        UIndex firstProc = expression.FindFirst (CHARPROC, lastPos);
        if (firstProc == MaxUIndex) {
            // Больше символов CHARPROC нет, копируем весь остаток строки и выходим
            result.Append (expression.GetSubstring (lastPos, len - lastPos));
            break;
        }
        // Ищем парный (второй) CHARPROC
        UIndex secondProc = expression.FindFirst (CHARPROC, firstProc + 1);
        if (secondProc == MaxUIndex) {
            // Нашлась непарная "процентка", копируем остаток строки как есть
            result.Append (expression.GetSubstring (lastPos, len - lastPos));
            break;
        }
        // Копируем обычный текст, который находился ДО
        if (firstProc > lastPos) {
            result.Append (expression.GetSubstring (lastPos, firstProc - lastPos));
        }
        GS::UniString part = expression.GetSubstring (firstProc + 1, secondProc - firstProc - 1);
        if (!part.IsEmpty ()) {
            // Аппендим префикс без создания временных строк через оператор "+"
            if (fromMaterial) {
                result.Append (PROPERTYNAMEPREFIX);
            } else {
                result.Append (BRACESTART);
            }
            // Добавляем имя свойства в нижнем регистре
            result.Append (part.ToLowerCase ());
            result.Append (CHARBRACEEND);
        } else {
            result.Append (CHARPROC);
            result.Append (CHARPROC);
        }
        // Сдвигаем указатель за второй обработанный CHARPROC
        lastPos = secondProc + 1;
    }
    expression = std::move (result);
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки %
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamNameMaterial (GS::UniString &expression, ParamDictValue &paramDict, bool fromMaterial) {
    ParamHelpers::ReplaceProcToBrace (expression, fromMaterial);
    return ParamHelpers::ParseParamName (expression, paramDict);
}

// -----------------------------------------------------------------------------
// Извлекает из строки все имена свойств или параметров, заключенные в знаки { }
// -----------------------------------------------------------------------------
bool ParamHelpers::ParseParamName (GS::UniString &expression, ParamDictValue &paramDict) {
    if (expression.IsEmpty ())
        return false;
    if (expression.FindFirst (CHARBRACESTART) == MaxUIndex)
        return false;
    GS::UniString result;
    result.SetCapacity (expression.GetLength () + 120);
    UIndex lastPos = 0;
    const UIndex len = expression.GetLength ();
    while (lastPos < len) {
        // Ищем следующую открывающую скобку
        UIndex startBrace = expression.FindFirst (CHARBRACESTART, lastPos);
        if (startBrace == MaxUIndex) {
            // Больше скобок нет, копируем остаток строки и выходим
            result.Append (expression.GetSubstring (lastPos, len - lastPos));
            break;
        }
        // Ищем соответствующую закрывающую скобку
        UIndex endBrace = expression.FindFirst (CHARBRACEEND, startBrace + 1);
        if (endBrace == MaxUIndex) {
            // Нашлась непарная скобка, копируем всё до конца как есть
            result.Append (expression.GetSubstring (lastPos, len - lastPos));
            break;
        }
        // Копируем обычный текст, который был ДО скобки
        if (startBrace > lastPos)
            result.Append (expression.GetSubstring (lastPos, startBrace - lastPos));

        // Выделяем токен внутри скобок {part}
        GS::UniString part = expression.GetSubstring (startBrace + 1, endBrace - startBrace - 1);
        FormatString formatstring;
        GS::UniString part_ = ParamHelpers::NameToRawName (part, formatstring);
        if (!paramDict.ContainsKey (part_)) {
            ParamValue pvalue = {};
            pvalue.rawName = part_;

            short inx = ParamHelpers::GetTypeInxByRawnamePrefix (part_);
            pvalue.typeinx = inx;

            switch (inx) {
            case PROPERTYTYPEINX:
                pvalue.fromPropertyDefinition = true;
                break;
            case GDLTYPEINX:
                pvalue.fromGDLparam = true;
                break;
            case GDLDESCTYPEINX:
                pvalue.fromGDLparam = true;
                pvalue.fromGDLdescription = true;
                break;
            case COORDTYPEINX:
                pvalue.fromCoord = true;
                break;
            case GLOBTYPEINX:
                pvalue.fromGlob = true;
                break;
            case INFOTYPEINX:
                pvalue.fromInfo = true;
                break;
            case MEPTYPEINX:
                pvalue.fromMEP = true;
                break;
            case FORMULATYPEINX:
                pvalue.val.hasFormula = true;
                break;
            case IDTYPEINX:
                pvalue.fromID = true;
                break;
            case IFCTYPEINX:
                pvalue.fromIFCProperty = true;
                break;
            case MORPHTYPEINX:
                pvalue.fromMorph = true;
                break;
            case ATTRIBTYPEINX:
                pvalue.fromAttribElement = true;
                break;
            case LISTDATATYPEINX:
                pvalue.fromListData = true;
                break;
            case MATERIALTYPEINX:
                pvalue.fromMaterial = true;
                break;
            case CLASSTYPEINX:
                pvalue.fromClassification = true;
                break;
            case ELEMENTTYPEINX:
                pvalue.fromElement = true;
                break;
            case FILETYPEINX:
                pvalue.fromFile = true;
                break;
            default:
                break;
            }
            pvalue.name = part.ToLowerCase ();
            pvalue.val.formatstring = formatstring;
            paramDict.Put (part_, std::move (pvalue));
        }
        if (!formatstring.isEmpty)
            part_.ReplaceAll (BRACEEND, DOT + formatstring.stringformat + BRACEEND);
        result.Append (part_);
        lastPos = endBrace + 1;
    }
    expression = std::move (result);
    return true;
}

// -----------------------------------------------------------------------------
// Замена имен параметров на значения в выражении
// Значения передаются словарём, вычисление значений см. GetParamValueDict
// -----------------------------------------------------------------------------
bool ParamHelpers::ReplaceParamInExpression (const ParamDictValue &pdictvalue, GS::UniString &expression) {
    if (pdictvalue.IsEmpty () || expression.IsEmpty ())
        return false;
    if (expression.FindFirst (CHARBRACESTART) == MaxUIndex)
        return true;
    bool overall_flag_find = false;
    GS::UniString attribsuffix_old;
    int iteration = 0;
    const int MAX_ITERATIONS = 10; // Максимальная глубина вложенности свойств
    // Внешний цикл: крутится до тех пор, пока в строке остаются скобки {
    while (expression.FindFirst (CHARBRACESTART) != MaxUIndex && iteration < MAX_ITERATIONS) {
        iteration++;
        GS::UniString result;
        result.SetCapacity (expression.GetLength () + 128);
        UIndex lastPos = 0;
        const UIndex len = expression.GetLength ();
        bool current_pass_changed = false;
        while (lastPos < len) {
            UIndex startBrace = expression.FindFirst (CHARBRACESTART, lastPos);
            if (startBrace == MaxUIndex) {
                result.Append (expression.GetSubstring (lastPos, len - lastPos));
                break;
            }
            UIndex endBrace = expression.FindFirst (CHARBRACEEND, startBrace + 1);
            if (endBrace == MaxUIndex) {
                result.Append (expression.GetSubstring (lastPos, len - lastPos));
                break;
            }

            if (startBrace > lastPos)
                result.Append (expression.GetSubstring (lastPos, startBrace - lastPos));
            GS::UniString part = expression.GetSubstring (startBrace + 1, endBrace - startBrace - 1);
            GS::UniString val;
            if (!part.IsEmpty ()) {
                GS::UniString part_clean = part;
                GS::UniString attribsuffix;

                if (part.Contains (CharENTER)) {
                    UIndex attribinx = part.FindLast (CharENTER);
                    USize partlen = part_clean.GetLength () - attribinx;
                    attribsuffix = part_clean.GetSubstring (attribinx, partlen) + CHARBRACEEND;
                    part_clean = part_clean.GetSubstring (0, attribinx);
                }

                FormatString formatstring;
                GS::UniString stringformat = FormatStringFunc::GetFormatString (part_clean);
                if (!stringformat.IsEmpty ()) {
                    formatstring = FormatStringFunc::ParseFormatString (stringformat);
                }

                GS::UniString partc_normal = BRACESTART + part_clean + BRACEEND;
                GS::UniString partc_current =
                    !attribsuffix.IsEmpty () ? (BRACESTART + part_clean + attribsuffix) : GS::UniString ();
                GS::UniString partc_old =
                    !attribsuffix_old.IsEmpty () ? (BRACESTART + part_clean + attribsuffix_old) : GS::UniString ();

                const ParamValue *pvaluePtr = nullptr;
                GS::UniString matchedKey;

                pvaluePtr = pdictvalue.GetPtr (partc_normal);
                if (pvaluePtr != nullptr && pvaluePtr->isValid) {
                    matchedKey = partc_normal;
                } else {
                    if (!partc_current.IsEmpty ()) {
                        pvaluePtr = pdictvalue.GetPtr (partc_current);
                        if (pvaluePtr != nullptr && pvaluePtr->isValid) {
                            matchedKey = partc_current;
                        }
                    }
                    if ((pvaluePtr == nullptr || !pvaluePtr->isValid) && !partc_old.IsEmpty ()) {
                        pvaluePtr = pdictvalue.GetPtr (partc_old);
                        if (pvaluePtr != nullptr && pvaluePtr->isValid) {
                            matchedKey = partc_old;
                        }
                    }
                }

                if (pvaluePtr != nullptr && pvaluePtr->isValid) {
                    ParamValue pvalue = *pvaluePtr;
                    if (!formatstring.isEmpty) {
                        pvalue.val.formatstring = formatstring;
                    }
                    val = ParamHelpers::ToString (pvalue);
                    overall_flag_find = true;
                } else {
                    val.Clear ();
#if defined(TESTING)
                    if (pvaluePtr != nullptr && !pvaluePtr->isValid) {
                        DBprnt ("ReplaceParamInExpression err pvalue.isValid",
                                !matchedKey.IsEmpty () ? matchedKey : partc_normal);
                    } else {
                        DBprnt ("ReplaceParamInExpression err not found parametr", partc_normal);
                    }
#endif
                }
                if (!attribsuffix.IsEmpty ())
                    attribsuffix_old = attribsuffix;
            }
            result.Append (val);
            current_pass_changed = true;
            lastPos = endBrace + 1;
        }
        expression = std::move (result);
        if (!current_pass_changed)
            break;
    }
#if defined(TESTING)
    if (iteration >= MAX_ITERATIONS) {
        DBprnt ("ReplaceParamInExpression err циклическая ссылка в свойствах.", expression);
    }
#endif
    return overall_flag_find;
}

bool ParamHelpers::GetParamValueForElements (const API_Guid &elemguid,
                                             const GS::UniString &rawname,
                                             const ParamDictElement &paramToRead,
                                             ParamValue &pvalue) {
    if (const auto *p = paramToRead.GetPtr (elemguid)) {
        if (const auto *val = p->GetPtr (rawname)) {
            if (val->isValid) {
                pvalue = *val;
                return true;
            }
        }
    }
    return false;
}

GS::UniString PropertyHelpers::ToString (const API_Variant &variant, const FormatString &stringformat) {
    switch (variant.type) {
    case API_PropertyIntegerValueType:
        return FormatStringFunc::NumToString (variant.intValue, stringformat);
    case API_PropertyRealValueType:
        return FormatStringFunc::NumToString (variant.doubleValue, stringformat);
    case API_PropertyStringValueType:
        return variant.uniStringValue;
    case API_PropertyBooleanValueType:
        return GS::ValueToUniString (variant.boolValue);
    case API_PropertyGuidValueType:
        return APIGuid2GSGuid (variant.guidValue).ToUniString ();
    case API_PropertyUndefinedValueType:
        return "@Undefined Value@";
    default:
        DBBREAK ();
        return "@Invalid Value@";
    }
}

GS::UniString PropertyHelpers::ToString (const API_Variant &variant) {
    FormatString f;
    return PropertyHelpers::ToString (variant, f);
}

GS::UniString PropertyHelpers::ToString (const API_Property &property) {
    FormatString f;
    return PropertyHelpers::ToString (property, f);
}

GS::UniString PropertyHelpers::ToString (const API_Property &property, const FormatString &stringformat) {
    GS::UniString string;
    const API_PropertyValue *value;
#if defined(AC_22) || defined(AC_23)
    if (!property.isEvaluated) {
        return string;
    }
    if (property.isDefault && !property.isEvaluated) {
        value = &property.definition.defaultValue.basicValue;
    } else {
        value = &property.value;
    }
#else
    if (property.status == API_Property_NotAvailable) {
        return string;
    }
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = &property.definition.defaultValue.basicValue;
    } else {
        value = &property.value;
    }
#endif
    switch (property.definition.collectionType) {
    case API_PropertySingleCollectionType: {
        string += ToString (value->singleVariant.variant, stringformat);
    } break;
    case API_PropertyListCollectionType: {
        for (UInt32 i = 0; i < value->listVariant.variants.GetSize (); i++) {
            string.Append (ToString (value->listVariant.variants[i], stringformat));
            if (i != value->listVariant.variants.GetSize () - 1) {
                string.Append ("; ");
            }
        }
    } break;
    case API_PropertySingleChoiceEnumerationCollectionType: {
#if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
        API_Guid guidValue = value->singleVariant.variant.guidValue;
        GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
        for (UInt32 i = 0; i < possibleEnumValues.GetSize (); i++) {
            if (possibleEnumValues[i].keyVariant.guidValue == guidValue) {
                string.Append (ToString (possibleEnumValues[i].displayVariant, stringformat));
                break;
            }
        }
#else // AC_25
        string += ToString (value->singleEnumVariant.displayVariant, stringformat);
#endif
    } break;
    case API_PropertyMultipleChoiceEnumerationCollectionType: {
#if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
        GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;
        UInt32 qty_finded_values = value->listVariant.variants.GetSize ();
        for (UInt32 i = 0; i < possibleEnumValues.GetSize (); i++) {
            API_Guid guidValue = possibleEnumValues[i].keyVariant.guidValue;
            for (UInt32 j = 0; j < value->listVariant.variants.GetSize (); j++) {
                if (value->listVariant.variants[j].guidValue == guidValue) {
                    string.Append (ToString (possibleEnumValues[i].displayVariant, stringformat));
                    qty_finded_values = qty_finded_values - 1;
                    if (qty_finded_values != 0)
                        string.Append ("; ");
                    break;
                }
            }
            if (qty_finded_values == 0)
                break;
        }
#else // AC_25
        for (UInt32 i = 0; i < value->multipleEnumVariant.variants.GetSize (); i++) {
            string += ToString (value->multipleEnumVariant.variants[i].displayVariant, stringformat);
            if (i != value->multipleEnumVariant.variants.GetSize () - 1) {
                string.Append ("; ");
            }
        }
#endif
    } break;
    default: {
        break;
    }
    }
    return string;
}

ParamValueData operator+ (const ParamValueData &lhs, const ParamValueData &rhs) {
    ParamValueData out = lhs;
    if (!lhs.hasrawDouble || !rhs.hasrawDouble)
        out.hasrawDouble = false;
    if (lhs.hasrawDouble && rhs.hasrawDouble)
        out.hasrawDouble = true;
    out.doubleValue = lhs.doubleValue + rhs.doubleValue;

    double lhsrawDoubleValue = 0;
    if (lhs.hasrawDouble) {
        lhsrawDoubleValue = lhs.rawDoubleValue;
    } else {
        lhsrawDoubleValue = lhs.doubleValue;
    }
    double rhsrawDoubleValue = 0;
    if (rhs.hasrawDouble) {
        rhsrawDoubleValue = rhs.rawDoubleValue;
    } else {
        rhsrawDoubleValue = rhs.doubleValue;
    }
    out.rawDoubleValue = lhsrawDoubleValue + rhsrawDoubleValue;
    out.uniStringValue = lhs.uniStringValue;
    out.uniStringValue.Append (SEMICOLON);
    out.uniStringValue.Append (rhs.uniStringValue);
    out.intValue = lhs.intValue + rhs.intValue;
    return out;
}

bool operator== (const ParamValue &lhs, const ParamValue &rhs) {
    double lhsd = 0.0;
    double rhsd = 0.0;
    if (!lhs.isValid)
        return false;
    if (!rhs.isValid)
        return false;
    switch (rhs.val.type) {
    case API_PropertyIntegerValueType:
        return lhs.val.intValue == rhs.val.intValue;
    case API_PropertyRealValueType:
        rhsd = rhs.val.doubleValue;
        if (!rhs.val.formatstring.needRound && !is_equal (rhs.val.rawDoubleValue, 0) && rhs.val.hasrawDouble) {
            rhsd = rhs.val.rawDoubleValue;
        }
        lhsd = lhs.val.doubleValue;
        if (!lhs.val.formatstring.needRound && !is_equal (lhs.val.rawDoubleValue, 0) && lhs.val.hasrawDouble) {
            lhsd = lhs.val.rawDoubleValue;
        }
        if (lhs.val.type == API_PropertyStringValueType && lhs.val.canCalculate) {
            Int32 n_zero = lhs.val.formatstring.n_zero;
            Int32 krat = lhs.val.formatstring.krat;
            double koeff = lhs.val.formatstring.koeff;
            bool trim_zero = lhs.val.formatstring.trim_zero;
            if (koeff != 1)
                n_zero = n_zero + (GS::Int32)log10 (koeff);
            lhsd = round_nzero (lhsd, n_zero);
        }
        return is_equal (lhsd, rhsd);
    case API_PropertyStringValueType:
        return lhs.val.uniStringValue == rhs.val.uniStringValue;
    case API_PropertyBooleanValueType:
        return lhs.val.boolValue == rhs.val.boolValue;
    case API_PropertyGuidValueType:
        return lhs.val.guidval == rhs.val.guidval;
    default:
        return false;
    }
}

bool operator!= (const ParamValue &lhs, const ParamValue &rhs) { return !(lhs == rhs); }

bool operator== (const API_Variant &lhs, const API_Variant &rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }

    switch (lhs.type) {
    case API_PropertyIntegerValueType:
        return lhs.intValue == rhs.intValue;
    case API_PropertyRealValueType:
        return lhs.doubleValue == rhs.doubleValue;
    case API_PropertyStringValueType:
        return lhs.uniStringValue == rhs.uniStringValue;
    case API_PropertyBooleanValueType:
        return lhs.boolValue == rhs.boolValue;
    case API_PropertyGuidValueType:
        return lhs.guidValue == rhs.guidValue;
    default:
        return false;
    }
}

bool operator== (const API_SingleVariant &lhs, const API_SingleVariant &rhs) { return lhs.variant == rhs.variant; }

bool operator== (const API_ListVariant &lhs, const API_ListVariant &rhs) { return lhs.variants == rhs.variants; }

bool operator== (const API_SingleEnumerationVariant &lhs, const API_SingleEnumerationVariant &rhs) {
    return lhs.keyVariant == rhs.keyVariant && lhs.displayVariant == rhs.displayVariant;
}

#if !defined(AC_25) && !defined(AC_26) && !defined(AC_27) && !defined(AC_28) && !defined(AC_29)
bool operator== (const API_MultipleEnumerationVariant &lhs, const API_MultipleEnumerationVariant &rhs) {
    return lhs.variants == rhs.variants;
}
#endif

bool Equals (const API_PropertyDefaultValue &lhs,
             const API_PropertyDefaultValue &rhs,
             API_PropertyCollectionType collType) {
    if (lhs.hasExpression != rhs.hasExpression) {
        return false;
    }

    if (lhs.hasExpression) {
        return lhs.propertyExpressions == rhs.propertyExpressions;
    } else {
        return Equals (lhs.basicValue, rhs.basicValue, collType);
    }
}

bool Equals (const API_PropertyValue &lhs, const API_PropertyValue &rhs, API_PropertyCollectionType collType) {
    if (lhs.variantStatus != rhs.variantStatus) {
        return false;
    }

    if (lhs.variantStatus != API_VariantStatusNormal) {
        return true;
    }

    switch (collType) {
    case API_PropertySingleCollectionType:
        return lhs.singleVariant == rhs.singleVariant;
    case API_PropertyListCollectionType:
        return lhs.listVariant == rhs.listVariant;
#if defined(AC_25) || defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
    case API_PropertySingleChoiceEnumerationCollectionType:
        return lhs.singleVariant == rhs.singleVariant;
    case API_PropertyMultipleChoiceEnumerationCollectionType:
        return lhs.listVariant == rhs.listVariant;
#else
    case API_PropertySingleChoiceEnumerationCollectionType:
        return lhs.singleEnumVariant == rhs.singleEnumVariant;
    case API_PropertyMultipleChoiceEnumerationCollectionType:
        return lhs.multipleEnumVariant == rhs.multipleEnumVariant;
#endif
    default:
        DBBREAK ();
        return false;
    }
}

bool operator== (const API_PropertyGroup &lhs, const API_PropertyGroup &rhs) {
    return lhs.guid == rhs.guid && lhs.name == rhs.name;
}

bool operator== (const API_PropertyDefinition &lhs, const API_PropertyDefinition &rhs) {
    return lhs.guid == rhs.guid && lhs.groupGuid == rhs.groupGuid && lhs.name == rhs.name &&
           lhs.description == rhs.description && lhs.collectionType == rhs.collectionType &&
           lhs.valueType == rhs.valueType && lhs.measureType == rhs.measureType &&
           Equals (lhs.defaultValue, rhs.defaultValue, lhs.collectionType) && lhs.availability == rhs.availability &&
           lhs.possibleEnumValues == rhs.possibleEnumValues;
}

bool operator== (const API_Property &lhs, const API_Property &rhs) {
    if (!(lhs.definition == rhs.definition) || !(lhs.isDefault == rhs.isDefault)) {
        return false;
    }
    if (!lhs.isDefault) {
        return Equals (lhs.value, rhs.value, lhs.definition.collectionType);
    } else {
        return true;
    }
}

// -----------------------------------------------------------------------------
// Синхронизация ParamValue и API_Property
// Возвращает true и подготовленное для записи свойство в случае отличий
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToProperty (const ParamValue &pvalue, API_Property &property) {
    if (!property.definition.canValueBeEditable) {
#if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToProperty err", "!property.definition.canValueBeEditable");
#endif
        return false;
    }
    if (!pvalue.isValid) {
#if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToProperty err", "!pvalue.isValid");
#endif
        return false;
    }
    bool flag_rec = false;
    GS::UniString val = "";
    API_PropertyValue value = {};
    bool isEval = true;
    bool isDefult = false;
#if defined(AC_22) || defined(AC_23)
    if (property.isDefault && !property.isEvaluated) {
        value = property.definition.defaultValue.basicValue;
        isDefult = true;
    } else {
        value = property.value;
    }
    isEval = property.isEvaluated;
#else
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = property.definition.defaultValue.basicValue;
        isDefult = true;
    } else {
        value = property.value;
    }
    isEval = (property.status == API_Property_HasValue);
#endif
    if (pvalue.needResetToDef) {
        if (property.isDefault)
            return false;
        property.isDefault = true;
        if (property.value.variantStatus != API_VariantStatusNormal)
            property.value.variantStatus = API_VariantStatusNormal;
#if defined(AC_22) || defined(AC_23)
        if (!property.isEvaluated)
            property.isEvaluated = true;
#else
        if (property.status != API_Property_HasValue) {
            property.status = API_Property_HasValue;
            if (property.definition.collectionType == API_PropertySingleCollectionType &&
                property.value.singleVariant.variant.type == API_PropertyUndefinedValueType) {
                property.value.singleVariant.variant.type = property.definition.valueType;
            }
        }
#endif
#if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToProperty", " reset property : " + pvalue.rawName);
#endif
        return true;
    }
    double dval = pvalue.val.doubleValue;
    if (pvalue.val.formatstring.forceRaw && pvalue.val.hasrawDouble)
        dval = pvalue.val.rawDoubleValue;

    switch (property.definition.valueType) {
    case API_PropertyIntegerValueType:
        if (value.singleVariant.variant.intValue != pvalue.val.intValue || !isEval) {
            property.value.singleVariant.variant.intValue = pvalue.val.intValue;
            flag_rec = true;
        } else {
            if (isDefult) {
                property.value.singleVariant.variant.intValue = pvalue.val.intValue;
                flag_rec = true;
            }
        }
        break;
    case API_PropertyRealValueType:
        // Конвертация угла из радиан в градусы
        if (property.definition.measureType == API_PropertyAngleMeasureType) {
            if (!is_equal (dval * PI / 180.0, value.singleVariant.variant.doubleValue) || !isEval) {
                property.value.singleVariant.variant.doubleValue = dval * PI / 180.0;
                flag_rec = true;
            } else {
                if (isDefult) {
                    property.value.singleVariant.variant.doubleValue = dval * PI / 180.0;
                    flag_rec = true;
                }
            }
        } else {
            if (!is_equal (value.singleVariant.variant.doubleValue, dval) || !isEval) {
                property.value.singleVariant.variant.doubleValue = dval;
                flag_rec = true;
            } else {
                if (isDefult) {
                    property.value.singleVariant.variant.doubleValue = dval;
                    flag_rec = true;
                }
            }
        }
        break;
    case API_PropertyBooleanValueType:
        if (value.singleVariant.variant.boolValue != pvalue.val.boolValue || !isEval) {
            property.value.singleVariant.variant.boolValue = pvalue.val.boolValue;
            flag_rec = true;
        } else {
            if (isDefult) {
                property.value.singleVariant.variant.boolValue = pvalue.val.boolValue;
                flag_rec = true;
            }
        }
        break;
    case API_PropertyStringValueType:
        val = ParamHelpers::ToString (pvalue);
        ReplaceCR (val, true);
        if (value.singleVariant.variant.uniStringValue != val || !isEval) {
            property.value.singleVariant.variant.uniStringValue = val;
            flag_rec = true;
        } else {
            if (isDefult) {
                property.value.singleVariant.variant.uniStringValue = val;
                flag_rec = true;
            }
        }
        break;
    default:
        break;
    }
    if (flag_rec && value.singleVariant.variant.type == API_PropertyGuidValueType &&
        property.definition.collectionType == API_PropertySingleChoiceEnumerationCollectionType) {
        API_Guid guidValue = APINULLGuid;
        GS::Array<API_SingleEnumerationVariant> possibleEnumValues = property.definition.possibleEnumValues;

        // Для свойств с набором параметров необходимо задавать не само значение, а его GUID
        for (const auto &poss_values : possibleEnumValues) {
            switch (property.definition.valueType) {
            case API_PropertyIntegerValueType:
                if (property.value.singleVariant.variant.intValue == poss_values.displayVariant.intValue) {
                    guidValue = poss_values.keyVariant.guidValue;
                }
                break;
            case API_PropertyRealValueType:
                if (!is_equal (property.value.singleVariant.variant.doubleValue,
                               poss_values.displayVariant.doubleValue)) {
                    guidValue = poss_values.keyVariant.guidValue;
                }
                break;
            case API_PropertyBooleanValueType:
                if (property.value.singleVariant.variant.boolValue == poss_values.displayVariant.boolValue) {
                    guidValue = poss_values.keyVariant.guidValue;
                }
                break;
            case API_PropertyStringValueType:
                if (property.value.singleVariant.variant.uniStringValue == poss_values.displayVariant.uniStringValue) {
                    guidValue = poss_values.keyVariant.guidValue;
                }
                break;
            default:
                break;
            }
            if (guidValue != APINULLGuid) {
                property.value.singleVariant.variant.guidValue = guidValue;
                break;
            }
        }
        if (guidValue == APINULLGuid) {
#if defined(TESTING)
            DBprnt ("ParamHelpers::ConvertToProperty err", "guidValue == APINULLGuid");
#endif
            flag_rec = false;
        }
    }
    if (flag_rec) {
        property.isDefault = false;
        if (property.value.variantStatus != API_VariantStatusNormal)
            property.value.variantStatus = API_VariantStatusNormal;
#if defined(AC_22) || defined(AC_23)
        if (!property.isEvaluated)
            property.isEvaluated = true;
#else
        if (property.status != API_Property_HasValue) {
            property.status = API_Property_HasValue;
            if (property.definition.collectionType == API_PropertySingleCollectionType &&
                property.value.singleVariant.variant.type == API_PropertyUndefinedValueType) {
                property.value.singleVariant.variant.type = property.definition.valueType;
            }
        }
#endif
#if defined(TESTING)
    } else {
        DBprnt ("ParamHelpers::ConvertToProperty", " EQ property : " + pvalue.rawName);
#endif
    }
    return flag_rec;
}

//--------------------------------------------------------------------------------------------------------------------------
// Ищет свойство property_flag_name в описании и по значению определяет - нужно ли обрабатывать элемент
//--------------------------------------------------------------------------------------------------------------------------
bool GetElemState (const API_Guid &elemGuid,
                   const GS::Array<API_PropertyDefinition> &definitions,
                   const GS::UniString &property_flag_name,
                   bool &flagfind,
                   bool check) {
    flagfind = false;
    bool flag = false;
    if (definitions.IsEmpty ())
        return false;
    GSErrCode err = NoError;
    short n = 0;
    GS::UniString flag_name = "";
    if (check) {
        for (const auto &definition : definitions) {
            if (definition.description.IsEmpty ())
                continue;
            if (!definition.description.Contains (property_flag_name))
                continue;
            n++;
            flag_name.Append (definition.name);
            flag_name.Append ("; ");
        }
        if (n == 0)
            return false;
        if (n > 1)
            msg_rep ("There are several sync flags as an element. This can lead to errors.",
                     flag_name,
                     APIERR_GENERAL,
                     elemGuid);
    }
    API_Property propertyflag = {};
    for (const auto &definition : definitions) {
        if (definition.description.IsEmpty ())
            continue;
        if (!definition.description.Contains (property_flag_name))
            continue;
        err = ACAPI_Element_GetPropertyValue (elemGuid, definition.guid, propertyflag);
        if (err == NoError) {
            flagfind = true;
#if defined(AC_22) || defined(AC_23)
            if (!propertyflag.isEvaluated) {
                return false;
            }
            if (propertyflag.isDefault && !propertyflag.isEvaluated) {
                return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
            } else {
                return propertyflag.value.singleVariant.variant.boolValue;
            }
#else
            if (propertyflag.status == API_Property_NotAvailable) {
                return false;
            }
            if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
                return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
            } else {
                return propertyflag.value.singleVariant.variant.boolValue;
            }
#endif
        } else {
            return false;
        }
    }
    return false;
}

bool GetElemStateReverse (const API_Guid &elemGuid,
                          const GS::Array<API_PropertyDefinition> &definitions,
                          const GS::UniString &property_flag_name,
                          bool &flagfind) {
    if (definitions.IsEmpty ())
        return false;
    GSErrCode err = NoError;
    API_Property propertyflag = {};
    for (const auto &definition : definitions) {
        if (definition.description.IsEmpty ())
            continue;
        if (!definition.description.Contains (property_flag_name))
            continue;
        err = ACAPI_Element_GetPropertyValue (elemGuid, definition.guid, propertyflag);
        if (err != NoError)
            return true;
        flagfind = true;
#if defined(AC_22) || defined(AC_23)
        if (!propertyflag.isEvaluated) {
            return true;
        }
        if (propertyflag.isDefault && !propertyflag.isEvaluated) {
            return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
        } else {
            return propertyflag.value.singleVariant.variant.boolValue;
        }
#else
        if (propertyflag.status == API_Property_NotAvailable) {
            return true;
        }
        if (propertyflag.isDefault && propertyflag.status == API_Property_NotEvaluated) {
            return propertyflag.definition.defaultValue.basicValue.singleVariant.variant.boolValue;
        } else {
            return propertyflag.value.singleVariant.variant.boolValue;
        }
#endif
    }
    return true;
}

// --------------------------------------------------------------------
// Запись словаря параметров для множества элементов
// --------------------------------------------------------------------
GS::Array<API_Guid> ParamHelpers::ElementsWrite (ParamDictElement &paramToWrite) {
    GS::Array<API_Guid> rereadelem = {};
    if (paramToWrite.IsEmpty ())
        return rereadelem;
#if defined(TESTING)
    DBprnt ("ElementsWrite start");
#endif
    for (ParamDictElement::PairIterator cIt = paramToWrite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamDictValue &params = cIt->value;
        API_Guid elemGuid = cIt->key;
#else
        ParamDictValue &params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
#endif
        if (ParamHelpers::Write (elemGuid, params))
            rereadelem.Push (elemGuid);
    }
#if defined(TESTING)
    if (!rereadelem.IsEmpty ())
        DBprnt ("ElementsWrite ReRead");
    DBprnt ("ElementsWrite end");
#endif
    return rereadelem;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в один элемент
// --------------------------------------------------------------------
bool ParamHelpers::Write (const API_Guid &elemGuid, ParamDictValue &params) {
    if (params.IsEmpty ())
        return false;
    if (elemGuid == APINULLGuid)
        return false;
    bool needReread = false;
    // Проходим поиском, специфичным для каждого типа
    // Для каждого типа - свой способ получения данных. Поэтому разбиваем по типам и обрабатываем по-отдельности

    GS::HashTable<short, ParamDictValue> paramByInx = {};
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (!param.isValid)
            continue;
        if (param.typeinx == 0) {
#if defined(TESTING)
            DBprnt ("ParamHelpers::Write err no inx", param.rawName);
#endif
            param.typeinx = ParamHelpers::GetTypeInxByRawnamePrefix (param.rawName);
        }
        if (param.typeinx == PROPERTYTYPEINX || param.typeinx == GDLTYPEINX || param.typeinx == IDTYPEINX ||
            param.typeinx == CLASSTYPEINX || param.typeinx == ATTRIBTYPEINX || param.typeinx == COORDTYPEINX) {
            ParamDictValue *pDictPtr = paramByInx.GetPtr (param.typeinx);
            if (pDictPtr != nullptr) {
                pDictPtr->Put (param.rawName, param);
            } else {
                ParamDictValue newDict;
                newDict.Put (param.rawName, param);
                paramByInx.Put (param.typeinx, std::move (newDict));
            }
        }
    }
    for (const short inx : paramTypesListWrite) {
        ParamDictValue *paramByTypePtr = paramByInx.GetPtr (inx);
        if (paramByTypePtr == nullptr)
            continue;
        ParamDictValue &paramByType = *paramByTypePtr;
        switch (inx) {
        case CLASSTYPEINX:
            needReread = ParamHelpers::WriteClassification (elemGuid, paramByType);
            break;
        case PROPERTYTYPEINX:
            ParamHelpers::WriteProperty (elemGuid, paramByType);
            break;
        case GDLTYPEINX:
            ParamHelpers::WriteGDL (elemGuid, paramByType);
            break;
        case IDTYPEINX:
            ParamHelpers::WriteID (elemGuid, paramByType);
            break;
        case ATTRIBTYPEINX:
            ParamHelpers::WriteAttribute (elemGuid, paramByType);
            break;
        case COORDTYPEINX:
            ParamHelpers::WriteCoord (elemGuid, paramByType);
            break;
        default:
            break;
        }
    }
    return needReread;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в автотекст
// --------------------------------------------------------------------
void ParamHelpers::WriteInfo (ParamDictElement &paramToWrite) {
    if (paramToWrite.IsEmpty ())
        return;
    clock_t start, finish;
    double duration;
    start = clock ();
    GS::HashTable<GS::UniString, GS::UniString> paramsinfo = {};
    for (auto elemIt = paramToWrite.EnumeratePairs (); elemIt != nullptr; ++elemIt) {
#if defined(AC_28) || defined(AC_29)
        ParamDictValue &params = elemIt->value;
#else
        ParamDictValue &params = *elemIt->value;
#endif
        for (auto paramIt = params.EnumeratePairs (); paramIt != nullptr; ++paramIt) {
#if defined(AC_28) || defined(AC_29)
            const ParamValue &param = paramIt->value;
#else
            const ParamValue &param = *paramIt->value;
#endif
            if (!param.fromInfo)
                continue;
            const GS::UniString &paramName = param.name;
            if (paramsinfo.ContainsKey (paramName))
                continue;
            GS::UniString value = ParamHelpers::ToString (param, param.val.formatstring);
            paramsinfo.Put (paramName, std::move (value));
        }
    }
    if (paramsinfo.IsEmpty ())
        return;
#if defined(TESTING)
    DBprnt ("WriteInfo start");
#endif
    GSErrCode err = NoError;
    for (GS::HashTable<GS::UniString, GS::UniString>::PairIterator cIt = paramsinfo.EnumeratePairs (); cIt != NULL;
         ++cIt) {
#if defined(AC_28) || defined(AC_29)
        GS::UniString dbKey = cIt->key;
        GS::UniString value = cIt->value;
#else
        GS::UniString dbKey = *cIt->key;
        GS::UniString value = *cIt->value;
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_AutoText_SetAnAutoText (&dbKey, &value);
#else
        err = ACAPI_Goodies (APIAny_SetAnAutoTextID, &dbKey, &value);
#endif
        if (err != NoError)
            msg_rep ("WriteInfo", "APIAny_SetAnAutoTextID", err, APINULLGuid);
    }
    finish = clock ();
    duration = (double)(finish - start) / CLOCKS_PER_SEC;
    GS::UniString time = GS::UniString::Printf (" %.3f s", duration);
    msg_rep ("WriteInfo", "write " + time, NoError, APINULLGuid);
}

bool ParamHelpers::WriteClassification (const API_Guid &elemGuid, ParamDictValue &params) {
    bool needReread = false;
    if (params.IsEmpty ())
        return false;
#if defined(TESTING)
    DBprnt ("    WriteClassification");
#endif
    GSErrCode err = NoError;
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (param.val.guidval != APINULLGuid) {
            err = ACAPI_Element_AddClassificationItem (elemGuid, param.val.guidval);
            if (err == NoError) {
                needReread = true;
            }
            if (err != NoError)
                msg_rep ("WriteClassification", "ACAPI_Element_AddClassificationItem", err, APINULLGuid);
        }
    }
    return needReread;
}

// --------------------------------------------------------------------
// Запись ParamDictValue в ID
// --------------------------------------------------------------------
void ParamHelpers::WriteID (const API_Guid &elemGuid, ParamDictValue &params) {
#ifdef AC_22
    msg_rep ("WriteID - ID", "Write ID not work in AC 22", NoError, elemGuid);
#else
    if (params.IsEmpty ())
        return;
    if (elemGuid == APINULLGuid)
        return;
    const auto *id = params.GetPtr (idRawname);
    if (id == nullptr) {
        return;
    }
    #if defined(TESTING)
    DBprnt ("    WriteID");
    #endif
    GS::UniString val = ParamHelpers::ToString (*id);
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Element_ChangeElementInfoString (&elemGuid, &val);
    #else
    err = ACAPI_Database (APIDb_ChangeElementInfoStringID, (void *)&elemGuid, (void *)&val);
    #endif
    if (err != NoError) {
        msg_rep ("WriteID - ID", "ACAPI_Database(APIDb_ChangeElementInfoStringID", err, elemGuid);
    }
#endif
}

// --------------------------------------------------------------------
// Запись ParamDictValue в аттрибуты элемента (слой)
// --------------------------------------------------------------------
void ParamHelpers::WriteAttribute (const API_Guid &elemGuid, ParamDictValue &params) {
    GSErrCode err = NoError;
    if (params.IsEmpty ())
        return;
#if defined(TESTING)
    DBprnt ("    WriteAttribute");
#endif
    if (elemGuid == APINULLGuid)
        return;
    const auto *p = params.GetPtr (attrlayerRawname);
    if (p == nullptr) {
#if defined(TESTING)
        DBprnt ("WriteAttribute err", "{ @attrib:layer } not found");
#endif
        return;
    }
    if (!p->isValid) {
#if defined(TESTING)
        DBprnt ("WriteAttribute err", "{ @attrib:layer } not valid");
#endif
        return;
    }
    // Поиск номера слоя по имени, если номер не найден
    API_AttributeIndex newlayer = {};
    if (!API_AttributeIndexFindByName (p->val.uniStringValue, API_LayerID, newlayer)) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Attribute_Search - " + p->val.uniStringValue, err, elemGuid);
        return;
    }
    API_Element element = {};
    API_Element elementMask = {};
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Element_Get", err, elemGuid);
        return;
    }
    if (newlayer == element.header.layer) {
#if defined(TESTING)
        DBprnt ("      WriteAttribute not need");
#endif
        return;
    }
    ACAPI_ELEMENT_MASK_CLEAR (elementMask);
    ACAPI_ELEMENT_MASK_SET (elementMask, API_Elem_Head, layer);
    element.header.layer = newlayer;
    err = ACAPI_Element_Change (&element, &elementMask, nullptr, 0, true);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteAttribute", "ACAPI_Element_Change", err, elemGuid);
        return;
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в координаты элемента
// --------------------------------------------------------------------
void ParamHelpers::WriteCoord (const API_Guid &elemGuid, ParamDictValue &params) {
    if (params.IsEmpty ())
        return;
#if defined(TESTING)
    DBprnt ("      WriteCoord");
#endif
    if (elemGuid == APINULLGuid)
        return;
    GSErrCode err = NoError;
    API_Elem_Head elem_head = {};
    API_Element element = {};
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_GetHeader", err, elem_head.guid);
        return;
    }
    API_ElemTypeID elemType = GetElemTypeID (elem_head);
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_Get", err, elem_head.guid);
        return;
    }
    API_Element mask = {};
    ACAPI_ELEMENT_MASK_CLEAR (mask);
    bool flag_write = false;
    double dval = 0;
    switch (elemType) {
    case API_WindowID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_x}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_WindowType, objLoc);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.window.objLoc = dval;
        }
        break;
    case API_DoorID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_x}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_DoorType, objLoc);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.door.objLoc = dval;
        }
        break;
    case API_ObjectID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_x}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.object.pos.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_y}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, pos);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.object.pos.y = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_z}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, level);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.object.level = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_rotangle}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ObjectType, angle);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.object.angle = dval * PI / 180.0;
        }
        break;
    case API_ColumnID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_x}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, origoPos);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.column.origoPos.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_y}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, origoPos);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.column.origoPos.y = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_rotangle}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_ColumnType, slantAngle);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.column.slantAngle = dval * PI / 180.0;
        }
        break;
    case API_WallID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_sx}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.wall.begC.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_sy}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, begC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.wall.begC.y = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_ex}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.wall.endC.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_ey}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_WallType, endC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.wall.endC.y = dval;
        }
        break;
    case API_BeamID:
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_sx}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.beam.begC.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_sy}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, begC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.beam.begC.y = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_ex}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.beam.endC.x = dval;
        }
        if (const auto *pval = params.GetPtr ("{@coord:symb_pos_ey}")) {
            flag_write = true;
            ACAPI_ELEMENT_MASK_SET (mask, API_BeamType, endC);
            if (pval->val.formatstring.forceRaw) {
                dval = pval->val.rawDoubleValue;
            } else {
                dval = pval->val.doubleValue;
            }
            element.beam.endC.y = dval;
        }
        break;
    default:
#if defined(TESTING)
        DBprnt ("WriteCoord err", "wrong type element");
#endif
        break;
    }
    if (flag_write) {
        err = ACAPI_Element_Change (&element, &mask, nullptr, 0, true);
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteCoord", "ACAPI_Element_Change", err, elem_head.guid);
            return;
        }
    } else {
#if defined(TESTING)
        DBprnt ("      WriteCoord no data");
#endif
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в GDL параметры
// --------------------------------------------------------------------
void ParamHelpers::WriteGDL (const API_Guid &elemGuid, ParamDictValue &params) {
    if (params.IsEmpty ())
        return;
#if defined(TESTING)
    DBprnt ("    WriteGDL\n");
#endif
    if (elemGuid == APINULLGuid)
        return;
    API_Elem_Head elem_head = {};
    API_Element element = {};
    API_ElemTypeID elemType;
    API_Guid elemGuidt;
    API_ParamOwnerType apiOwner = {};
    API_GetParamsType apiParams = {};
    API_ChangeParamType chgParam;
    elem_head.guid = elemGuid;
    GSErrCode err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_GetHeader", err, elem_head.guid);
        return;
    }
    element.header.guid = elemGuid;
    err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_Get", err, elem_head.guid);
        return;
    }
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    GetGDLParametersHead (element, elem_head, elemType, elemGuidt);
    apiOwner.guid = elemGuidt;
#if defined(AC_26) || defined(AC_27) || defined(AC_28) || defined(AC_29)
    apiOwner.type.typeID = elemType;
#else
    apiOwner.typeID = elemType;
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryPart_OpenParameters (&apiOwner);
#else
    err = ACAPI_Goodies (APIAny_OpenParametersID, &apiOwner, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_OpenParametersID", err, elem_head.guid);
        return;
    }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
#else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
#endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_GetActParametersID", err, elem_head.guid);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_LibraryPart_CloseParameters ();
#else
        err = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);
            return;
        }
    }
    bool flagFind = false;
    Int32 addParNum = BMGetHandleSize ((GSHandle)apiParams.params) / sizeof (API_AddParType);
    Int32 nfind = params.GetSize ();
    constexpr USize MaxStrValueLength = 512;
    for (Int32 i = 0; i < addParNum; ++i) {
        API_AddParType &actualParam = (*apiParams.params)[i];
        if (actualParam.typeMod != API_ParSimple)
            continue; // TODO Добавить замись массивов
        GS::UniString rawname = GetGDLRawName (actualParam.name);
        const auto *pValuePtr = params.GetPtr (rawname);
        if (pValuePtr == nullptr)
            continue;

        BNZeroMemory (&chgParam, sizeof (API_ChangeParamType));
        chgParam.index = actualParam.index;
        CHTruncate (actualParam.name, chgParam.name, API_NameLen);
        API_AttrTypeID type = API_ZombieAttrID;
        switch (actualParam.typeID) {
        case APIParT_LineTyp:
            type = API_LinetypeID;
            break;

        case APIParT_Profile:
            type = API_ProfileID;
            break;

        case APIParT_BuildingMaterial:
            type = API_BuildingMaterialID;
            break;

        case APIParT_FillPat:
            type = API_FilltypeID;
            break;

        case APIParT_Mater:
            type = API_MaterialID;
            break;

        case APIParT_CString:
            static GS::uchar_t strValuePtr[MaxStrValueLength];
            GS::ucscpy (strValuePtr,
                        pValuePtr->val.uniStringValue
                            .ToUStr (0, GS::Min (pValuePtr->val.uniStringValue.GetLength (), (USize)MaxStrValueLength))
                            .Get ());
            chgParam.uStrValue = strValuePtr;
            break;

        case APIParT_Integer:
            chgParam.realValue = pValuePtr->val.intValue;
            break;

        case APIParT_PenCol:
            if (pValuePtr->val.intValue > 0 && pValuePtr->val.intValue < 255)
                chgParam.realValue = pValuePtr->val.intValue;
            break;

        case APIParT_Length:
            if (pValuePtr->val.formatstring.forceRaw) {
                chgParam.realValue = pValuePtr->val.rawDoubleValue;
            } else {
                chgParam.realValue = pValuePtr->val.doubleValue;
            }
            break;

        case APIParT_Angle:
            if (pValuePtr->val.formatstring.forceRaw) {
                chgParam.realValue = pValuePtr->val.rawDoubleValue;
            } else {
                chgParam.realValue = pValuePtr->val.doubleValue;
            }
            break;

        case APIParT_RealNum:
        case APIParT_ColRGB:
        case APIParT_Intens:
            if (pValuePtr->val.formatstring.forceRaw) {
                chgParam.realValue = pValuePtr->val.rawDoubleValue;
            } else {
                chgParam.realValue = pValuePtr->val.doubleValue;
            }
            break;

        case APIParT_Boolean:
            if (pValuePtr->val.boolValue) {
                chgParam.realValue = 1;
            } else {
                chgParam.realValue = 0;
            }
            break;
        default:
            break;
        }
        if (type != API_ZombieAttrID) {
            Int32 attribinxint = 0;
            if (pValuePtr->val.type == API_PropertyStringValueType) {
                // Поиск индекса аттрибута при необходимости
                API_AttributeIndex attribinx;
                if (API_AttributeIndexFindByName (pValuePtr->val.uniStringValue, type, attribinx)) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
                    attribinxint = attribinx.ToInt32_Deprecated ();
#else
                    attribinxint = attribinx;
#endif
                } else {
                    attribinxint = pValuePtr->val.intValue;
                }
            } else {
                attribinxint = pValuePtr->val.intValue;
            }
            if (attribinxint > 0)
                chgParam.realValue = attribinxint;
        }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_LibraryPart_ChangeAParameter (&chgParam);
#else
        err = ACAPI_Goodies (APIAny_ChangeAParameterID, &chgParam, nullptr);
#endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_ChangeAParameterID", err, elem_head.guid);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_LibraryPart_CloseParameters ();
#else
            err = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
            if (err != NoError)
                msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);
            return;
        }
    }
    ACAPI_DisposeAddParHdl (&apiParams.params);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
#else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
#endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_GetActParametersID", err, elem_head.guid);
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_LibraryPart_CloseParameters ();
#else
        err = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
        if (err != NoError)
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);
        return;
    }
    API_ElementMemo elemMemo = {};
    elemMemo.params = apiParams.params;
    err = ACAPI_Element_ChangeMemo (elemGuidt, APIMemoMask_AddPars, &elemMemo);
    if (err != NoError)
        msg_rep ("ParamHelpers::WriteGDL", "ACAPI_Element_ChangeMemo", err, elem_head.guid);

#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    GSErrCode err_ = ACAPI_LibraryPart_CloseParameters ();
#else
    GSErrCode err_ = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
    if (err_ != NoError)
        msg_rep ("ParamHelpers::WriteGDL", "APIAny_CloseParametersID", err, elem_head.guid);

    ACAPI_DisposeAddParHdl (&apiParams.params);
    if (err == NoError) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_LibraryManagement_RunGDLParScript (&elem_head, 0);
#else
        err = ACAPI_Goodies (APIAny_RunGDLParScriptID, &elem_head, 0);
#endif
        if (err != NoError) {
            msg_rep ("ParamHelpers::WriteGDL", "APIAny_RunGDLParScriptID", err, elemGuid);
            return;
        }
    }
}

// --------------------------------------------------------------------
// Запись ParamDictValue в свойства
// --------------------------------------------------------------------
void ParamHelpers::WriteProperty (const API_Guid &elemGuid, ParamDictValue &params) {
    if (elemGuid == APINULLGuid) {
#if defined(TESTING)
        DBprnt ("    WriteProperty err", "elemGuid == APINULLGuid");
#endif
        return;
    }
    GSErrCode error = NoError;
#if defined(TESTING)
    DBprnt ("    WriteProperty");
#endif
    // Если для свойств известно только определение, но не было получено свойство - самое время это сделать
    GS::Array<API_PropertyDefinition> propertyDefinitions = {};
    GS::Array<API_Property> property2write;
    property2write.SetCapacity (params.GetSize ());
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (param.property.definition.guid != APINULLGuid) {
            // Если у параметра есть сохранённое свойство - сразу записываем его
            API_Property property = param.property;
            if (!ParamHelpers::ConvertToProperty (param, property)) { // Конвертируем параметр в свойство
#if defined(TESTING)
                DBprnt ("    WriteProperty err", "!ConvertToProperty " + param.rawName);
#endif
                continue;
            }
            property2write.Push (std::move (property));
            continue;
        }
        // Если нет гуида определения свойства - поищем его в кэше
        if (param.definition.guid == APINULLGuid) {
            if (const auto *ptr = PROPERTYCACHE ().property.GetPtr (param.rawName)) {
                // Если у параметра нет свойства, но есть определение - добавим его определение для поиска
                if (ptr->definition.guid != APINULLGuid) {
                    propertyDefinitions.Push (ptr->definition);
#if defined(TESTING)
                } else {
                    DBprnt ("    WriteProperty err", "param.definition.guid == APINULLGuid " + param.rawName);
#endif
                }
#if defined(TESTING)
            } else {
                DBprnt ("    WriteProperty err", "param.definition.guid == APINULLGuid " + param.rawName);
#endif
            }
        }
    }
    if (!propertyDefinitions.IsEmpty ()) {
// Поиск свойств по их определению и повторная запись
#if defined(TESTING)
        DBprnt ("    WriteProperty", "!propertyDefinitions.IsEmpty()");
#endif
        GS::Array<API_Property> properties = {};
        error = ACAPI_Element_GetPropertyValues (elemGuid, propertyDefinitions, properties);
        if (error != NoError) {
            msg_rep ("WriteProperty", "ACAPI_Element_GetPropertyValues", error, elemGuid);
            return;
        }
        GS::UniString fname = "";
        GS::UniString rawName = "";
        for (auto &property : properties) {
            GetPropertyFullName (property.definition, fname);
            rawName = PROPERTYNAMEPREFIX + fname.ToLowerCase () + BRACEEND;
            const auto *paramPtr = params.GetPtr (rawName);
            if (paramPtr == nullptr) {
#if defined(TESTING)
                DBprnt ("    WriteProperty err", "params.ContainsKey (rawName) " + rawName);
#endif
                continue;
            }
            if (!ParamHelpers::ConvertToProperty (*paramPtr, property)) { // Конвертируем параметр в свойство
// Конвертация выдаёт ложь при ошибке или если значения свойства и параметра совпадают
#if defined(TESTING)
                DBprnt ("    WriteProperty err", "!ConvertToProperty " + rawName);
#endif
                continue;
            }
            property2write.Push (std::move (property));
        }
    }
    error = ACAPI_Element_SetProperties (elemGuid, property2write);
    if (error != NoError) {
        msg_rep ("WriteProperty err", "ACAPI_Element_SetProperty", error, elemGuid);
    }
}

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamHelpers::ElementsRead (ParamDictElement &paramToRead) {
    ParamDictCompositeElement paramCompositeToRead = {};
    ListData::LibElements paramListDataToRead = {};
    ParamHelpers::ElementsRead (paramToRead, paramCompositeToRead, paramListDataToRead, false, false);
}

void ParamHelpers::Read (const API_Guid &elemGuid, ParamDictValue &params) {
    ParamDictComposite paramCompositeToRead = {};
    ListData::LibElement paramListDataToRead = {};
    ParamHelpers::Read (elemGuid, params, paramCompositeToRead, paramListDataToRead, false, false);
}

// --------------------------------------------------------------------
// Заполнение словаря параметров для множества элементов
// --------------------------------------------------------------------
void ParamHelpers::ElementsRead (ParamDictElement &paramToRead,
                                 ParamDictCompositeElement &paramCompositeToRead,
                                 ListData::LibElements &paramListDataToRead,
                                 bool needReturnComposite,
                                 bool needListData) {
    if (paramToRead.IsEmpty ())
        return;
#if defined(TESTING)
    DBprnt ("ElementsRead start");
#endif
    // Выбираем по-элементно параметры для чтения
    for (ParamDictElement::PairIterator cIt = paramToRead.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamDictValue &params = cIt->value;
        API_Guid elemGuid = cIt->key;
#else
        ParamDictValue &params = *cIt->value;
        API_Guid elemGuid = *cIt->key;
#endif
        if (params.IsEmpty ())
            continue;
        const API_Guid elemGuid_comp = needReturnComposite ? elemGuid : APINULLGuid;
        ParamDictComposite *compPtr = paramCompositeToRead.GetPtr (elemGuid_comp);
        if (compPtr == nullptr) {
            // Вставляем пустой контейнер только если ключа вообще не было
            paramCompositeToRead.Put (elemGuid_comp, ParamDictComposite{});
            compPtr = paramCompositeToRead.GetPtr (elemGuid_comp);
        }
        ParamDictComposite &paramcomposite = *compPtr;
        const API_Guid elemGuid_list = needListData ? elemGuid : APINULLGuid;
        ListData::LibElement *listPtr = paramListDataToRead.GetPtr (elemGuid_list);
        if (listPtr == nullptr) {
            paramListDataToRead.Put (elemGuid_list, ListData::LibElement{});
            listPtr = paramListDataToRead.GetPtr (elemGuid_list);
        }
        ListData::LibElement &paramListData = *listPtr;
        ParamHelpers::Read (elemGuid, params, paramcomposite, paramListData, needReturnComposite, needListData);
    }
#if defined(TESTING)
    DBprnt ("ElementsRead end");
#endif
}

// --------------------------------------------------------------------
// Заполнение словаря с параметрами
// --------------------------------------------------------------------
void ParamHelpers::Read (const API_Guid &elemGuid,
                         ParamDictValue &params,
                         ParamDictComposite &paramcomposite,
                         ListData::LibElement &paramListData,
                         bool needReturnComposite,
                         bool needListData) {
    if (params.IsEmpty ())
        return;
    if (elemGuid == APINULLGuid) {
        msg_rep ("ParamDictRead", "elemGuid == APINULLGuid", APIERR_GENERAL, elemGuid);
        return;
    }
    API_Elem_Head elem_head = {};
    elem_head.guid = elemGuid;
    GSErrCode err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("ParamDictRead", "ACAPI_Element_GetHeader", err, elemGuid);
        return;
    }
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    bool can_read_fromMaterial = true;
    if (eltype != API_WallID && eltype != API_MeshID && eltype != API_SlabID && eltype != API_ColumnID &&
        eltype != API_BeamID && eltype != API_RoofID && eltype != API_BeamSegmentID && eltype != API_ColumnSegmentID &&
        eltype != API_ShellID)
        can_read_fromMaterial = false;
    bool can_read_fromGDL = !can_read_fromMaterial;
    if (eltype == API_MorphID)
        can_read_fromGDL = false;
    // Для некоторых типов элементов есть общая информация, которая может потребоваться
    // Пройдём по параметрам и посмотрим - что нам нужно заранее прочитать
    bool needGetElement = false;
    bool hasListData = false;
    bool hasQuantity = false;
    GS::HashTable<short, bool> hasparambytypes = {}; // Словарь наличия параметров для чтения по типу параметра
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (param.fromGuid == APINULLGuid)
            param.fromGuid = elemGuid;
        if (param.fromGuid == APINULLGuid)
            continue;
        if (param.fromGuid != elemGuid)
            continue;

        if (param.typeinx == 0) {
            param.typeinx = ParamHelpers::GetTypeInxByRawnamePrefix (param.rawName);
        }
        if (param.typeinx == PROPERTYTYPEINX || param.typeinx == GLOBTYPEINX || param.typeinx == ATTRIBTYPEINX ||
            param.typeinx == INFOTYPEINX) {
            ParamHelpers::SetParamValueFromCache (param.rawName, param);
        }
        if (param.fromQuantity)
            hasQuantity = true;
        if (param.fromListData)
            hasListData = true;
        // Когда нужно получить весь элемент
        if (param.fromGDLdescription && can_read_fromGDL)
            needGetElement = true;
        if (param.fromElement || param.fromCoord || (param.fromMorph && eltype == API_MorphID) ||
            param.fromAttribDefinition)
            needGetElement = true;
        if (can_read_fromMaterial && param.fromMaterial)
            needGetElement = true;
        if (eltype == API_CurtainWallPanelID || eltype == API_CurtainWallFrameID ||
            eltype == API_CurtainWallJunctionID || eltype == API_CurtainWallAccessoryID ||
            eltype == API_RailingToprailID || eltype == API_RailingHandrailID || eltype == API_RailingRailID ||
            eltype == API_RailingPostID || eltype == API_RailingInnerPostID || eltype == API_RailingBalusterID ||
            eltype == API_RailingPanelID || eltype == API_RailingNodeID || eltype == API_RailingToprailEndID ||
            eltype == API_RailingHandrailEndID || eltype == API_RailingRailEndID ||
            eltype == API_RailingToprailConnectionID || eltype == API_RailingHandrailConnectionID ||
            eltype == API_RailingRailConnectionID || eltype == API_RailingEndFinishID) {
            needGetElement = true;
        }
        hasparambytypes.Put (param.typeinx, true);
        if (param.val.hasFormula)
            hasparambytypes.Put (FORMULATYPEINX, true);
    }
    if (hasQuantity && can_read_fromGDL) {
        can_read_fromMaterial = true;
        // Даже если читать компоненты и дескрипторы не нужно - там может находится указание на толщину материала
        if (!hasListData) {
            GS::UniString k = "@listdata:some_stuff_th";
            ParamHelpers::AddValueToParamDictValue (params, k);
            if (ParamValue *pPtr = params.GetPtr (k)) {
                if (pPtr->fromGuid == APINULLGuid) {
                    pPtr->fromGuid = elemGuid;
                }
            }
            hasparambytypes.Put (LISTDATATYPEINX, true);
        }
    }
    if (needListData && can_read_fromGDL) {
        hasListData = true;
        hasparambytypes.Put (LISTDATATYPEINX, true);
    }
    API_Element element = {};
    if (needGetElement) {
        element.header.guid = elemGuid;
        err = ACAPI_Element_Get (&element);
        if (err != NoError) {
            msg_rep ("ParamDictRead", "ACAPI_Element_Get", err, elem_head.guid);
            return;
        }
    } else {
        UNUSED_VARIABLE (element);
    }
    ParamDictValue paramByType = {};
    GS::Array<API_PropertyDefinition> propertyDefinitions = {};
    for (const short inx : paramTypesList) {
        if (!hasparambytypes.ContainsKey (inx))
            continue;
        if (inx == GDLTYPEINX && !can_read_fromGDL)
            continue;
        if (inx == LISTDATATYPEINX && !can_read_fromGDL)
            continue;
        if (inx == MORPHTYPEINX && eltype != API_MorphID)
            continue;
        if (inx == MATERIALTYPEINX && !can_read_fromMaterial)
            continue;
        if (inx == PROPERTYTYPEINX || inx == GDLTYPEINX || inx == CLASSTYPEINX || inx == IFCTYPEINX) {
            paramByType.Clear ();
            // Для некоторых параметров выборка не требуется
            for (GS::HashTable<GS::UniString, ParamValue>::PairIterator cIt = params.EnumeratePairs (); cIt != NULL;
                 ++cIt) {
#if defined(AC_28) || defined(AC_29)
                ParamValue &param = cIt->value;
#else
                ParamValue &param = *cIt->value;
#endif
                // Выбираем элементы с подходящим Guid
                if (param.fromGuid != elemGuid)
                    continue;
                // Для свойств с формулами основной признак - флаг, т.к. вычислению подлежат и свойства с составом
                // конструкций.
                if (param.typeinx != inx)
                    continue;
                if (inx == PROPERTYTYPEINX) {
                    if (param.fromPropertyDefinition && param.definition.guid != APINULLGuid) {
                        propertyDefinitions.Push (param.definition);
                    }
                } else {
                    if (inx == GDLTYPEINX) {
                        if (needGetElement) {
                            bool b = IsUnreadGDLParams (element.object.libInd, param.rawName);
                            if (!b) {
                                paramByType.Put (param.rawName, param);
#if defined(TESTING)
                            } else {
                                DBprnt ("        Skip unread param", param.rawName);
#endif
                            }
                        } else {
                            paramByType.Put (param.rawName, param);
                        }
                    } else {
                        paramByType.Put (param.rawName, param);
                    }
                }
            }
        }
        bool needCompare = false; // Флаг необходимости записи в словарь.
        switch (inx) {
        case PROPERTYTYPEINX:
            ParamHelpers::ReadProperty (elemGuid, params, propertyDefinitions);
            propertyDefinitions.Clear ();
            break;
        case COORDTYPEINX:
            ParamHelpers::ReadCoords (element, params);
            break;
        case GDLTYPEINX:
            ParamHelpers::ReadGDL (element, elem_head, paramByType, params);
            break;
        case LISTDATATYPEINX:
            ParamHelpers::ReadListData (elem_head, params, paramListData, needListData);
            break;
        case IFCTYPEINX:
#if !defined(AC_29)
            needCompare = ParamHelpers::ReadIFC (elemGuid, paramByType);
#endif
            break;
        case MORPHTYPEINX:
            if (element.header.hasMemo)
                ParamHelpers::ReadMorphParam (elemGuid, params);
            break;
        case IDTYPEINX:
            ParamHelpers::ReadID (elem_head, params);
            break;
        case CLASSTYPEINX:
            needCompare = ParamHelpers::ReadClassification (elemGuid, paramByType);
#if defined(TESTING)
            if (!needCompare) {
                DBprnt ("ParamHelpers::ReadClassification ERROR");
            }
#endif
            break;
        case MATERIALTYPEINX:
            ParamHelpers::ReadMaterial (element, params, paramcomposite, hasQuantity);
            break;
        case FORMULATYPEINX:
            ParamHelpers::ReadFormula (params, false);
            break;
        case ATTRIBTYPEINX:
            ParamHelpers::ReadAttributeValues (elem_head, params);
            break;
        case ELEMENTTYPEINX:
            ParamHelpers::ReadElementValues (element, params);
            break;
        case MEPTYPEINX:
#if defined(AC_28) || defined(AC_29)
            MEPv1::ReadMEP (elem_head, params);
#endif
            break;
        case FILETYPEINX:
            ReadFile (params);
            break;
        default:
            break;
        }
        if (needCompare) {
#if defined(TESTING)
            DBprnt ("        CompareParamDictValue");
#endif
            ParamHelpers::CompareParamDictValue (paramByType, params);
        }
    }
#if defined(TESTING)
    DBprnt ("        ConvertByFormatString");
#endif
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (!param.isValid)
            continue;
        if (param.val.canCalculate && !param.val.hasFormula) {
            ParamHelpers::ConvertByFormatString (param);
        }
        if (param.fromPropertyDefinition) {
            if (param.definition.description.Contains ("Sync_to{Attribute:Layer}")) {
                GS::UniString key = "{@attrib:layer_name_" + param.val.uniStringValue.ToLowerCase () + BRACEEND;
                ParamValue chacheval = {};
                if (ParamHelpers::GetParamValueFromCache (key, chacheval)) {
                    param.val = chacheval.val;
                    param.isValid = true;
                } else {
                    param.isValid = false;
                }
                param.fromAttribElement = true;
            }
        }
        if (param.toQRCode) {
            GS::UniString qr = TextToQRCode (param.val.uniStringValue);
            param.val.uniStringValue = qr;
        }
    }
    if (needListData) {
        paramListData.keys = ListData::GetAllKeys (paramListData);
    }
}

// --------------------------------------------------------------------
// Получение массива описаний свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetDefinition (const GS::Array<API_PropertyDefinition> &definitions,
                                          GS::Array<API_PropertyDefinition> &definitionsout) {
    if (definitions.IsEmpty ())
        return false;
    bool flag_find = false;
    for (auto &definition : definitions) {
        if (definition.description.IsEmpty ())
            continue;
        if (!definition.description.Contains (SYNCGUID))
            continue;
        definitionsout.Push (definition);
        flag_find = true;
    }
    return flag_find;
}

// --------------------------------------------------------------------
// Получение словаря значений свойств с указанием GUID родительского объекта
// --------------------------------------------------------------------
bool ParamHelpers::SubGuid_GetParamValue (const API_Guid &elemGuid,
                                          const GS::Array<API_PropertyDefinition> &definitions,
                                          ParamDictValue &subproperty) {
    if (definitions.IsEmpty ())
        return false;
    GS::Array<API_PropertyDefinition> subdefinitions = {};
    GS::Array<API_Property> properties = {};
    if (!SubGuid_GetDefinition (definitions, subdefinitions))
        return false;
    auto &cache = PROPERTYCACHE ();
    if (!(cache.isPropertyDefinitionRead_full && cache.isPropertyDefinition_OK)) {
        cache.AddPropertyDefinition (subdefinitions);
    }
    GSErrCode error = ACAPI_Element_GetPropertyValues (elemGuid, subdefinitions, properties);
    if (error != NoError) {
        msg_rep ("SubGuid_GetParamValue", "ACAPI_Element_GetPropertyValues", error, elemGuid);
        return false;
    }
    bool flag_add = false;
    for (const auto &property : properties) {
        ParamValue pvalue = {};
        if (!ParamHelpers::ConvertToParamValue (pvalue, property)) {
#if defined(TESTING)
            DBprnt ("AddProperty err convert to pvalue", property.definition.name);
#endif
            continue;
        }
        if (elemGuid != APINULLGuid)
            pvalue.fromGuid = elemGuid;
        if (!subproperty.ContainsKey (pvalue.rawName))
            subproperty.Put (pvalue.rawName, std::move (pvalue));
        flag_add = true;
    }
#if defined(TESTING)
    if (!flag_add) {
        DBprnt ("SubGuid_GetParamValue not found");
    } else {
        DBprnt ("SubGuid_GetParamValue FOUND");
    }
#endif
    return flag_add;
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictElement
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictElement (ParamDictElement &paramsFrom, ParamDictElement &paramsTo) {
    if (paramsFrom.IsEmpty () || paramsTo.IsEmpty ())
        return;
    for (auto &cIt : paramsTo) {
#if defined(AC_28) || defined(AC_29)
        ParamDictValue &paramTo = cIt.value;
        API_Guid elemGuid = cIt.key;
#else
        ParamDictValue &paramTo = *cIt.value;
        API_Guid elemGuid = *cIt.key;
#endif
        if (ParamDictValue *pfromPtr = paramsFrom.GetPtr (elemGuid))
            ParamHelpers::CompareParamDictValue (*pfromPtr, paramTo);
    }
}

// --------------------------------------------------------------------
// Сопоставление двух словарей ParamDictValue
// --------------------------------------------------------------------
void ParamHelpers::CompareParamDictValue (ParamDictValue &paramsFrom, ParamDictValue &paramsTo) {
    if (paramsFrom.IsEmpty () || paramsTo.IsEmpty ())
        return;

    for (auto &cIt : paramsFrom) {
#if defined(AC_28) || defined(AC_29)
        const GS::UniString &k = cIt.key;
        ParamValue &paramFrom = cIt.value;
#else
        const GS::UniString &k = *cIt.key;
        ParamValue &paramFrom = *cIt.value;
#endif
        if (ParamValue *paramToPtr = paramsTo.GetPtr (k)) {
            API_Guid savedGuid = paramToPtr->fromGuid;
            auto savedQRCode = paramToPtr->toQRCode;

            *paramToPtr = paramFrom;

            paramToPtr->fromGuid = savedGuid;
            paramToPtr->toQRCode = savedQRCode;
        }
    }
}

// --------------------------------------------------------------------
// Чтение значений свойств в ParamDictValue
// --------------------------------------------------------------------
bool ParamHelpers::ReadProperty (const API_Guid &elemGuid,
                                 ParamDictValue &params,
                                 const GS::Array<API_PropertyDefinition> &propertyDefinitions) {
    if (params.IsEmpty ())
        return false;
#if defined(TESTING)
    DBprnt ("      ReadProperty");
#endif
    if (propertyDefinitions.IsEmpty ())
        return false;
    GS::Array<API_Property> properties = {};
    GSErrCode error = ACAPI_Element_GetPropertyValues (elemGuid, propertyDefinitions, properties);
    if (error != NoError) {
        msg_rep ("ParamDictGetPropertyValues", "ACAPI_Element_GetPropertyValues", error, elemGuid);
        return false;
    }
    return (ParamHelpers::AddProperty (params, properties, elemGuid));

#if defined(TESTING)
    DBprnt ("ReadProperty err", "no property");
#endif
    return false;
}

// -----------------------------------------------------------------------------
// Получение значения IFC свойств
// -----------------------------------------------------------------------------
#if !defined(AC_29)
bool ParamHelpers::ReadIFC (const API_Guid &elemGuid, ParamDictValue &params) {
    if (params.IsEmpty ())
        return false;
    #if defined(TESTING)
    DBprnt ("      ReadIFC");
    #endif
    GS::Array<API_IFCProperty> properties = {};
    GSErrCode err = ACAPI_Element_GetIFCProperties (elemGuid, false, &properties);
    if (err != NoError) {
        msg_rep ("ParamDictGetIFCValues", "ACAPI_Element_GetIFCProperties", err, elemGuid);
        return false;
    }
    bool flag_find = false;
    UInt32 nparams = params.GetSize ();
    GS::UniString fname = "";
    GS::UniString rawName = "";
    for (UInt32 i = 0; i < properties.GetSize (); i++) {
        API_IFCProperty property = properties.Get (i);
        fname = properties.Get (i).head.propertySetName;
        fname.Append (SLASH);
        fname.Append (properties.Get (i).head.propertyName);

        rawName = IFCNAMEPREFIX;
        rawName.Append (fname.ToLowerCase ());
        rawName.Append (BRACEEND);
        if (params.ContainsKey (rawName)) {
            ParamValue pvalue;
            if (ParamHelpers::ConvertToParamValue (pvalue, property)) {
                params.Get (rawName) = pvalue;
                flag_find = true;
            }
            nparams--;
            if (nparams == 0) {
                return flag_find;
            }
        } else {
            fname = properties[i].head.propertyName;
            rawName = IFCNAMEPREFIX;
            rawName.Append (fname.ToLowerCase ());
            rawName.Append (BRACEEND);
            if (params.ContainsKey (rawName)) {
                ParamValue pvalue = {};
                if (ParamHelpers::ConvertToParamValue (pvalue, property)) {
                    params.Get (rawName) = pvalue;
                    flag_find = true;
                }
                nparams--;
                if (nparams == 0) {
                    return flag_find;
                }
            }
        }
    }
    return flag_find;
}
#endif

// -----------------------------------------------------------------------------
// Обработка данных о классификации
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadClassification (const API_Guid &elemGuid, ParamDictValue &paramByType) {
#if defined(TESTING)
    DBprnt ("      ReadClassification");
#endif
    if (!ClassificationFunc::ReadSystemDict ())
        return false;
    ClassificationFunc::SystemDict &systemdict = PROPERTYCACHE ().systemdict;
    GSErrCode err = NoError;
    GS::HashTable<GS::UniString, API_Guid> elementsystem = {};
    for (ParamDictValue::PairIterator cIt = paramByType.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (systemdict.ContainsKey (param.name)) {
            ClassificationFunc::ClassificationDict &c = systemdict.Get (param.name);
            if (c.ContainsKey ("@system@")) {
                API_Guid systemguid = c.Get ("@system@").system.guid;
                elementsystem.Put (param.name, systemguid);
            }
        } else {
            param.val.uniStringValue = ""; // Если система не найдена - обнулим значение
            msg_rep ("System not found", param.name, NoError, APINULLGuid);
        }
    }
    if (elementsystem.IsEmpty ())
        return false;
    bool flag_find = false;
    for (GS::HashTable<GS::UniString, API_Guid>::PairIterator cIt = elementsystem.EnumeratePairs (); cIt != NULL;
         ++cIt) {
#if defined(AC_28) || defined(AC_29)
        API_Guid systemguid = cIt->value;
        GS::UniString systemname = cIt->key;
#else
        API_Guid systemguid = *cIt->value;
        GS::UniString systemname = *cIt->key;
#endif
        API_ClassificationItem item = {};
        err = ACAPI_Element_GetClassificationInSystem (elemGuid, systemguid, item);
        if (err == NoError) {
            GS::UniString rawname = CLASSNAMEPREFIX + systemname + ";fullname}";
            if (paramByType.ContainsKey (rawname)) {
                ParamValue &p = paramByType.Get (rawname);
                GS::UniString fullname = "";
                ClassificationFunc::GetFullName (item, systemdict.Get (systemname), fullname);
                p.isValid = true;
                p.val.uniStringValue = fullname;
                p.val.type = API_PropertyStringValueType;
                flag_find = true;
            }
            rawname = CLASSNAMEPREFIX + systemname + BRACEEND;
            if (paramByType.ContainsKey (rawname)) {
                ParamValue &p = paramByType.Get (rawname);
                p.isValid = true;
                p.val.guidval = item.guid;
                GS::UniString fullname = "";
                ClassificationFunc::GetFullName (item, systemdict.Get (systemname), fullname);
                p.val.uniStringValue = fullname;
                p.val.type = API_PropertyGuidValueType;
                flag_find = true;
            }
        } else {
            msg_rep ("ReadClassification", "ACAPI_Element_GetClassificationInSystem", err, systemguid);
        }
    }
    return flag_find;
}

// -----------------------------------------------------------------------------
// Получение аттрибутов элемента
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadAttributeValues (const API_Elem_Head &elem_head, ParamDictValue &params) {
    if (params.IsEmpty ())
        return false;
#if defined(TESTING)
    DBprnt ("      ReadAttributeValues");
#endif

    auto *p = params.GetPtr (attrlayerRawname);
    if (p == nullptr)
        return false;
    p->isValid = true;
    p->fromAttribElement = true;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    GS::Int32 intValue = elem_head.layer.ToInt32_Deprecated ();
#else
    GS::Int32 intValue = elem_head.layer;
#endif
    GS::UniString cname = "{@attrib:layer_inx_" + GS::UniString::Printf ("%d", intValue) + BRACEEND;
    ParamValue cacheparam;
    if (GetParamValueFromCache (cname, cacheparam)) {
        p->val = cacheparam.val;
        return true;
    } else {
        msg_rep ("ParamHelpers::ReadAttributeValues", "Layer not found - " + cname, NoError, elem_head.guid);
    }

    API_Attribute attrib = {};
    GS::UniString name = "";
    attrib.header.typeID = API_LayerID;
    attrib.header.index = elem_head.layer;
    GSErrCode error = ACAPI_Attribute_Get (&attrib);
    if (error != NoError) {
        msg_rep ("ParamHelpers::ReadAttributeValues", "ACAPI_Attribute_Get", error, elem_head.guid);
        return false;
    };
    ParamValue pvalue = {};
    ParamHelpers::ConvertAttributeToParamValue (pvalue, "layer", attrib);
    p->val = pvalue.val;
    return true;
}

// -----------------------------------------------------------------------------
// Получение ID элемента
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadID (const API_Elem_Head &elem_head, ParamDictValue &params) {
    if (params.IsEmpty ())
        return false;
#if defined(TESTING)
    DBprnt ("      ReadID");
#endif
    auto *id = params.GetPtr (idRawname);
    if (id == nullptr)
        return false;
    GS::UniString infoString = "";
    API_Guid elguid = elem_head.guid;
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Element_GetElementInfoString (&elguid, &infoString);
#else
    err = ACAPI_Database (APIDb_GetElementInfoStringID, &elguid, &infoString);
#endif
    if (err != NoError) {
        msg_rep ("ReadID - ID", "ACAPI_Database(APIDb_GetElementInfoStringID", err, elguid);
        return false;
    } else {
        id->isValid = true;
        id->val.type = API_PropertyStringValueType;
        id->type = API_PropertyStringValueType;
        id->val.boolValue = !infoString.IsEmpty ();
        if (UniStringToDouble (infoString, id->val.doubleValue)) {
            id->val.intValue = (GS::Int32)id->val.doubleValue;
            id->val.canCalculate = true;
        } else {
            id->val.intValue = !infoString.IsEmpty ();
            id->val.doubleValue = id->val.intValue * 1.0;
        }
        id->val.rawDoubleValue = id->val.doubleValue;
        id->val.hasrawDouble = true;
        id->val.uniStringValue = infoString;
        return true;
    }
}

// -----------------------------------------------------------------------------
// Получить значение GDL параметра по его имени или описанию в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadGDL (const API_Element &element,
                            const API_Elem_Head &elem_head,
                            ParamDictValue &params,
                            ParamDictValue &allparams) {
    if (params.IsEmpty ())
        return false;
#if defined(TESTING)
    DBprnt ("      ReadGDL");
#endif
    API_ElemTypeID eltype = GetElemTypeID (elem_head);
    // Обрабатываем только вложенные элементы иерархических структур (навесных стен и ограждений)
    if (eltype == API_RailingID || eltype == API_CurtainWallID) {
        return false;
    }
    ParamDictValue paramBydescription = {};
    ParamDictValue paramByName = {};
    GS::HashTable<GS::UniString, GS::Array<GS::UniString>> paramnamearray = {};

    // Если диапазоны массивов хранятся в параметра х - прочитаем сначала их
    ParamDictValue paramdiap = {};
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (param.needPreRead) {
            if (!param.rawName_row_start.IsEmpty ()) {
                ParamValue arr_row_start = {};
                arr_row_start.fromGDLparam = true;
                arr_row_start.rawName = param.rawName_row_start;
                paramdiap.Put (arr_row_start.rawName, arr_row_start);
            }
            if (!param.rawName_row_end.IsEmpty ()) {
                ParamValue arr_row_end = {};
                arr_row_end.fromGDLparam = true;
                arr_row_end.rawName = param.rawName_row_end;
                paramdiap.Put (arr_row_end.rawName, arr_row_end);
            }
            if (!param.rawName_col_start.IsEmpty ()) {
                ParamValue arr_col_start = {};
                arr_col_start.fromGDLparam = true;
                arr_col_start.rawName = param.rawName_col_start;
                paramdiap.Put (arr_col_start.rawName, arr_col_start);
            }
            if (!param.rawName_col_end.IsEmpty ()) {
                ParamValue arr_col_end = {};
                arr_col_end.fromGDLparam = true;
                arr_col_end.rawName = param.rawName_col_end;
                paramdiap.Put (arr_col_end.rawName, arr_col_end);
            }
        }
    }
    if (!paramdiap.IsEmpty ()) {
        if (ParamHelpers::GDLParamByName (element, elem_head, paramdiap, paramnamearray)) {
            for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
                ParamValue &param = cIt->value;
#else
                ParamValue &param = *cIt->value;
#endif
                if (param.needPreRead) {
                    if (!param.rawName_row_start.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_row_start))
                            params.Get (param.rawName).val.array_row_start =
                                paramdiap.Get (param.rawName_row_start).val.intValue;
                    }
                    if (!param.rawName_row_end.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_row_end))
                            params.Get (param.rawName).val.array_row_end =
                                paramdiap.Get (param.rawName_row_end).val.intValue;
                    }
                    if (!param.rawName_col_start.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_col_start))
                            params.Get (param.rawName).val.array_column_start =
                                paramdiap.Get (param.rawName_col_start).val.intValue;
                    }
                    if (!param.rawName_col_end.IsEmpty ()) {
                        if (paramdiap.ContainsKey (param.rawName_col_end))
                            params.Get (param.rawName).val.array_column_end =
                                paramdiap.Get (param.rawName_col_end).val.intValue;
                    }
                }
            }
        }
    }

    // Разбиваем по типам поиска - по описанию/по имени
    GS::Array<GS::UniString> tparams;
    GS::Array<GS::UniString> tarray;
    GS::Array<GS::UniString> local_scratch;
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        GS::UniString rawName = param.rawName;
        if (param.fromGDLArray) {
            tparams.Clear ();
            UInt32 nparam = StringSplt (rawName, "@arr", tparams, true, &local_scratch);

            // Проверим - были ли заданы числовые параметры для чтения (диапазоны и тип обработки массива)
            if ((param.val.array_row_start == 0 || param.val.array_row_end == 0 || param.val.array_column_start == 0 ||
                 param.val.array_column_end == 0 || param.val.array_format_out == ARRAY_UNDEF) &&
                nparam > 1) {
                tarray.Clear ();
                UInt32 narray = StringSplt (tparams[1], "_", tarray, true, &local_scratch);
                if (param.val.array_row_start == 0 && narray > 0) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[0], doubleValue)) {
                        param.val.array_row_start = (int)doubleValue;
                    }
                }
                if (param.val.array_row_end == 0 && narray > 1) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[1], doubleValue)) {
                        param.val.array_row_end = (int)doubleValue;
                    }
                }
                if (param.val.array_column_start == 0 && narray > 2) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[2], doubleValue)) {
                        param.val.array_column_start = (int)doubleValue;
                    }
                }
                if (param.val.array_column_end == 0 && narray > 3) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[3], doubleValue)) {
                        param.val.array_column_end = (int)doubleValue;
                    }
                }
                if (param.val.array_format_out == ARRAY_UNDEF && narray > 4) {
                    double doubleValue = 0;
                    if (UniStringToDouble (tarray[4], doubleValue)) {
                        param.val.array_format_out = (int)doubleValue;
                    }
                }
            }
            if (param.val.array_format_out == ARRAY_UNDEF)
                param.val.array_format_out = ARRAY_SUM;
            rawName = tparams[0] + BRACEEND;

            if (auto *p = paramnamearray.GetPtr (rawName)) {
                if (!p->Contains (param.rawName))
                    p->Push (param.rawName);
            } else {
                GS::Array<GS::UniString> t = {};
                t.Push (param.rawName);
                paramnamearray.Put (rawName, std::move (t));
            }
            if (param.fromGDLdescription && eltype == API_ObjectID) {
                paramBydescription.Put (rawName, param);
            } else {
                if (param.fromGDLparam)
                    paramByName.Put (rawName, param);
            }
        }
        if (param.fromGDLdescription && eltype == API_ObjectID) {
            paramBydescription.Put (param.rawName, param);
        } else {
            if (param.fromGDLparam)
                paramByName.Put (param.rawName, param);
        }
    }
    if (paramBydescription.IsEmpty () && paramByName.IsEmpty ())
        return false;

    // Поиск по описанию
    bool flag_find_desc = false;
    bool flag_find_name = false;
    if (!paramBydescription.IsEmpty ()) {
        flag_find_desc = ParamHelpers::GDLParamByDescription (element, paramBydescription, paramByName, paramnamearray);
    }
    if (!paramByName.IsEmpty ())
        flag_find_name = ParamHelpers::GDLParamByName (element, elem_head, paramByName, paramnamearray);
    if (flag_find_desc && flag_find_name) {
        for (ParamDictValue::PairIterator cIt = paramBydescription.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            const ParamValue &param_by_desc = cIt->value;
            const auto &key = cIt->key;
#else
            const ParamValue &param_by_desc = *cIt->value;
            const auto &key = *cIt->key;
#endif
            const GS::UniString &rawname = param_by_desc.name;
            const ParamValue *paramByNamePtr = paramByName.GetPtr (rawname);
            if (paramByNamePtr == nullptr)
                continue;
            ParamValue param_by_name = *paramByNamePtr;
            param_by_name.name = param_by_desc.val.uniStringValue;
            param_by_name.rawName = param_by_desc.rawName;
            paramByName.Put (key, std::move (param_by_name));
        }
    }
    if (!flag_find_name)
        return flag_find_name;
    for (const auto &cIt : paramByName) {
#if defined(AC_28) || defined(AC_29)
        const GS::UniString &k = cIt.key;
        ParamValue &p = cIt.value;
#else
        const GS::UniString &k = *cIt.key;
        ParamValue &p = *cIt.value;
#endif
        if (!p.isValid) {
            if (element.header.guid == elem_head.guid)
                AddUnreadGDLParams (element.object.libInd, k);
            continue;
        }
        if (ParamValue *paramFromPtr = allparams.GetPtr (k)) {
            p.fromGuid = paramFromPtr->fromGuid;
            p.toQRCode = paramFromPtr->toQRCode;
            *paramFromPtr = p;
        }
    }
    return flag_find_name;
}

// -----------------------------------------------------------------------------
// Поиск по описанию GDL параметра
// Данный способ не работает с элементами навесных стен
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByDescription (const API_Element &element,
                                          ParamDictValue &params,
                                          ParamDictValue &find_params,
                                          GS::HashTable<GS::UniString, GS::Array<GS::UniString>> &paramnamearray) {
    API_LibPart libpart = {};
    libpart.index = element.object.libInd;
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryPart_Get (&libpart);
#else
    err = ACAPI_LibPart_Get (&libpart);
#endif
    if (err != NoError) {
        msg_rep ("ParamHelpers::GDLParamByDescription", "ACAPI_LibPart_Get", err, element.header.guid);
        return false;
    }
    double aParam = 0.0;
    double bParam = 0.0;
    Int32 addParNum = 0;
    API_AddParType **addPars = NULL;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_LibraryPart_GetParams (libpart.index, &aParam, &bParam, &addParNum, &addPars);
#else
    err = ACAPI_LibPart_GetParams (libpart.index, &aParam, &bParam, &addParNum, &addPars);
#endif
    if (err != NoError) {
        ACAPI_DisposeAddParHdl (&addPars);
        msg_rep ("ParamHelpers::GDLParamByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
        return false;
    }

    if (addPars == nullptr || *addPars == nullptr) {
        ACAPI_DisposeAddParHdl (&addPars);
        msg_rep ("FindGDLParametersByDescription", "ACAPI_LibPart_GetParams", err, element.header.guid);
        return false;
    }

    bool flagFind = false;
    Int32 nfind = params.GetSize ();
    // Ищем описание параметров
    for (Int32 i = 0; i < addParNum; ++i) {
        API_AddParType &actualParam = (*addPars)[i];
        GS::UniString desc_rawname = GetGDLRawName (actualParam.uDescname);
        if (!params.ContainsKey (desc_rawname))
            continue;

        // Получаем имя параметра
        GS::UniString rawname = GetGDLRawName (actualParam.name);
        ParamValue &pvalue = params.Get (desc_rawname);
        // Если в словаре для чтения по имени параметра такого параметра нет - добавим
        if (!find_params.ContainsKey (rawname)) {
            pvalue.rawName = rawname;
            find_params.Put (rawname, pvalue);
        }
        // Описание на время сохраним в val.uniStringValue
        pvalue.val.uniStringValue = pvalue.name;

        // rawname с именем параметра для дальнейшего сопоставления
        pvalue.name = rawname;
        flagFind = true;
        nfind--;
        if (nfind == 0) {
            ACAPI_DisposeAddParHdl (&addPars);
            return flagFind;
        }
    }
    ACAPI_DisposeAddParHdl (&addPars);
    return flagFind;
}

// -----------------------------------------------------------------------------
// Поиск по имени GDL параметра
// -----------------------------------------------------------------------------
bool ParamHelpers::GDLParamByName (const API_Element &element,
                                   const API_Elem_Head &elem_head,
                                   ParamDictValue &params,
                                   GS::HashTable<GS::UniString, GS::Array<GS::UniString>> &paramnamearray) {
    API_ElemTypeID elemType;
    API_Guid elemGuid;
    GetGDLParametersHead (element, elem_head, elemType, elemGuid);
    API_ElementMemo memo = {};
    GSErrCode err = ACAPI_Element_GetMemo (elemGuid, &memo, APIMemoMask_AddPars);
    if (err != NoError) {
        msg_rep ("ParamHelpers::GDLParamByName", "GetGDLParameters", err, elemGuid);
        ACAPI_DisposeElemMemoHdls (&memo);
        return false;
    }

    auto UpdateParamValue = [&] (ParamValue *pvaluePtr, const API_AddParType &actualParam) {
        if (pvaluePtr == nullptr)
            return false;
        FormatString fstring = pvaluePtr->val.formatstring;
        ParamHelpers::ConvertToParamValue (*pvaluePtr, actualParam);
        if (pvaluePtr->isValid) {
            if (!fstring.isEmpty) {
                pvaluePtr->val.formatstring = fstring;
            }
            return true;
        }
        return false;
    };

    bool flagFind = false;
    const GSSize nParams = BMGetHandleSize ((GSHandle)memo.params) / sizeof (API_AddParType);
    Int32 nfind = params.GetSize ();
    GS::UniString rawname;
    for (Int32 i = 0; i < nParams; ++i) {
        API_AddParType &actualParam = (*memo.params)[i];
        rawname = GetGDLRawName (actualParam.name);
        ParamValue *pvaluePtr = params.GetPtr (rawname);
        if (pvaluePtr == nullptr)
            continue;
        // Проверим - нет ли подходящих параметров-массивов?
        if (const auto *paramarrayPtr = paramnamearray.GetPtr (rawname)) {
            for (UInt32 j = 0; j < paramarrayPtr->GetSize (); ++j) {
                nfind--;
                if (UpdateParamValue (params.GetPtr ((*paramarrayPtr)[j]), actualParam)) {
                    flagFind = true;
                }
            }
            if (UpdateParamValue (pvaluePtr, actualParam)) {
                flagFind = true;
            }
            nfind--;
        } else {
            if (UpdateParamValue (pvaluePtr, actualParam)) {
                flagFind = true;
            }
            nfind--;
        }
        if (nfind == 0) {
            // Всё нужное нашли, выходим
            break;
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return flagFind;
}

bool hasLibData (const GS::UniString &description) {
    if (description.Contains (STRINGPROC)) {
        if (description.Contains ("%elem."))
            return true;
        if (description.Contains ("%prokat."))
            return true;
        if (description.Contains ("%mat."))
            return true;
        if (description.Contains ("%arm."))
            return true;
        if (description.Contains ("%subpos."))
            return true;
    } else {
        if (description.Contains (LISTDATANAMEPREFIX)) {
            if (description.Contains ("{@listdata:elem."))
                return true;
            if (description.Contains ("{@listdata:prokat."))
                return true;
            if (description.Contains ("{@listdata:mat."))
                return true;
            if (description.Contains ("{@listdata:arm."))
                return true;
            if (description.Contains ("{@listdata:subpos."))
                return true;
        }
    }
    return false;
}

// -----------------------------------------------------------------------------
// Обработка свойств с формулами
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadFormula (ParamDictValue &params, bool hasListData) {
#if defined(TESTING)
    DBprnt ("      ReadFormula");
#endif
    bool flag_find = false;
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
        GS::UniString key = cIt->key;
#else
        ParamValue &param = *cIt->value;
        GS::UniString key = *cIt->key;
#endif
        if (!param.val.hasFormula)
            continue;
        GS::UniString expression = param.val.uniStringValue;
        if (!hasListData) {
            if (hasLibData (expression))
                continue;
        }
        FormatString f = param.val.formatstring;
        if (!param.val.formatstring.isEmpty)
            expression = expression + '.' + param.val.formatstring.stringformat;
        if (!ParamHelpers::ReplaceParamInExpression (params, expression)) {
#if defined(TESTING)
            DBprnt ("ReadFormula err ReplaceParamInExpression", expression);
#endif
            continue;
        }
        if (!EvalExpression (expression)) {
#if defined(TESTING)
            DBprnt ("ReadFormula err EvalExpression", expression);
#endif
            continue;
        }
        flag_find = true;
        ParamValue pvalue = {};
        ParamHelpers::ConvertStringToParamValue (pvalue, key, expression);
        pvalue.val.formatstring = f;
        param.val = pvalue.val;
        // Если выражение можно вычислить ещё раз - запишем в чиловое значение результат, текст трогать не будем
        GS::UniString expression_ = "";
        if (!param.val.formatstring.isEmpty) {
            expression_ = STRFORMULASTART + expression + STRFORMULAEND + '.' + param.val.formatstring.stringformat;
        } else {
            expression_ = STRFORMULASTART + expression + STRFORMULAEND;
        }
        if (EvalExpression (expression_)) {
            ParamHelpers::ConvertStringToParamValue (pvalue, key, expression_);
            pvalue.val.formatstring = f;
            param.val = pvalue.val;
            param.val.uniStringValue = expression;
        }
        param.isValid = true;
    }
    return flag_find;
}

bool ParamHelpers::ReadListData (const API_Elem_Head &elem_head,
                                 ParamDictValue &pdictvalue,
                                 ListData::LibElement &paramListDataToRead,
                                 bool needListData) {
#if defined(TESTING)
    DBprnt ("      ReadListData");
#endif
    GSErrCode err = NoError;
    Int32 nComp = 0;
#if defined(AC_22) || defined(AC_23) || defined(AC_24)
    API_DescriptorRefType **descRefs;
    err = ACAPI_Element_GetDescriptors (&elem_head, &descRefs, &nComp);
#else
    API_DescriptorRefType **descRefs;
    err = ACAPI_Element_GetDescriptors (&elem_head, &descRefs, &nComp);
#endif
    if (err != NoError) {
        msg_rep ("ReadListData", "ACAPI_Element_GetDescriptors", err, elem_head.guid);
        return false;
    }
#if defined(TESTING)
    DBprnt ("          Read Descriptors");
#endif
    GS::UniString key_th = "some_stuff_th";
    GS::Array<GS::UniString> partstring = {};
    GS::Array<GS::UniString> local_scratch;
    for (Int32 i = 0; i < nComp; i++) {
        if ((*descRefs)[i].status == APIDBRef_Deleted)
            continue;
        API_ListData listdata = {};
        listdata.header.typeID = API_DescriptorID;
        listdata.header.index = (*descRefs)[i].index;
        listdata.header.setIndex = (*descRefs)[i].setIndex;
        switch ((*descRefs)[i].status) {
        case APIDBRef_Normal:
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_OldListing_Get (&listdata);
#else
            err = ACAPI_ListData_Get (&listdata);
#endif
            break;
        case APIDBRef_Local:
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_OldListing_GetLocal ((*descRefs)[i].libIndex, &elem_head, &listdata);
#else
            err = ACAPI_ListData_GetLocal ((*descRefs)[i].libIndex, &elem_head, &listdata);
#endif
            break;
        }
        if (err != NoError)
            continue;
        char tcode[API_DBCodeLen];
        char tkeycode[API_DBCodeLen];
        char tname[API_UAddParNumDescLen];
        CHTruncate (listdata.descriptor.code, tcode, API_DBCodeLen);
        CHTruncate (listdata.descriptor.keycode, tkeycode, API_DBCodeLen);
        CHTruncate (*listdata.descriptor.name, tname, API_DBNameLen);
        GS::UniString code = GS::UniString (tcode);
        GS::UniString name = GS::UniString (tname);
        GS::UniString keycode = GS::UniString (tkeycode);
        if (name.Contains (key_th)) {
            partstring.Clear ();
            UInt32 n = StringSplt (name, SEMICOLON, partstring, false, &local_scratch);
            if (n < 3)
                continue;
            GS::UniString attribsuffix = key_th + CharENTER + partstring[1];
            GS::UniString ttxt = partstring[2];
            double t = 0;
            if (!UniStringToDouble (ttxt, t))
                continue;
            ParamHelpers::AddLengthValueToParamDictValue (
                pdictvalue, elem_head.guid, LISTDATANAMEPREFIX, attribsuffix, t / 1000.0, false);
            continue;
        } else {
            if (!needListData)
                continue;
        }
    }
    if (!needListData)
        return !pdictvalue.IsEmpty ();
#if defined(TESTING)
    DBprnt ("          Read Components");
#endif
    BMKillHandle ((GSHandle *)&descRefs);
    nComp = 0;
#if defined(AC_22) || defined(AC_23) || defined(AC_24)
    API_ComponentRefType **compRefs;
    err = ACAPI_Element_GetComponents (&elem_head, &compRefs, &nComp);
#else
    API_Obsolete_ComponentRefType **compRefs;
    err = ACAPI_Element_GetComponents_Obsolete (&elem_head, &compRefs, &nComp);
#endif
    if (err != NoError) {
        msg_rep ("ReadListData", "ACAPI_Element_GetComponents_Obsolete", err, elem_head.guid);
        return false;
    }
    for (Int32 i = 0; i < nComp; i++) {
        if ((*compRefs)[i].status == APIDBRef_Deleted)
            continue;
        API_ListData listdata = {};
#if defined(AC_22) || defined(AC_23) || defined(AC_24)
        listdata.header.typeID = API_ComponentID;
#else
        listdata.header.typeID = API_Obsolete_ComponentID;
#endif
        listdata.header.index = (*compRefs)[i].index;
        listdata.header.setIndex = (*compRefs)[i].setIndex;
        switch ((*compRefs)[i].status) {
        case APIDBRef_Normal:
#if defined(AC_28) || defined(AC_29) || defined(AC_27)
            err = ACAPI_OldListing_Get (&listdata);
#else
            err = ACAPI_ListData_Get (&listdata);
#endif
            break;
        case APIDBRef_Local:
#if defined(AC_28) || defined(AC_29) || defined(AC_27)
            err = ACAPI_OldListing_GetLocal ((*compRefs)[i].libIndex, &elem_head, &listdata);
#else
            err = ACAPI_ListData_GetLocal ((*compRefs)[i].libIndex, &elem_head, &listdata);
#endif
            break;
        default:
            continue;
        }
        if (err != NoError) {
            msg_rep ("ReadListData", "ACAPI_ListData_Get", err, elem_head.guid);
            continue;
        }
        char tname[API_DBNameLen];
        CHTruncate (listdata.component.name, tname, API_DBNameLen);
        GS::UniString name = GS::UniString (tname);
        char tunitcode[API_DBCodeLen];
        CHTruncate (listdata.component.unitcode, tunitcode, API_DBCodeLen);
        GS::UniString unitcode = GS::UniString (tunitcode);
        double qty = listdata.component.quantity;
        ListData::Add (paramListDataToRead, name, unitcode, qty);
    }
    BMKillHandle ((GSHandle *)&compRefs);
    return !pdictvalue.IsEmpty ();
}

// -----------------------------------------------------------------------------
// Получение информации о элементе
// -----------------------------------------------------------------------------
void ParamHelpers::ReadQuantities (const API_Elem_Head &elemhead,
                                   ParamDictValue &params,
                                   ParamDictComposite &paramcomposite) {
#if defined(TESTING)
    DBprnt ("      ReadQuantities");
#endif
    API_ElementQuantity quantity = {};
    const API_ElemTypeID eltype = GetElemTypeID (elemhead);
    API_QuantityPar paramq = {};
    paramq.minOpeningSize = EPS;
    GSErrCode err = NoError;
    GS::Array<API_CompositeQuantity> composites = {};
    GS::Array<API_ElemPartQuantity> elemPartQuantities = {};
    GS::Array<API_ElemPartCompositeQuantity> elemPartComposites = {};
    API_QuantitiesMask mask;
    GS::HashTable<API_AttributeIndex, ParamValueComposite> composites_quantity = {}; // Словарь с материалами,
                                                                                     // считанными из компонент
    ACAPI_ELEMENT_QUANTITY_MASK_CLEAR (mask);
    ACAPI_ELEMENT_COMPOSITES_QUANTITY_MASK_SETFULL (mask);
    GS::Array<API_Quantities> quantities = {};
    quantities.Push (API_Quantities ());
    quantities[0].elements = &quantity;
    quantities[0].composites = &composites;
    quantities[0].elemPartQuantities = &elemPartQuantities;
    quantities[0].elemPartComposites = &elemPartComposites;
    GS::Array<API_Guid> elemGuids = {};
    elemGuids.Push (elemhead.guid);
    err = ACAPI_Element_GetMoreQuantities (&elemGuids, &paramq, &quantities, &mask);
    if (err != NoError) {
        msg_rep ("ReadQuantities", "ACAPI_Element_GetMoreQuantities", err, elemhead.guid);
        return;
    }
    GS::UniString rawname_th = MAT_SOME_STUFF_TH;
    GS::UniString rawname_unit = MAT_SOME_STUFF_UNITS;
    GS::UniString rawname_kzap = MAT_SOME_STUFF_KZAP;
    GS::UniString rawname_thlist = "{@listdata:some_stuff_th";
    // В случае, если прежде не были считаны данные по слоям (Например, для объекта) - добавляем слои из прочитанного
    GS::Array<ParamValueComposite> add_composite = {}; // Состав конструкции, считанный из компонент
    ParamDictValue paramsAdd = {};
    GS::UniString units = ""; // Единицы измерения из свойств
    double kzap = 1;          // Коэфф. запаса из свойств
    double th = 0;            // Толщина из свойств
    ParamHelpers::AddValueToParamDictValue (params, rawname_th);
    rawname_th = BRACESTART + rawname_th;
    ParamHelpers::AddValueToParamDictValue (params, rawname_unit);
    rawname_unit = BRACESTART + rawname_unit;
    ParamHelpers::AddValueToParamDictValue (params, rawname_kzap);
    rawname_kzap = BRACESTART + rawname_kzap;
    bool flag_find = false;
    bool need_add_composite = false;
    API_ModelElemStructureType composite_type = API_BasicStructure;
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
        GS::UniString rawname = cIt->key;
#else
        ParamValue &param = *cIt->value;
        GS::UniString rawname = *cIt->key;
#endif
        if (!param.fromQuantity)
            continue;
        if (param.composite_pen > 0)
            continue;
        if (const auto *compPtr = paramcomposite.GetPtr (rawname)) {
            composite_type = compPtr->composite_type;
            if (compPtr->composite.IsEmpty ()) {
                need_add_composite = true;
            }
        } else {
            need_add_composite = true;
        }
        break;
    }
    // Разбираем считанные компонены
    int num = 1;
    for (const auto &composit : composites) {
        const API_AttributeIndex &constrinx = composit.buildMatIndices;
        double volume = composit.volumes;     // Объём из компонент
        double area = composit.projectedArea; // Площадь проекции из компонент
        short flags = composit.flags;

        if (ParamValueComposite *pPtr = composites_quantity.GetPtr (constrinx)) {
            pPtr->volume += volume;
            pPtr->area += area;
            // Добавляем компонент
            ParamValueComposite pc = {};
            pc.inx = constrinx;
            pc.volume = volume;
            pc.area = area;
            pc.fillThick = pPtr->fillThick;
            pc.num = num;
            pc.unit = pPtr->unit;
            pc.kzap = pPtr->kzap;
            pc.structype = flags;
            num += 1;
            add_composite.Push (std::move (pc));
        } else {
            // Новый компонент, считываем его свойства
            ParamHelpers::GetAttributeValues (constrinx, params, paramsAdd);
            // Ищем единицы измерения в свойствах
            units = "";
            GS::UniString attribsuffix = CharENTER + GS::UniString::Printf ("%d", constrinx) + BRACEEND;
            GS::UniString fullUnitName = rawname_unit + attribsuffix;
            if (const ParamValue *paramPtr = params.GetPtr (fullUnitName)) {
                if (paramPtr->isValid) {
                    units = paramPtr->val.uniStringValue;
                } else {
                    units = "";
                }
            }

            // Ищем коэфф. запаса в свойствах
            kzap = 1;
            fullUnitName = rawname_kzap + attribsuffix;
            if (const ParamValue *paramPtr = params.GetPtr (fullUnitName)) {
                if (paramPtr->isValid) {
                    kzap = paramPtr->val.doubleValue;
                    if (is_equal (kzap, 0) || kzap < 0)
                        kzap = 1;
                }
            }

            // Ищем толщину в свойствах
            th = 0;
            fullUnitName = rawname_thlist + attribsuffix;
            if (const ParamValue *paramPtr = params.GetPtr (fullUnitName)) {
                if (paramPtr->isValid) {
                    th = paramPtr->val.doubleValue;
                }
            }
            if (is_equal (th, 0)) {
                fullUnitName = rawname_th + attribsuffix;
                if (const ParamValue *paramPtr = params.GetPtr (fullUnitName)) {
                    if (paramPtr->isValid) {
                        th = paramPtr->val.doubleValue;
                    }
                }
            }
            // Добавляем компонент
            ParamValueComposite p = {};
            p.inx = constrinx;
            p.volume = volume;
            p.area = area;
            p.fillThick = th;
            p.num = add_composite.GetSize () + 1;
            p.unit = units;
            p.kzap = kzap;
            p.structype = flags;
            composites_quantity.Put (p.inx, p);
            num += 1;
            add_composite.Push (std::move (p));
        }
        flag_find = true;
    }
    // Дочитываем параметры, найденный в свойствах
    bool needReadQuantities = true;

    // Получаем список всех компонент из предыдущих функций
    GS::Array<ParamValueComposite> all_composite = {}; // Состав конструкции, считанный из компонент
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &param = cIt->value;
#else
        ParamComposite &param = *cIt->value;
#endif
        if (param.composite_pen > 0)
            continue;
        if (!param.composite.IsEmpty ()) {
            all_composite = param.composite;
            break;
        }
    }
    // Пробуем простоq способ сопоставления.
    // Если массивы состава из компонент и из предыдущих функций одинаковые -
    // Сопоставляем по индексу строительного материала и флагу

    int num_add = add_composite.GetSize ();
    bool isOk = true;
    if (all_composite.GetSize () == add_composite.GetSize ()) {
        for (const auto &pll : all_composite) {
            if (pll.num <= 0) {
                isOk = false;
#if defined(TESTING)
                DBprnt ("ReadQuantities err", "pll.num <= 0");
#endif
                break;
            }
            if (pll.num > num_add) {
                isOk = false;
#if defined(TESTING)
                DBprnt ("ReadQuantities err", "pll.num > num_add");
#endif
                break;
            }
            auto &pdd = add_composite[pll.num - 1]; // Считанный компонент с объёмом
            if (pll.inx != pdd.inx) {
                isOk = false;
#if defined(TESTING)
                DBprnt ("ReadQuantities err", "pll.inx != pdd.inx");
#endif
                break;
            }
            if (pll.structype != pdd.structype && composite_type != API_BasicStructure) {
                isOk = false;
#if defined(TESTING)
                DBprnt ("ReadQuantities err", "pll.structype != pdd.structype");
#endif
                break;
            }
            if (!is_equal (pdd.fillThick, pll.fillThick) && !is_equal (pdd.fillThick, 0)) {
                GS::UniString msg = GS::UniString::Printf ("%f", pdd.fillThick);
                msg += " <-> ";
                msg += GS::UniString::Printf ("%f", pll.fillThick);
                msg += GS::UniString::Printf (" attrib inx: %d", pdd.inx);
                msg_rep ("Warning : Layer thickness",
                         "Different thickness in property and model : " + msg,
                         APIERR_GENERAL,
                         elemhead.guid);
            }
            if (is_equal (pdd.fillThick, 0))
                pdd.fillThick = pll.fillThick;
            if (pll.fillThick_min > 0)
                pdd.fillThick_min = pll.fillThick_min;
            pdd.length = pll.length;
            pdd.width = pll.width;
            pdd.area_fill = pll.area_fill;
            pdd.structype = pll.structype;
            if (!is_equal (pdd.fillThick, 0) && is_equal (pdd.area, 0))
                pdd.area = pdd.volume / pdd.fillThick;
            if (!is_equal (pdd.area_fill, 0) && is_equal (pdd.length, 0))
                pdd.length = pdd.volume / pdd.area_fill;
            if (!is_equal (pdd.fillThick, 0) && is_equal (pdd.width, 0))
                pdd.width = pdd.area_fill / pdd.fillThick;
            if (!is_equal (pdd.length, 0) && is_equal (pdd.area_fill, 0))
                pdd.area_fill = pdd.volume / pdd.length;
            ParamHelpers::SetUnitsAndQty2ParamValueComposite (pdd);
        }
    }
    if (all_composite.IsEmpty () && need_add_composite) {
        for (auto &pdd : add_composite) {
            if (!is_equal (pdd.fillThick, 0) && is_equal (pdd.area, 0))
                pdd.area = pdd.volume / pdd.fillThick;
            if (!is_equal (pdd.area_fill, 0) && is_equal (pdd.length, 0))
                pdd.length = pdd.volume / pdd.area_fill;
            if (!is_equal (pdd.fillThick, 0) && is_equal (pdd.width, 0))
                pdd.width = pdd.area_fill / pdd.fillThick;
            if (!is_equal (pdd.length, 0) && is_equal (pdd.area_fill, 0))
                pdd.area_fill = pdd.volume / pdd.length;
            ParamHelpers::SetUnitsAndQty2ParamValueComposite (pdd);
        }
    }
    if (isOk) {
        // Всё совпадает, можно записать в свойства и выходить
        for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            ParamComposite &param = cIt->value;
#else
            ParamComposite &param = *cIt->value;
#endif
            if (param.composite_pen > 0)
                continue;
            param.composite = add_composite;
        }
        if (!paramsAdd.IsEmpty ()) {
            ParamHelpers::ReadMaterial_ReadAddParam (paramsAdd, paramcomposite, params, true);
            paramsAdd.Clear ();
        }
        return;
    }
#if defined(TESTING)
    DBprnt ("ReadQuantities err", "long way");
    msg_rep ("Warning : ReadQuantities", "Old method", APIERR_GENERAL, elemhead.guid);
#endif
    // Если была необходимость добавления списка слоёв в общий словарь
    if (need_add_composite && !add_composite.IsEmpty ()) {
        for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
            ParamComposite &param = cIt->value;
#else
            ParamComposite &param = *cIt->value;
#endif
            if (param.composite_pen > 0)
                continue;
            if (param.composite.IsEmpty ())
                param.composite = add_composite;
        }
    }
    if (!paramsAdd.IsEmpty ()) {
        ParamHelpers::ReadMaterial_ReadAddParam (paramsAdd, paramcomposite, params, true);
        paramsAdd.Clear ();
    }
    // Создаём словарь компонент, считанных из предыдущих функций. Складываем толщины и площади сечений
    GS::HashTable<API_AttributeIndex, ParamValueComposite> composites_quantity_param = {}; // Словарь с компонентами,
                                                                                           // прочитанный прежде
                                                                                           // (сложный профиль и т.д.)
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &param = cIt->value;
#else
        ParamComposite &param = *cIt->value;
#endif
        if (param.composite_pen > 0)
            continue; // Считываем только композиты со всеми слоями (-1 и -2)
        for (const auto &p : param.composite) {
            if (ParamValueComposite *pcPtr = composites_quantity_param.GetPtr (p.inx)) {
                pcPtr->area_fill += p.area_fill;
                pcPtr->fillThick += p.fillThick;
#if defined(TESTING)
                DBtest (is_equal (p.area, 0), "is_equal(p.area)", true);
                DBtest (is_equal (p.volume, 0), "is_equal(p.volume)", true);
                DBtest (!is_equal (p.fillThick, 0), "!is_equal(p.fillThick)", true);
#endif
            } else {
                composites_quantity_param.Put (p.inx, p);
            }
        }
        break; // Достаточно считать только один композит, т.к. считываем только композиты со всеми слоями
    }
    // Добавляем площадь, объём и толщину из считанного словаря компонент
    for (GS::HashTable<API_AttributeIndex, ParamValueComposite>::PairIterator cIt =
             composites_quantity_param.EnumeratePairs ();
         cIt != NULL;
         ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValueComposite &p = cIt->value;
        API_AttributeIndex inx = cIt->key;
#else
        ParamValueComposite &p = *cIt->value;
        API_AttributeIndex inx = *cIt->key;
#endif
        const ParamValueComposite *pkPtr = composites_quantity.GetPtr (inx);
        if (pkPtr == nullptr) {
#if defined(TESTING)
            DBprnt ("ReadQuantities err", "!composites_quantity.ContainsKey (p.inx)");
#endif
            continue;
        }
        p.volume = pkPtr->volume;
        p.area = pkPtr->area;
    }

    // Расчитываем количества
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &param = cIt->value;
#else
        ParamComposite &param = *cIt->value;
#endif
        for (auto &p : param.composite) {
            const ParamValueComposite *qtyPtr = composites_quantity.GetPtr (p.inx);
            if (qtyPtr == nullptr)
                continue;

            ParamValueComposite *qtyParamPtr = composites_quantity_param.GetPtr (p.inx);
            if (qtyParamPtr == nullptr)
                continue;

            double volume_total = qtyPtr->volume;
            double area_fill_total = qtyParamPtr->area_fill;
            if (is_equal (area_fill_total, 0)) {
                area_fill_total = qtyPtr->area;
            }
            double fillThick_total = qtyParamPtr->fillThick;
            // Определяем толщину
            double fillThick = qtyPtr->fillThick;
            if (is_equal (fillThick, 0)) {
                fillThick = p.fillThick;
            } else {
                if (!is_equal (fillThick, p.fillThick)) {
                    GS::UniString msg = GS::UniString::Printf ("%f", fillThick);
                    msg += " <-> ";
                    msg += GS::UniString::Printf ("%f", p.fillThick);
                    msg += GS::UniString::Printf (" attrib inx: %d", p.inx);
                    msg_rep ("Warning : Layer thickness",
                             "Different thickness in property and model : " + msg,
                             APIERR_GENERAL,
                             elemhead.guid);
                }
            }
            double proc = 0;
            // Определяем долю площади проекции для текущего слоя
            if (is_equal (p.fillThick, fillThick_total) && is_equal (proc, 0) && !is_equal (p.fillThick, 0)) {
                proc = 1; // Если толщина слоя совпадает с суммарной площадью материала
            }
            if (is_equal (area_fill_total, p.area_fill) && is_equal (proc, 0) && !is_equal (p.area_fill, 0)) {
                proc = 1; // Если площадь слоя совпадает с суммарной площадью материала
            }
            // Определяем долю площади проекции для текущего слоя
            // По соотношению площадей
            if (!is_equal (area_fill_total, 0) && is_equal (proc, 0)) {
                proc = p.area_fill / area_fill_total;
            }
            // По оотношению толщин
            if (!is_equal (fillThick_total, 0) && is_equal (proc, 0)) {
                proc = p.fillThick / fillThick_total;
            }
            // Расчитываем площадь сечения, если она пустая
            if (is_equal (p.area_fill, 0) && !is_equal (p.length, 0)) {
                p.area_fill = p.volume / p.length;
            }
            if (is_equal (p.area_fill, 0)) {
                p.area_fill = area_fill_total * proc;
            }
            if (is_equal (proc, 0) && need_add_composite) {
                proc = 1; // Если не удалось определить долю слоя, но слой добавлен из свойств - считаем, что он весь
            }
            p.unit = composites_quantity.Get (p.inx).unit;
            p.volume = volume_total * proc;
            if (!is_equal (fillThick, 0) && is_equal (p.area, 0))
                p.area = p.volume / fillThick;
            if (!is_equal (p.area_fill, 0) && is_equal (p.length, 0))
                p.length = p.volume / p.area_fill;
            if (!is_equal (p.fillThick, 0) && is_equal (p.width, 0))
                p.width = p.area_fill / fillThick;
            ParamHelpers::SetUnitsAndQty2ParamValueComposite (p);
        }
    }
    return;
}

// -----------------------------------------------------------------------------
// Получение информации о элементе
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadElementValues (const API_Element &element, ParamDictValue &params) {
#if defined(TESTING)
    DBprnt ("      ReadElement");
#endif
    API_ElemTypeID eltype = GetElemTypeID (element);
    GS::UniString rawname = "";
    bool flag_find = false;
    rawname = "{@element:material overridden}";
    if (params.ContainsKey (rawname)) {
        ParamValue &param = params.Get (rawname);
        switch (eltype) {
        case API_WindowID:
            ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.window.openingBase.useObjMaterials);
            break;
        case API_DoorID:
            ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.door.openingBase.useObjMaterials);
            break;
        case API_ObjectID:
            ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.object.useObjMaterials);
        case API_LampID:
            ParamHelpers::ConvertBoolToParamValue (param, param.name, !element.lamp.useObjMaterials);
            break;
        case API_ZoneID:
            ParamHelpers::ConvertBoolToParamValue (param, param.name, element.zone.oneMat);
            break;
        default:
            param.isValid = false;
            break;
        }
        if (param.isValid)
            flag_find = true;
    }
    if (eltype == API_ZoneID) {
        rawname = "{@element:zone manual poly}";
        if (params.ContainsKey (rawname)) {
            ParamValue &param = params.Get (rawname);
            ParamHelpers::ConvertBoolToParamValue (param, param.name, element.zone.manual);
            if (param.isValid)
                flag_find = true;
        }
        rawname = "{@element:zone by ref line}";
        if (params.ContainsKey (rawname)) {
            ParamValue &param = params.Get (rawname);
            ParamHelpers::ConvertBoolToParamValue (param, param.name, element.zone.refLineFlag);
            if (param.isValid)
                flag_find = true;
        }
    }
    return flag_find;
}

// -----------------------------------------------------------------------------
// Обработка свойств с поиском в файлах
// -----------------------------------------------------------------------------
void ParamHelpers::ReadFile (ParamDictValue &params) {
#if defined(TESTING)
    DBprnt ("      ReadFile");
#endif
    bool flag_find = false;
    auto &cache = PROPERTYCACHE ();
    GS::UniString filename;
    GS::Array<ParamValue> vals;
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (!param.fromFile)
            continue;
        if (param.isValid)
            continue;
        int col_out = param.composite_pen;
        if (col_out <= 0)
            continue;
        // Получаем имя файла
        filename = param.val.uniStringValue;
        if (filename.Contains (ATSIGN)) {
            if (const auto *flenamePtr = params.GetPtr (filename)) {
                if (flenamePtr->isValid) {
                    filename = flenamePtr->val.uniStringValue;
                } else {
#if defined(TESTING)
                    DBprnt ("      ReadFile err ", filename);
#endif
                    continue;
                }
            }
        }
        filename.ReplaceAll ("\"", EMPTYSTRING);
        if (!filename.Contains (".txt") && !filename.Contains (".csv"))
            filename.Append (".txt");
        // 2. СНАЧАЛА открываем файл / достаем из кэша
        if (!cache.AddFile (filename))
            continue;
        const auto *dataPtr = cache.filedata.GetPtr (filename);
        if (dataPtr == nullptr || dataPtr->IsEmpty ())
            continue;
        const int firstRowSize = (int)(*dataPtr)[0].GetSize ();
        if (col_out > firstRowSize) {
            msg_rep ("ParamHelpers::ReadFile",
                     "col_out is out of bounds for the first row",
                     APIERR_BADINDEX,
                     APINULLGuid,
                     false);
            continue;
        }

        vals.Clear ();
        bool hasInvalidIndex = false;

        // Получаем свойства для поиска
        auto collectSearchParam = [&] (const GS::UniString &rawName, int arrayColumn) {
            if (hasInvalidIndex)
                return; // Если уже нашли ошибку, пропускаем остальные
            if (arrayColumn > 0 && rawName.Contains (ATSIGN)) {
                if (arrayColumn > firstRowSize) {
                    hasInvalidIndex = true;
                    return;
                }
                if (const auto *valPtr = params.GetPtr (rawName)) {
                    if (valPtr->isValid) {
                        ParamValue val = *valPtr;
                        val.val.array_format_out = arrayColumn;
                        vals.Push (std::move (val));
                    }
                }
            }
        };

        collectSearchParam (param.rawName_col_end, param.val.array_column_end);
        collectSearchParam (param.rawName_col_start, param.val.array_column_start);
        collectSearchParam (param.rawName_row_end, param.val.array_row_end);
        collectSearchParam (param.rawName_row_start, param.val.array_row_start);
        if (hasInvalidIndex || vals.IsEmpty ())
            continue;

        for (const auto &d : *dataPtr) {
            const int currentRowSize = (int)d.GetSize ();
            // Защита от поврежденных строк в файле
            if (currentRowSize < firstRowSize || col_out > currentRowSize)
                continue;
            bool flag = true;
            for (const auto &v : vals) {
                if (!d[v.val.array_format_out - 1].IsEqual (v.val.uniStringValue)) {
                    flag = false;
                    break;
                }
            }
            if (flag) {
                ParamHelpers::ConvertStringToParamValue (param, param.rawName, d[col_out - 1]);
                break;
            }
        }
    }
    return;
}

GS::UniString ParamHelpers::GetUnitsPrefix (GS::UniString &unit) {
    if (unit.IsEmpty ()) {
        return EMPTYSTRING;
    }
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    GS::UniString nameunits = RSGetIndString (iseng, 58, ACAPI_GetOwnResModule ());
    GS::UniString units = unit.ToLowerCase ();
    if (units.Contains (nameunits)) {
        return EMPTYSTRING;
    }
    nameunits = RSGetIndString (iseng, 53, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "S";
    }
    nameunits = RSGetIndString (iseng, 54, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "S";
    }
    nameunits = RSGetIndString (iseng, 55, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "V";
    }
    nameunits = RSGetIndString (iseng, 56, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "V";
    }
    nameunits = RSGetIndString (iseng, 57, ACAPI_GetOwnResModule ());
    if (units.Contains (nameunits)) {
        return "L";
    }
    return EMPTYSTRING;
}

void ParamHelpers::SetUnitsAndQty2ParamValueComposite (ParamValueComposite &comp) {
    if (comp.unit.IsEmpty ()) {
        comp.qty = 0;
        return;
    }
    const auto &cache = PROPERTYCACHE ();
    GS::UniString units = comp.unit;
    units.SetToLowerCase ();
    if (units.Contains (cache.dontspecstr_1)) { //"не специфицировать"
        comp.qty = 0;
        return;
    }
    if (units.Contains (cache.areastr_1)) { //"кв.м."
        comp.qty = comp.area * comp.kzap;
        return;
    }
    if (units.Contains (cache.areastr_2)) { //"м²"
        comp.qty = comp.area * comp.kzap;
        return;
    }
    if (units.Contains (cache.volumestr_1)) { //"куб.м."
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    if (units.Contains (cache.volumestr_2)) { //"м³"
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    if (units.Contains (cache.lengthstr_1)) { //"п.м."
        comp.qty = comp.length * comp.kzap;
        return;
    }
    if (units.Contains (cache.dontspecstr_2)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    if (units.Contains (cache.areastr_3)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    if (units.Contains (cache.areastr_4)) {
        comp.qty = comp.area * comp.kzap;
        return;
    }
    if (units.Contains (cache.volumestr_3)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    if (units.Contains (cache.volumestr_4)) {
        comp.qty = comp.volume * comp.kzap;
        return;
    }
    if (units.Contains (cache.lengthstr_3)) {
        comp.qty = comp.length * comp.kzap;
        return;
    }
}

void ParamHelpers::ReadMaterial_ReadAddParam (ParamDictValue &paramsAdd,
                                              ParamDictComposite &paramcomposite,
                                              ParamDictValue &params,
                                              bool needReadQuantities) {
    // В свойствах могли быть ссылки на другие свойста. Проверим, распарсим
    if (paramsAdd.IsEmpty ())
        return;
    if (needReadQuantities) {
        ParamHelpers::AddValueToParamDictValue (paramsAdd, MAT_SOME_STUFF_TH);
        ParamHelpers::AddValueToParamDictValue (paramsAdd, MAT_SOME_STUFF_UNITS);
        ParamHelpers::AddValueToParamDictValue (paramsAdd, MAT_SOME_STUFF_KZAP);
    }
    if (!ParamHelpers::isPropertyDefinitionRead ())
        return;
    ParamDictValue &propertyParams = PROPERTYCACHE ().property;
    ParamHelpers::CompareParamDictValue (propertyParams, paramsAdd);

    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &param_composite = cIt->value;
#else
        ParamComposite &param_composite = *cIt->value;
#endif
        ParamDictValue paramsAdd_1 = {};
        if (!param_composite.templatestring.Contains (BRACESTART))
            continue;
        bool flag = false;
        for (const auto &p : param_composite.composite) {
            API_AttributeIndex constrinx = p.inx;
            ParamHelpers::GetAttributeValues (constrinx, paramsAdd, paramsAdd_1);
            if (!paramsAdd_1.IsEmpty ())
                ParamHelpers::GetAttributeValues (constrinx, paramsAdd_1, paramsAdd_1);
            flag = true;
        }
        if (flag) {
            for (ParamDictValue::PairIterator cIt = paramsAdd.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
                if (!params.ContainsKey (cIt->key)) {
                    params.Put (cIt->key, cIt->value);
#else
                if (!params.ContainsKey (*cIt->key)) {
                    params.Put (*cIt->key, *cIt->value);
#endif
                }
            }
        }
        if (!paramsAdd_1.IsEmpty ()) {
            for (ParamDictValue::PairIterator cIt = paramsAdd_1.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
                if (!params.ContainsKey (cIt->key)) {
                    params.Put (cIt->key, cIt->value);
#else
                if (!params.ContainsKey (*cIt->key)) {
                    params.Put (*cIt->key, *cIt->value);
#endif
                }
            }
        }
    }
}

// -----------------------------------------------------------------------------
// Получение информации о материалах и составе конструкции
// -----------------------------------------------------------------------------
bool ParamHelpers::ReadMaterial (const API_Element &element,
                                 ParamDictValue &params,
                                 ParamDictComposite &paramcomposite,
                                 bool &needReadQuantities) {
#if defined(TESTING)
    DBprnt ("      ReadMaterial");
#endif
    // Получим состав элемента, добавив в словарь требуемые параметры
    ParamDictValue paramsAdd = {}; // Словарь для свойств, найденных в формулах в значениях свойств. Вот так-то.
    if (!ParamHelpers::Components (element, params, paramsAdd, paramcomposite, needReadQuantities))
        return false;
    if (paramcomposite.IsEmpty ())
        return true;

    ParamHelpers::ReadMaterial_ReadAddParam (paramsAdd, paramcomposite, params, needReadQuantities);
    bool flag_add = false;
    if (needReadQuantities)
        ParamHelpers::ReadQuantities (element.header, params, paramcomposite);

    static const GS::UniString layers_inv = "{@material:layers_inv";
    static const GS::UniString layers_auto = "{@material:layers_auto";
    static const GS::UniString nosyncname = "{@property:nosyncname}";
    static const GS::UniString layer_minthickness = "{@material:layer min thickness}";
    static const GS::UniString layer_thickness = "{@material:layer thickness}";
    static const GS::UniString area = "{@material:area}";
    static const GS::UniString area_section = "{@material:area_section}";
    static const GS::UniString width = "{@material:width}";
    static const GS::UniString length = "{@material:length}";
    static const GS::UniString volume = "{@material:volume}";
    static const GS::UniString qty = "{@material:qty}";
    static const GS::UniString unit_prefix = "{@material:unit_prefix}";
    static const GS::UniString unit = "{@material:unit}";
    static const GS::UniString n_ = "{@material:n}";
    static const GS::UniString ns_ = "{@material:ns}";

    auto *layer_thickness_ptr = params.GetPtr (layer_thickness);
    auto *layer_minthickness_ptr = params.GetPtr (layer_minthickness);
    auto *area_ptr = params.GetPtr (area);
    auto *area_section_ptr = params.GetPtr (area_section);
    auto *width_ptr = params.GetPtr (width);
    auto *length_ptr = params.GetPtr (length);
    auto *volume_ptr = params.GetPtr (volume);
    auto *qty_ptr = params.GetPtr (qty);
    auto *unit_prefix_ptr = params.GetPtr (unit_prefix);
    auto *n_ptr = params.GetPtr (n_);
    auto *ns_ptr = params.GetPtr (ns_);

    if (n_ptr == nullptr) {
        ParamHelpers::AddStringValueToParamDictValue (params, element.header.guid, MATERIALNAMEPREFIX, "n", "");
        n_ptr = params.GetPtr (n_);
    }
    if (ns_ptr == nullptr) {
        ParamHelpers::AddStringValueToParamDictValue (params, element.header.guid, MATERIALNAMEPREFIX, "ns", "");
        ns_ptr = params.GetPtr (ns_);
    }

    API_ElemTypeID eltype = GetElemTypeID (element.header);
    // Если есть строка-шаблон - заполним её
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
        bool flag = false;
#if defined(AC_28) || defined(AC_29)
        ParamComposite &param_composite = cIt->value;
        GS::UniString rawName = cIt->key;
#else
        ParamComposite &param_composite = *cIt->value;
        GS::UniString rawName = *cIt->key;
#endif
        GS::UniString outstring = "";
        GS::UniString stringformat = "";
        if (param_composite.templatestring.Contains (BRACESTART)) {
            bool inverse = rawName.Contains (layers_inv);
            if (rawName.Contains (layers_auto)) {
                if (param_composite.composite_type != API_ProfileStructure) {
                    if (param_composite.eltype == API_WallID)
                        inverse = true;
                }
            }
            // Если строка-шаблон содержит указание nosyncname - то применять спецтекст не нужно
            bool ignore_sync = false;
            if (param_composite.templatestring.Contains (nosyncname)) {
                param_composite.templatestring.ReplaceAll (nosyncname, EMPTYSTRING);
                ignore_sync = true;
            } else {
                if (param_composite.templatestring.IsEqual (qty)) {
                    ignore_sync = true;
                } else {
                    if (param_composite.templatestring.IsEqual (unit)) {
                        ignore_sync = true;
                    }
                }
            }
            if (param_composite.composite_pen == -2)
                ParamHelpers::ComponentsGetUnic (param_composite.composite);
            Int32 nlayers = param_composite.composite.GetSize ();
            if (param_composite.hasFormula) {
                // Если есть формула - заменим повторим все участки, заключенные в <> по количеству слоёв
                //  Например, 1+<толщина> -> 1+<&2><&1><&0>
                outstring = param_composite.templatestring;
                GS::UniString part = outstring.GetSubstring (CHARFORMULASTART, CHARFORMULAEND, 0);
                stringformat = "";
                FormatStringFunc::GetFormatStringFromFormula (outstring, part, stringformat);
                for (Int32 i = 0; i < nlayers; ++i) {
                    if (i == nlayers - 1) {
                        outstring.ReplaceAll (part, GS::UniString::Printf ("&%d&", i));
                    } else {
                        outstring.ReplaceAll (part, part + GS::UniString::Printf ("&%d&", i));
                    }
                }
            }
            Int32 ns = 0;
            for (Int32 i = 0; i < nlayers; ++i) {
                GS::UniString templatestring = param_composite.templatestring;
                // Для формул возьмём только часть в <>, только она повторяется для каждого слоя
                if (param_composite.hasFormula) {
                    if (!stringformat.IsEmpty ())
                        templatestring.ReplaceAll (STRFORMULAEND + stringformat, STRFORMULAEND);
                    templatestring = param_composite.templatestring.GetSubstring (CHARFORMULASTART, CHARFORMULAEND, 0);
                }
                Int32 indx = i;
                if (inverse)
                    indx = nlayers - i - 1;
                API_AttributeIndex constrinx = param_composite.composite[indx].inx;
                // Если для материала было указано уникальное наименование - заменим его
                GS::UniString attribsuffix = CharENTER + GS::UniString::Printf ("%d", constrinx) + BRACEEND;
                if (!ignore_sync) {
                    for (UInt32 inx = 0; inx < 20; inx++) {
                        GS::UniString syncname =
                            "{@property:sync_name" + GS::UniString::Printf ("%d", inx) + attribsuffix;
                        if (const auto *pp = params.GetPtr (syncname)) {
                            if (pp->isValid && !pp->property.isDefault) {
                                templatestring = pp->val.uniStringValue;
                                break;
                            }
                        }
                    }
                }
                // Если нужно заполнить толщину
                if (layer_thickness_ptr != nullptr) {
                    double fillThick = param_composite.composite[indx].fillThick;
                    if (layer_thickness_ptr->val.formatstring.isEmpty) {
                        layer_thickness_ptr->val.formatstring =
                            FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
                    }
                    layer_thickness_ptr->val.doubleValue = fillThick;
                    layer_thickness_ptr->val.rawDoubleValue = fillThick;
                    layer_thickness_ptr->val.hasrawDouble = true;
                    layer_thickness_ptr->val.type = API_PropertyRealValueType;
                    layer_thickness_ptr->isValid = true;
                }
                if (layer_minthickness_ptr != nullptr) {
                    if (!is_equal (param_composite.composite[indx].fillThick_min, -1.0) &&
                        !is_equal (param_composite.composite[indx].fillThick_min,
                                   param_composite.composite[indx].fillThick)) {
                        double fillThick = param_composite.composite[indx].fillThick_min;
                        if (layer_minthickness_ptr->val.formatstring.isEmpty) {
                            layer_minthickness_ptr->val.formatstring =
                                FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
                        }
                        layer_minthickness_ptr->val.doubleValue = fillThick;
                        layer_minthickness_ptr->val.rawDoubleValue = fillThick;
                        layer_minthickness_ptr->val.hasrawDouble = true;
                        layer_minthickness_ptr->val.type = API_PropertyRealValueType;
                        layer_minthickness_ptr->isValid = true;
                    }
                }
                if (param_composite.composite_pen < 0) {
                    if (area_ptr != nullptr) {
                        double val = param_composite.composite[indx].area;
                        if (area_ptr->val.formatstring.isEmpty) {
                            area_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("2m");
                        }
                        area_ptr->val.doubleValue = val;
                        area_ptr->val.rawDoubleValue = val;
                        area_ptr->val.hasrawDouble = true;
                        area_ptr->val.type = API_PropertyRealValueType;
                        area_ptr->isValid = true;
                    }
                    if (area_section_ptr != nullptr) {
                        double val = param_composite.composite[indx].area_fill;
                        if (area_section_ptr->val.formatstring.isEmpty) {
                            area_section_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("2m");
                        }
                        area_section_ptr->val.doubleValue = val;
                        area_section_ptr->val.rawDoubleValue = val;
                        area_section_ptr->val.hasrawDouble = true;
                        area_section_ptr->val.type = API_PropertyRealValueType;
                        area_section_ptr->isValid = true;
                    }
                    if (width_ptr != nullptr) {
                        double val = param_composite.composite[indx].width;
                        if (width_ptr->val.formatstring.isEmpty) {
                            width_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("0mm");
                        }
                        width_ptr->val.doubleValue = val;
                        width_ptr->val.rawDoubleValue = val;
                        width_ptr->val.hasrawDouble = true;
                        width_ptr->val.type = API_PropertyRealValueType;
                        width_ptr->isValid = true;
                    }
                    if (length_ptr != nullptr) {
                        double val = param_composite.composite[indx].length;
                        if (length_ptr->val.formatstring.isEmpty) {
                            length_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("0mm");
                        }
                        length_ptr->val.doubleValue = val;
                        length_ptr->val.rawDoubleValue = val;
                        length_ptr->val.hasrawDouble = true;
                        length_ptr->val.type = API_PropertyRealValueType;
                        length_ptr->isValid = true;
                    }
                    if (volume_ptr != nullptr) {
                        double val = param_composite.composite[indx].volume;
                        if (volume_ptr->val.formatstring.isEmpty) {
                            volume_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("2m");
                        }
                        volume_ptr->val.doubleValue = val;
                        volume_ptr->val.rawDoubleValue = val;
                        volume_ptr->val.hasrawDouble = true;
                        volume_ptr->val.type = API_PropertyRealValueType;
                        volume_ptr->isValid = true;
                    }
                    if (qty_ptr != nullptr) {
                        double val = param_composite.composite[indx].qty;
                        if (qty_ptr->val.formatstring.isEmpty) {
                            qty_ptr->val.formatstring = FormatStringFunc::ParseFormatString ("2m");
                        }
                        qty_ptr->val.doubleValue = val;
                        qty_ptr->val.rawDoubleValue = val;
                        qty_ptr->val.hasrawDouble = true;
                        qty_ptr->val.type = API_PropertyRealValueType;
                        qty_ptr->isValid = true;
                    }
                    if (unit_prefix_ptr != nullptr) {
                        GS::UniString val = ParamHelpers::GetUnitsPrefix (param_composite.composite[indx].unit);
                        unit_prefix_ptr->val.uniStringValue = val;
                        unit_prefix_ptr->val.type = API_PropertyStringValueType;
                        unit_prefix_ptr->isValid = true;
                    }
                }
                GS::UniString n_txt = GS::UniString::Printf ("%d", i + 1);
                templatestring.ReplaceAll (n_, n_txt);
                if (n_ptr != nullptr) {
                    n_ptr->val.uniStringValue = n_txt;
                    n_ptr->val.intValue = i + 1;
                    n_ptr->val.doubleValue = i + 1;
                    n_ptr->val.rawDoubleValue = i + 1;
                    n_ptr->val.type = API_PropertyStringValueType;
                    n_ptr->isValid = true;
                }
                n_txt = GS::UniString::Printf ("%d", ns + 1);
                templatestring.ReplaceAll (ns_, n_txt);
                if (ns_ptr != nullptr) {
                    ns_ptr->val.uniStringValue = n_txt;
                    ns_ptr->val.intValue = i + 1;
                    ns_ptr->val.doubleValue = i + 1;
                    ns_ptr->val.rawDoubleValue = i + 1;
                    ns_ptr->val.type = API_PropertyStringValueType;
                    ns_ptr->isValid = true;
                }
                templatestring.ReplaceAll (BRACEEND, attribsuffix);
                if (ParamHelpers::ReplaceParamInExpression (params, templatestring)) {
                    flag = true;
                    flag_add = true;
                    ReplaceSymbSpase (templatestring);
                    param_composite.composite[indx].val = templatestring;
                    if (param_composite.hasFormula) {
                        outstring.ReplaceAll (GS::UniString::Printf ("&%d&", i), templatestring);
                    } else {
                        outstring.Append (templatestring);
                    }
                    templatestring.Trim ();
                    if (!templatestring.IsEmpty ())
                        ns += 1;
                }
            }
        }
        if (flag) {
            if (param_composite.hasFormula) {
                if (outstring.Contains (BRACESTART))
                    ParamHelpers::ReplaceParamInExpression (params, outstring);
                if (!stringformat.IsEmpty ()) {
                    GS::UniString expression = outstring.GetSubstring (CHARFORMULASTART, CHARFORMULAEND, 0);
                    GS::UniString first_char = expression.GetSubstring (0, 1);
                    expression = STRFORMULASTART + expression + STRFORMULAEND + stringformat;
                    GS::UniString expression_clean = expression;
                    EvalExpression (expression);
                    expression = first_char + expression;
                    outstring.ReplaceAll (expression_clean, expression);
                }
                if (outstring.Contains (CHARFORMULASTART) && outstring.Contains (CHARFORMULAEND)) {
                    outstring.ReplaceAll (STRFORMULASTART, EMPTYSTRING);
                    outstring.ReplaceAll (STRFORMULAEND, EMPTYSTRING);
                }
                outstring = STRFORMULASTART + outstring + STRFORMULAEND;
            }
            param_composite.templatestring = outstring;
            ParamValue &p = params.Get (rawName);
            p.val.uniStringValue = outstring;
            p.isValid = true;
            p.val.type = API_PropertyStringValueType;
        }
    }
    return flag_add;
}

void ParamHelpers::Array2ParamValue (GS::Array<ParamValueData> &pvalue, ParamValueData &pvalrezult) {
    if (pvalue.IsEmpty ())
        return;
    GS::UniString delim = SEMICOLON;
    int array_format_out = pvalrezult.array_format_out;
    GS::UniString param_string = "";
    double param_real = 0;
    bool param_bool = false;
    if (array_format_out == ARRAY_MIN)
        param_bool = true;
    GS::Int32 param_int = 0;
    bool canCalculate = false;
    int array_column_end = pvalrezult.array_column_end;
    int array_column_start = pvalrezult.array_column_start;
    int array_row_end = pvalrezult.array_row_end;
    int array_row_start = pvalrezult.array_row_start;
    if (array_format_out == ARRAY_MAX || array_format_out == ARRAY_MIN) {
        ParamValueData &pval = pvalue.Get (0);
        param_real = pval.doubleValue;
        param_int = pval.intValue;
        param_bool = pval.boolValue;
        param_string = pval.uniStringValue;
    }

    for (UInt32 i = 0; i < pvalue.GetSize (); i++) {
        ParamValueData &pval = pvalue.Get (i);
        if (pval.canCalculate) {
            canCalculate = true;
            if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
                param_real = param_real + pval.doubleValue;
                param_int = param_int + pval.intValue;
                if (pval.boolValue)
                    param_bool = true;
            }
            if (array_format_out == ARRAY_MAX) {
                param_real = fmax (param_real, pval.doubleValue);
                if (pval.intValue > param_int)
                    param_int = pval.intValue;
                if (pval.boolValue)
                    param_bool = true;
            }
            if (array_format_out == ARRAY_MIN) {
                param_real = fmin (param_real, pval.doubleValue);
                if (pval.intValue < param_int)
                    param_int = pval.intValue;
                if (!pval.boolValue)
                    param_bool = false;
            }
        }
        if (array_format_out == ARRAY_SUM || array_format_out == ARRAY_UNIC) {
            if (param_string.IsEmpty ()) {
                param_string = pval.uniStringValue;
            } else {
                param_string.Append (delim);
                param_string.Append (pval.uniStringValue);
            }
        }
        if (array_format_out == ARRAY_MAX) {
            GSCharCode chcode = GetCharCode (pval.uniStringValue);
            std::string s = pval.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            std::string p = param_string.ToCStr (0, MaxUSize, chcode).Get ();
            if (doj::alphanum_comp (s, p) > 0)
                param_string = GS::UniString (s.c_str (), chcode);
        }
        if (array_format_out == ARRAY_MIN) {
            GSCharCode chcode = GetCharCode (pval.uniStringValue);
            std::string s = pval.uniStringValue.ToCStr (0, MaxUSize, chcode).Get ();
            std::string p = param_string.ToCStr (0, MaxUSize, chcode).Get ();
            if (doj::alphanum_comp (s, p) < 0)
                param_string = GS::UniString (s.c_str (), chcode);
        }
    }
    pvalrezult = pvalue.Get (0);
    if (array_format_out == ARRAY_UNIC) {
        pvalrezult.uniStringValue = StringUnic (param_string, delim);
    } else {
        pvalrezult.uniStringValue = param_string;
    }
    pvalrezult.array_column_end = array_column_end;
    pvalrezult.array_column_start = array_column_start;
    pvalrezult.array_row_end = array_row_end;
    pvalrezult.array_row_start = array_row_start;
    pvalrezult.array_format_out = array_format_out;
    pvalrezult.boolValue = param_bool;
    pvalrezult.doubleValue = param_real;
    pvalrezult.rawDoubleValue = param_real;
    pvalrezult.hasrawDouble = true;
    pvalrezult.intValue = param_int;
    pvalrezult.canCalculate = canCalculate;
}

// -----------------------------------------------------------------------------
// Конвертация одиночного параметра библиотечного элемента (тип API_ParSimple) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValueData &pvalue,
                                        const API_AddParID &typeIDr,
                                        const GS::UniString &pstring,
                                        double preal) {

    // TODO добавить округления на основе настроек проекта
    GS::UniString param_string = pstring;
    double param_real = preal;
    bool param_bool = false;
    GS::Int32 param_int = 0;
    pvalue.canCalculate = false;
    if (typeIDr == APIParT_CString) {
        pvalue.type = API_PropertyStringValueType;
        param_bool = (!param_string.IsEmpty ());

        if (UniStringToDouble (param_string, param_real)) {
            param_real = round (param_real * 100000.0) / 100000.0;
            param_int = (GS::Int32)param_real;
            if (param_int / 1 < param_real)
                param_int += 1;
            pvalue.canCalculate = true;
        } else {
            if (param_bool) {
                param_int = 1;
                param_real = 1.0;
            }
        }
    } else {
        param_real = round (param_real * 100000) / 100000;
        if (preal - param_real > 0.00001)
            param_real += 0.00001;
        param_int = (GS::Int32)param_real;
        if (param_int / 1 < param_real)
            param_int += 1;
    }
    if (fabs (param_real) > std::numeric_limits<double>::epsilon ())
        param_bool = true;
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    // Если параметр не строковое - определяем текстовое значение конвертацией
    if (typeIDr != APIParT_CString) {
        API_AttrTypeID attrType = API_ZombieAttrID;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
        API_AttributeIndex attrInx = ACAPI_CreateAttributeIndex (param_int);
#else
        short attrInx = (short)param_int;
#endif
        switch (typeIDr) {
        case APIParT_PenCol:
        case APIParT_Integer:
            param_string = GS::UniString::Printf ("%d", param_int);
            pvalue.type = API_PropertyIntegerValueType;
            pvalue.canCalculate = true;
            pvalue.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
            break;
        case APIParT_Boolean:
            if (param_bool) {
                param_string = RSGetIndString (iseng, TrueId, ACAPI_GetOwnResModule ());
                param_int = 1;
                param_real = 1.0;
            } else {
                param_string = RSGetIndString (iseng, FalseId, ACAPI_GetOwnResModule ());
                param_int = 0;
                param_real = 0.0;
            }
            pvalue.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
            pvalue.canCalculate = true;
            pvalue.type = API_PropertyBooleanValueType;
            break;
        case APIParT_Length:
            param_string = GS::UniString::Printf ("%.0f", param_real * 1000);
            pvalue.canCalculate = true;
            pvalue.type = API_PropertyRealValueType;
            pvalue.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
            break;
        case APIParT_Angle:
            param_real = round ((preal * 180.0 / PI) * 100000.0) / 100000.0;
            if (preal - param_real > 0.00001)
                param_real += 0.00001;
            param_int = (GS::Int32)param_real;
            if (param_int / 1 < param_real)
                param_int += 1;
            param_string = GS::UniString::Printf ("%.1f", param_real);
            pvalue.canCalculate = true;
            pvalue.type = API_PropertyRealValueType;
            pvalue.formatstring = FormatStringFunc::ParseFormatString ("2m");
            break;
        case APIParT_ColRGB:
        case APIParT_Intens:
        case APIParT_RealNum:
            param_string = GS::UniString::Printf ("%.3f", param_real);
            pvalue.canCalculate = true;
            pvalue.type = API_PropertyRealValueType;
            pvalue.formatstring = FormatStringFunc::ParseFormatString ("3m");
            break;
            // Для реквезитов в текст выведем имена
        case APIParT_LineTyp:
            attrType = API_LinetypeID;
            break;
        case APIParT_Profile:
            attrType = API_ProfileID;
            break;
        case APIParT_BuildingMaterial:
            attrType = API_BuildingMaterialID;
            break;
        case APIParT_FillPat:
            attrType = API_FilltypeID;
            break;
        case APIParT_Mater:
            attrType = API_MaterialID;
            break;
        default:
            return false;
            break;
        }
        if (attrType != API_ZombieAttrID) {
            API_Attribute attrib = {};
            attrib.header.typeID = attrType;
            attrib.header.index = attrInx;
            attrib.header.uniStringNamePtr = &param_string;
            GSErrCode err = ACAPI_Attribute_Get (&attrib);
            if (err == NoError) {
                pvalue.type = API_PropertyStringValueType;
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
                param_bool = (attrInx.ToInt32_Deprecated () != 0);
                param_int = attrInx.ToInt32_Deprecated ();
#else
                param_bool = (attrInx != 0);
                param_int = attrInx;
#endif
                param_real = param_int / 1.0;
                pvalue.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
            } else {
                if (err != APIERR_BADNAME)
                    return false;
            }
        }
    }
    pvalue.boolValue = param_bool;
    pvalue.doubleValue = param_real;
    pvalue.rawDoubleValue = param_real;
    pvalue.hasrawDouble = true;
    pvalue.intValue = param_int;
    pvalue.uniStringValue = param_string;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация параметра-массива библиотечного элемента (тип API_ParArray) в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValueData &pvalue,
                                        const API_AddParID &typeIDr,
                                        const GS::Array<GS::UniString> &pstring,
                                        const GS::Array<double> &preal,
                                        const GS::Int32 &dim1,
                                        const GS::Int32 &dim2) {
    // TODO Добавить обработку игнорируемых значений
    GS::Array<ParamValueData> pvalues;
    GS::UniString param_string = "";
    double param_real = 0.0;
    GS::Int32 inx_row = 1;
    GS::Int32 inx_col = 0;
    int array_row_start = pvalue.array_row_start;
    int array_row_end = pvalue.array_row_end;
    int array_column_start = pvalue.array_column_start;
    int array_column_end = pvalue.array_column_end;
    if (array_row_end < 1)
        array_row_end = dim1;
    if (array_column_end < 1)
        array_column_end = dim2;

    UInt32 n = 0;
    if (typeIDr == APIParT_CString) {
        if (pstring.IsEmpty ())
            return false;
        n = pstring.GetSize ();
    } else {
        if (preal.IsEmpty ())
            return false;
        n = preal.GetSize ();
    }
    for (UInt32 i = 0; i < n; i++) {
        inx_col += 1;
        if (inx_col > dim2) {
            inx_col = 1;
            inx_row += 1;
        }
        if (inx_col <= array_column_end && inx_col >= array_column_start && inx_row >= array_row_start &&
            inx_row <= array_row_end) {
            ParamValueData pval = {};
            if (typeIDr == APIParT_CString) {
                param_string = pstring.Get (i);
            } else {
                param_real = preal.Get (i);
            }
            if (ParamHelpers::ConvertToParamValue (pval, typeIDr, param_string, param_real)) {
                pval.array_row_start = inx_row;
                pval.array_row_end = inx_row;
                pval.array_column_start = inx_col;
                pval.array_column_end = inx_col;
                pvalues.Push (pval);
            }
        }
    }
    if (pvalues.IsEmpty ()) {
#if defined(TESTING)
        DBprnt ("ParamHelpers::ConvertToParamValue", "Empty array");
#endif
        return false;
    } else {
        ParamHelpers::Array2ParamValue (pvalues, pvalue);
        return true;
    }
}

// -----------------------------------------------------------------------------
// Конвертация параметра библиотечного элемента в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue &pvalue, const API_AddParType &nthParameter) {
    GS::UniString param_string = "";
    double param_real = 0.0;
    API_AddParID typeIDr = nthParameter.typeID;
    ParamValueData pval = pvalue.val;
    if (nthParameter.typeMod == API_ParArray) {
        size_t ind = 0;
        GS::Array<GS::UniString> arr_param_string;
        GS::Array<double> arr_param_real;
        if (nthParameter.typeID != APIParT_CString) {
            for (Int32 i = 1; i <= nthParameter.dim1; i++) {
                for (Int32 j = 1; j <= nthParameter.dim2; j++) {
                    param_real = ((double *)*nthParameter.value.array)[ind];
                    arr_param_real.Push (param_real);
                    ind++;
                }
            }
        } else {
            Int32 arrayIndex = 0;
            for (Int32 i = 1; i <= nthParameter.dim1; i++) {
                for (Int32 j = 1; j <= nthParameter.dim2; j++) {
                    GS::uchar_t *uValueStr = (reinterpret_cast<GS::uchar_t *> (*nthParameter.value.array)) + arrayIndex;
                    arrayIndex += GS::ucslen32 (uValueStr) + 1;
                    arr_param_string.Push (GS::UniString (uValueStr));
                }
            }
        }
        if (!ParamHelpers::ConvertToParamValue (
                pval, typeIDr, arr_param_string, arr_param_real, nthParameter.dim1, nthParameter.dim2))
            return false;
    }
    if (nthParameter.typeMod == API_ParSimple) {
        param_string = nthParameter.value.uStr;
        param_real = nthParameter.value.real;
        if (!ParamHelpers::ConvertToParamValue (pval, typeIDr, param_string, param_real))
            return false;
    }
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = GDLNAMEPREFIX;
        pvalue.rawName.Append (GS::UniString (nthParameter.name).ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
    }
    if (pvalue.name.IsEmpty ())
        pvalue.name = nthParameter.name;
    pvalue.val = pval;
    pvalue.type = pval.type;
    pvalue.fromGDLparam = true;
    pvalue.typeinx = GDLTYPEINX;
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Заполнение rawName для ParamValue по описанию в API_Property
// -----------------------------------------------------------------------------
void ParamHelpers::SetrawNameFromProperty (ParamValue &pvalue, const API_Property &property) {
    bool fromAttrib = false;
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname;
        GetPropertyFullName (property.definition, fname);
        if (pvalue.name.IsEmpty ())
            pvalue.name = fname;
        if (pvalue.rawName.IsEmpty ()) {
            pvalue.rawName = PROPERTYNAMEPREFIX;
            fname.SetToLowerCase ();
            pvalue.rawName.Append (fname);
            pvalue.rawName.Append (BRACEEND);
        }
    }
    if (property.definition.description.Contains (SYNCCORRECTFLAG)) {
        pvalue.rawName = "{@property:sync_correct_flag}";
        return;
    }
    if (pvalue.rawName.Contains (CharENTER)) {
        GS::UniString description = property.definition.description;
        description.SetToLowerCase ();
        if (description.Contains ("some_stuff_th")) {
            // Заданная толщина в материале. Используется, если не удалось вычислить из профиля
            GS::UniString inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, CHARBRACEEND, 0);
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_th" + inx + BRACEEND;
            pvalue.name = "some_stuff_th";
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
            return;
        }
        if (description.Contains ("some_stuff_units")) {
            GS::UniString inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, CHARBRACEEND, 0);
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_units" + inx + BRACEEND;
            pvalue.name = "some_stuff_units";
            return;
        }
        if (description.Contains ("some_stuff_kzap")) {
            GS::UniString inx = CharENTER + pvalue.rawName.GetSubstring (CharENTER, CHARBRACEEND, 0);
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_kzap" + inx + BRACEEND;
            pvalue.name = "some_stuff_kzap";
            return;
        }
    }
}

// -----------------------------------------------------------------------------
// Конвертация свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue &pvalue, const API_Property &property) {
    auto &cache = PROPERTYCACHE ();
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ())
        ParamHelpers::SetrawNameFromProperty (pvalue, property);
    API_PropertyValue value = {};
#if defined(AC_22) || defined(AC_23)
    pvalue.isValid = property.isEvaluated;
    if (property.isDefault && !property.isEvaluated) {
        value = property.definition.defaultValue.basicValue;
    } else {
        value = property.value;
    }
#else
    pvalue.isValid = (property.status == API_Property_HasValue);
    if (property.isDefault && property.status == API_Property_NotEvaluated) {
        value = property.definition.defaultValue.basicValue;
    } else {
        value = property.value;
    }
#endif
    pvalue.fromProperty = true;

    pvalue.definition = property.definition;
    pvalue.property = property;
    ParamHelpers::ConvertToParamValue_CheckAttrib (pvalue, property.definition);
    if (!pvalue.fromAttribDefinition)
        pvalue.fromPropertyDefinition = true;
    if (!pvalue.isValid && property.definition.guid == APINULLGuid) {
        return false;
    }
    pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
    if (pvalue.val.uniStringValue.IsEqual ("@Undefined Value@") && property.isDefault) {
        pvalue.isValid = false;
        return false;
    }
    FormatStringDict formatstringdict = {};
    switch (property.definition.valueType) {
    case API_PropertyIntegerValueType:
        pvalue.val.intValue = value.singleVariant.variant.intValue;
        pvalue.val.doubleValue = value.singleVariant.variant.intValue * 1.0;
        pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
        pvalue.val.hasrawDouble = true;
        if (pvalue.val.intValue > 0)
            pvalue.val.boolValue = true;
        pvalue.val.type = API_PropertyIntegerValueType;
        pvalue.val.canCalculate = true;
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
        pvalue.val.uniStringValue = PropertyHelpers::ToString (property, pvalue.val.formatstring);
        break;
    case API_PropertyRealValueType:

        // Конвертация угла из радиан в градусы
        if (property.definition.measureType == API_PropertyAngleMeasureType) {
            double ang = value.singleVariant.variant.doubleValue * 180.0 / PI;
            pvalue.val.rawDoubleValue = ang;
            pvalue.val.hasrawDouble = true;
            pvalue.val.doubleValue = round (ang * 100000.0) / 100000.0;
        } else {
            pvalue.val.rawDoubleValue = value.singleVariant.variant.doubleValue;
            pvalue.val.hasrawDouble = true;
            pvalue.val.doubleValue = round (value.singleVariant.variant.doubleValue * 100000.0) / 100000.0;
            if (value.singleVariant.variant.doubleValue - pvalue.val.doubleValue > 0.001)
                pvalue.val.doubleValue += 0.001;
        }
        if (pvalue.rawName.IsEqual ("{@property:buildingmaterialproperties/some_stuff_th}")) {
            if (property.definition.measureType != API_PropertyLengthMeasureType) {
                pvalue.val.rawDoubleValue = pvalue.val.rawDoubleValue / 1000.0;
                pvalue.val.doubleValue = pvalue.val.doubleValue / 1000.0;
                pvalue.val.doubleValue = round (pvalue.val.doubleValue * 100000.0) / 100000.0;
                pvalue.val.rawDoubleValue = round (pvalue.val.rawDoubleValue * 100000.0) / 100000.0;
            }
        }
        pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
        if (pvalue.val.intValue / 1 < pvalue.val.doubleValue)
            pvalue.val.intValue += 1;
        if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ())
            pvalue.val.boolValue = true;
        pvalue.val.type = API_PropertyRealValueType;
        if (!cache.isFormatStringFormeasureTypeRead)
            cache.ReadFormatStringForMeasureType ();
        if (cache.isFormatStringFormeasureType_OK) {
            if (const auto *formatstringch =
                    cache.formatstringformeasuretype.GetPtr (property.definition.measureType)) {
                bool needRound = formatstringch->needRound;
                if (pvalue.rawName.IsEqual ("{@property:buildingmaterialproperties/some_stuff_th}"))
                    needRound = false;
                if (needRound && property.definition.measureType != API_PropertyLengthMeasureType) {
                    pvalue.val.doubleValue = round_nzero (pvalue.val.doubleValue, formatstringch->n_zero);
                    pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
                    if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ())
                        pvalue.val.boolValue = true;
                }
                if (pvalue.val.formatstring.isEmpty) {
                    pvalue.val.formatstring = FormatStringFunc::ParseFormatString (formatstringch->stringformat);
                    pvalue.val.formatstring.needRound = needRound;
                }
                pvalue.val.uniStringValue = ParamHelpers::ToString (pvalue);
            } else {
            }
        }
        pvalue.val.canCalculate = true;
        break;
    case API_PropertyBooleanValueType:
        pvalue.val.boolValue = value.singleVariant.variant.boolValue;
        if (pvalue.val.boolValue) {
            pvalue.val.intValue = 1;
            pvalue.val.doubleValue = 1.0;
        }
        pvalue.val.type = API_PropertyBooleanValueType;
        pvalue.val.canCalculate = true;
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
        pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
        pvalue.val.hasrawDouble = true;
        pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
        break;
    case API_PropertyStringValueType:
    case API_PropertyGuidValueType:
        pvalue.val.uniStringValue = PropertyHelpers::ToString (property);
        pvalue.val.type = API_PropertyStringValueType;
        pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
        if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
            pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
            if (pvalue.val.intValue / 1 < pvalue.val.doubleValue)
                pvalue.val.intValue += 1;
            pvalue.val.canCalculate = true;
        } else {
            if (pvalue.val.boolValue) {
                pvalue.val.intValue = 1;
                pvalue.val.doubleValue = 1.0;
            }
        }
        pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
        pvalue.val.hasrawDouble = true;
        break;
    case API_PropertyUndefinedValueType:
        pvalue.isValid = false;
        return false;
        break;
    default:
        pvalue.isValid = false;
        return false;
        break;
    }
    pvalue.type = pvalue.val.type;
    return true;
}

void ParamHelpers::ConvertToParamValue_CheckAttrib (ParamValue &pvalue, const API_PropertyDefinition &definition) {
    GS::UniString description = definition.description;
    description.SetToLowerCase ();
    pvalue.typeinx = PROPERTYTYPEINX;
    if (description.Contains ("to{class:")) {
        GS::Array<GS::UniString> params = {};
        UInt32 nparam = StringSplt (definition.description, "to{Class", params, true);
        if (nparam > 1) {
            GS::UniString systemname = params.Get (1).GetSubstring (':', CHARBRACEEND, 0);
            pvalue.name = systemname.ToLowerCase ();
        }
        pvalue.fromClassification = true;
        return;
    }
    if (description.Contains (SYNCNAME)) {
        if (!pvalue.rawName.Contains ("{@property:sync_name")) {
            pvalue.rawName = "{@property:sync_name0}";
            pvalue.name = "Sync_name0";
        }
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (pvalue.rawName.Contains ("buildingmaterial")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (pvalue.rawName.Contains ("component")) {
        pvalue.fromAttribDefinition = true;
        return;
    }
    if (description.Contains ("some_stuff")) {
        if (description.Contains ("some_stuff_th")) {
            // Заданная толщина в материале. Используется, если не удалось вычислить из профиля
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_th}";
            pvalue.name = "some_stuff_th";
            pvalue.fromAttribDefinition = true;
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTLEGHTFSTRING);
            return;
        }
        if (description.Contains ("some_stuff_units")) {
            // Единицы измерения
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_units}";
            pvalue.name = "some_stuff_units";
            pvalue.fromAttribDefinition = true;
            return;
        }
        if (description.Contains ("some_stuff_layer_onoff")) {
            pvalue.fromAttribDefinition = true;
            return;
        }
        if (description.Contains ("some_stuff_layer_has_finish")) {
            pvalue.fromAttribDefinition = true;
            return;
        }
        if (description.Contains ("some_stuff_layer_description")) {
            pvalue.fromAttribDefinition = true;
            return;
        }
        if (description.Contains ("some_stuff_layer_favorite_name")) {
            pvalue.fromAttribDefinition = true;
            return;
        }
        if (description.Contains ("some_stuff_kzap")) {
            // Единицы измерения
            pvalue.rawName = "{@property:buildingmaterialproperties/some_stuff_kzap}";
            pvalue.name = "some_stuff_kzap";
            pvalue.fromAttribDefinition = true;
            return;
        }
    }
}

// -----------------------------------------------------------------------------
// Конвертация определения свойства в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertToParamValue (ParamValue &pvalue, const API_PropertyDefinition &definition) {
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname = "";
        GetPropertyFullName (definition, fname);
        if (pvalue.rawName.IsEmpty ()) {
            pvalue.rawName = PROPERTYNAMEPREFIX;
            pvalue.rawName.Append (fname.ToLowerCase ());
            pvalue.rawName.Append (BRACEEND);
        }
        if (pvalue.name.IsEmpty ())
            pvalue.name = fname;
    }
    ParamHelpers::ConvertToParamValue_CheckAttrib (pvalue, definition);
    pvalue.fromProperty = true;
    if (!pvalue.fromAttribDefinition)
        pvalue.fromPropertyDefinition = true;
    pvalue.val.type = definition.valueType;
    pvalue.type = definition.valueType;
    pvalue.definition = definition;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация строки в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertStringToParamValue (ParamValue &pvalue,
                                              const GS::UniString &paramName,
                                              const GS::UniString strvalue) {
    if (pvalue.name.IsEmpty ())
        pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = GDLNAMEPREFIX;
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        pvalue.typeinx = GDLTYPEINX;
    }
    pvalue.val.uniStringValue = strvalue;
    pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
    if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
        pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
        if (pvalue.val.intValue / 1 < pvalue.val.doubleValue)
            pvalue.val.intValue += 1;
        pvalue.val.canCalculate = true;
    } else {
        if (pvalue.val.boolValue) {
            pvalue.val.intValue = 1;
            pvalue.val.doubleValue = 1.0;
        }
    }
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.type = API_PropertyStringValueType;
    pvalue.type = API_PropertyStringValueType;
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertBoolToParamValue (ParamValue &pvalue, const GS::UniString &paramName, const bool boolValue) {
    if (pvalue.name.IsEmpty ())
        pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = GDLNAMEPREFIX;
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        pvalue.typeinx = GDLTYPEINX;
    }
    pvalue.val.type = API_PropertyBooleanValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.boolValue = boolValue;
    const Int32 iseng = ID_ADDON_STRINGS + isEng ();
    if (pvalue.val.boolValue) {
        pvalue.val.uniStringValue = RSGetIndString (iseng, TrueId, ACAPI_GetOwnResModule ());
        pvalue.val.intValue = 1;
        pvalue.val.doubleValue = 1.0;
    } else {
        pvalue.val.uniStringValue = RSGetIndString (iseng, FalseId, ACAPI_GetOwnResModule ());
        pvalue.val.intValue = 0;
        pvalue.val.doubleValue = 0.0;
    }
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.canCalculate = true;
    pvalue.isValid = true;
    if (pvalue.val.formatstring.isEmpty)
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация аттрибута в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertAttributeToParamValue (ParamValue &pvalue,
                                                 const GS::UniString &paramName,
                                                 const API_Attribute attr) {
    if (pvalue.name.IsEmpty ())
        pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = ATTRIBNAMEPREFIX;
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
    }
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    pvalue.val.intValue = attr.header.index.ToInt32_Deprecated ();
#else
    pvalue.val.intValue = attr.header.index;
#endif
    pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    if (pvalue.val.intValue > 0)
        pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString (attr.header.name);
    pvalue.val.type = API_PropertyStringValueType;
    pvalue.type = API_PropertyStringValueType;
    pvalue.isValid = true;
    pvalue.fromAttribElement = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация целого числа в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertIntToParamValue (ParamValue &pvalue, const GS::UniString &paramName, const Int32 intValue) {
    if (pvalue.name.IsEmpty ())
        pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = GDLNAMEPREFIX;
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        pvalue.typeinx = GDLTYPEINX;
    }
    pvalue.val.type = API_PropertyIntegerValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.canCalculate = true;
    pvalue.val.intValue = intValue;
    pvalue.val.doubleValue = intValue * 1.0;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    if (pvalue.val.intValue > 0)
        pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString::Printf ("%d", intValue);
    pvalue.isValid = true;
    if (pvalue.val.formatstring.isEmpty)
        pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация double в ParamValue
// -----------------------------------------------------------------------------
bool ParamHelpers::ConvertDoubleToParamValue (ParamValue &pvalue,
                                              const GS::UniString &paramName,
                                              const double doubleValue) {
    if (pvalue.name.IsEmpty ())
        pvalue.name = paramName;
    if (pvalue.rawName.IsEmpty ()) {
        pvalue.rawName = GDLNAMEPREFIX;
        pvalue.rawName.Append (paramName.ToLowerCase ());
        pvalue.rawName.Append (BRACEEND);
        pvalue.typeinx = GDLTYPEINX;
    }
    pvalue.val.type = API_PropertyRealValueType;
    pvalue.type = pvalue.val.type;
    pvalue.val.canCalculate = true;
    pvalue.val.intValue = (GS::Int32)doubleValue;
    pvalue.val.doubleValue = doubleValue;
    pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
    pvalue.val.hasrawDouble = true;
    pvalue.val.boolValue = false;
    if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ())
        pvalue.val.boolValue = true;
    pvalue.val.uniStringValue = GS::UniString::Printf ("%.3f", doubleValue);
    pvalue.isValid = true;
    return true;
}

// -----------------------------------------------------------------------------
// Конвертация API_IFCProperty в ParamValue
// -----------------------------------------------------------------------------
#if !defined(AC_29)
bool ParamHelpers::ConvertToParamValue (ParamValue &pvalue, const API_IFCProperty &property) {
    if (pvalue.rawName.IsEmpty () || pvalue.name.IsEmpty ()) {
        GS::UniString fname = property.head.propertySetName + SLASH + property.head.propertyName;
        if (pvalue.rawName.IsEmpty ())
            pvalue.rawName = IFCNAMEPREFIX + fname.ToLowerCase () + BRACEEND;
        if (pvalue.name.IsEmpty ())
            pvalue.name = fname;
    }
    pvalue.isValid = true;
    if (property.head.propertyType == API_IFCPropertySingleValueType) {
        const Int32 iseng = ID_ADDON_STRINGS + isEng ();
        switch (property.singleValue.nominalValue.value.primitiveType) {
        case API_IFCPropertyAnyValueStringType:
            pvalue.val.type = API_PropertyStringValueType;
            pvalue.val.uniStringValue = property.singleValue.nominalValue.value.stringValue;
            pvalue.val.boolValue = !pvalue.val.uniStringValue.IsEmpty ();
            if (UniStringToDouble (pvalue.val.uniStringValue, pvalue.val.doubleValue)) {
                pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
                if (pvalue.val.intValue / 1 < pvalue.val.doubleValue)
                    pvalue.val.intValue += 1;
                pvalue.val.canCalculate = true;
            } else {
                if (pvalue.val.boolValue) {
                    pvalue.val.intValue = 1;
                    pvalue.val.doubleValue = 1.0;
                }
            }
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        case API_IFCPropertyAnyValueRealType:
            pvalue.val.canCalculate = true;
            pvalue.val.type = API_PropertyRealValueType;
            pvalue.val.doubleValue = round (property.singleValue.nominalValue.value.doubleValue * 1000) / 1000;
            if (property.singleValue.nominalValue.value.doubleValue - pvalue.val.doubleValue > 0.001)
                pvalue.val.doubleValue += 0.001;
            pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
            if (pvalue.val.intValue / 1 < pvalue.val.doubleValue)
                pvalue.val.intValue += 1;
            if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ())
                pvalue.val.boolValue = true;
            pvalue.val.uniStringValue = GS::UniString::Printf ("%.3f", pvalue.val.doubleValue);
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        case API_IFCPropertyAnyValueIntegerType:
            pvalue.val.canCalculate = true;
            pvalue.val.type = API_PropertyIntegerValueType;
            pvalue.val.intValue = (GS::Int32)property.singleValue.nominalValue.value.intValue;
            pvalue.val.doubleValue = pvalue.val.intValue * 1.0;
            if (pvalue.val.intValue > 0)
                pvalue.val.boolValue = true;
            pvalue.val.uniStringValue = GS::UniString::Printf ("%d", pvalue.val.intValue);
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        case API_IFCPropertyAnyValueBooleanType:
            pvalue.val.canCalculate = true;
            pvalue.val.type = API_PropertyBooleanValueType;
            pvalue.val.boolValue = property.singleValue.nominalValue.value.boolValue;
            if (pvalue.val.boolValue) {
                pvalue.val.uniStringValue = RSGetIndString (iseng, TrueId, ACAPI_GetOwnResModule ());
                pvalue.val.intValue = 1;
                pvalue.val.doubleValue = 1.0;
            } else {
                pvalue.val.uniStringValue = RSGetIndString (iseng, FalseId, ACAPI_GetOwnResModule ());
                pvalue.val.intValue = 0;
                pvalue.val.doubleValue = 0.0;
            }
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        case API_IFCPropertyAnyValueLogicalType:
            pvalue.val.formatstring = FormatStringFunc::ParseFormatString (DEFULTINTFSTRING);
            pvalue.val.type = API_PropertyBooleanValueType;
            if (property.singleValue.nominalValue.value.intValue == 0)
                pvalue.val.boolValue = false;
            if (property.singleValue.nominalValue.value.intValue == 1)
                pvalue.isValid = false;
            if (property.singleValue.nominalValue.value.intValue == 2)
                pvalue.val.boolValue = true;
            if (pvalue.val.boolValue) {
                pvalue.val.uniStringValue = RSGetIndString (iseng, TrueId, ACAPI_GetOwnResModule ());
                pvalue.val.intValue = 1;
                pvalue.val.doubleValue = 1.0;
            } else {
                pvalue.val.uniStringValue = RSGetIndString (iseng, FalseId, ACAPI_GetOwnResModule ());
                pvalue.val.intValue = 0;
                pvalue.val.doubleValue = 0.0;
            }
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
            break;
        default:
            pvalue.val.canCalculate = false;
            pvalue.isValid = false;
            break;
        }
    }
    pvalue.type = pvalue.val.type;
    pvalue.typeinx = IFCTYPEINX;
    return pvalue.isValid;
}
#endif

void ParamHelpers::ConvertByFormatString (ParamValue &pvalue) {
    if (pvalue.val.type == API_PropertyRealValueType || pvalue.val.type == API_PropertyIntegerValueType) {
        Int32 n_zero = pvalue.val.formatstring.n_zero;
        Int32 krat = pvalue.val.formatstring.krat;
        double koeff = pvalue.val.formatstring.koeff;
        bool trim_zero = pvalue.val.formatstring.trim_zero;
        if (!pvalue.val.hasrawDouble) {
            pvalue.val.rawDoubleValue = pvalue.val.doubleValue;
            pvalue.val.hasrawDouble = true;
        }
        pvalue.val.uniStringValue = ParamHelpers::ToString (pvalue);
        if (koeff != 1)
            n_zero = n_zero + (GS::Int32)log10 (koeff);
        pvalue.val.doubleValue = round_nzero (pvalue.val.doubleValue, n_zero);
        pvalue.val.intValue = (GS::Int32)pvalue.val.doubleValue;
        if (fabs (pvalue.val.doubleValue) > std::numeric_limits<double>::epsilon ())
            pvalue.val.boolValue = true;
    }
}

GS::UniString ParamHelpers::ToString (const ParamValue &pvalue) {
    return ParamHelpers::ToString (pvalue, pvalue.val.formatstring);
}

GS::UniString ParamHelpers::ToString (const ParamValue &pvalue, const FormatString &stringformat) {
    switch (pvalue.val.type) {
    case API_PropertyIntegerValueType:
        return FormatStringFunc::NumToString (pvalue.val.intValue, stringformat);
    case API_PropertyRealValueType:
        if (stringformat.needRound) {
            return FormatStringFunc::NumToString (pvalue.val.doubleValue, stringformat);
        } else {
            return FormatStringFunc::NumToString (pvalue.val.rawDoubleValue, stringformat);
        }
    case API_PropertyStringValueType:
        return pvalue.val.uniStringValue;
    case API_PropertyBooleanValueType:
        return GS::ValueToUniString (pvalue.val.boolValue);
    default:
        DBBREAK ();
        return "Invalid Value";
    }
}

// --------------------------------------------------------------------
// Получение данных из однородной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::ComponentsBasicStructure (const API_AttributeIndex &constrinx,
                                             double fillThick,
                                             const API_AttributeIndex &constrinx_ven,
                                             double fillThick_ven,
                                             double fillThick_min,
                                             ParamDictValue &params,
                                             ParamDictComposite &paramcomposite,
                                             ParamDictValue &paramsAdd,
                                             short &structype_ven,
                                             double &width,
                                             double &length) {
#if defined(TESTING)
    DBprnt ("        ComponentsBasicStructure\n");
#endif
    ParamComposite param_composite = {};
    if (fillThick_ven > 0.0001) {
        ParamValueComposite layer = {};
        layer.inx = constrinx_ven;
        layer.fillThick = fillThick_ven;
        layer.num = 2;
        layer.structype = structype_ven;
        layer.length = length;
        layer.width = width;
        param_composite.composite.Push (std::move (layer));
        ParamHelpers::GetAttributeValues (constrinx_ven, params, paramsAdd);
    }
    ParamValueComposite layer = {};
    layer.inx = constrinx;
    layer.fillThick = fillThick;
    layer.fillThick_min = fillThick_min;
    layer.length = length;
    layer.width = width;
    layer.num = 1;
    layer.structype = APICWallComp_Core;
    param_composite.composite.Push (std::move (layer));
    ParamHelpers::GetAttributeValues (constrinx, params, paramsAdd);
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &c = cIt->value;
#else
        ParamComposite &c = *cIt->value;
#endif
        c.composite = param_composite.composite;
    }
    return true;
}

void ParamHelpers::ComponentsGetUnic (GS::Array<ParamValueComposite> &composite) {
    GS::Array<ParamValueComposite> p = {};
    GS::HashTable<GS::UniString, ParamValueComposite> existsmaterial = {};
    for (const auto &c : composite) {
        GS::UniString key = GS::UniString::Printf ("%d", c.inx) + GS::UniString::Printf ("_%.4f", c.fillThick);
        if (existsmaterial.ContainsKey (key)) {
            ParamValueComposite &e = existsmaterial.Get (key);
            e.area += c.area;
            e.volume += c.volume;
            e.area_fill += c.area_fill;
            e.length += c.length;
            e.qty += c.qty;
        } else {
            existsmaterial.Put (key, c);
        }
    }
    if (existsmaterial.IsEmpty ())
        return;
    p.SetCapacity (existsmaterial.GetSize ());
    for (GS::HashTable<GS::UniString, ParamValueComposite>::PairIterator cIt = existsmaterial.EnumeratePairs ();
         cIt != NULL;
         ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValueComposite &c = cIt->value;
#else
        ParamValueComposite &c = *cIt->value;
#endif
        p.Push (std::move (c));
    }
    composite = std::move (p);
    return;
}

// --------------------------------------------------------------------
// Получение данных из многослойной конструкции
// --------------------------------------------------------------------
bool ParamHelpers::ComponentsCompositeStructure (const API_Guid &elemguid,
                                                 API_AttributeIndex &constrinx,
                                                 ParamDictValue &params,
                                                 ParamDictComposite &paramcomposite,
                                                 ParamDictValue &paramsAdd,
                                                 double &width,
                                                 double &length) {
#if defined(TESTING)
    DBprnt ("        ComponentsCompositeStructure");
#endif
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 key = constrinx.ToInt32_Deprecated ();
#else
    Int32 key = (Int32)constrinx;
#endif
    auto &cache = PROPERTYCACHE ().compositeCache;
    if (!cache.ContainsKey (key)) {
#if defined(TESTING)
        DBprnt ("        ComponentsCompositeStructure get new");
#endif
        API_Attribute attrib = {};
        API_AttributeDef defs = {};
        attrib.header.index = constrinx;
        attrib.header.typeID = API_CompWallID;
        GSErrCode err = ACAPI_Attribute_Get (&attrib);
        if (err != NoError) {
            msg_rep ("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_Get", err, elemguid);
            return false;
        }
        err = ACAPI_Attribute_GetDef (attrib.header.typeID, attrib.header.index, &defs);
        if (err != NoError) {
            msg_rep ("materialString::ComponentsCompositeStructure", " ACAPI_Attribute_GetDef", err, elemguid);
            ACAPI_DisposeAttrDefsHdls (&defs);
            return false;
        }
        GS::Array<CachedLayer> cachedLayers;
        cachedLayers.SetCapacity (attrib.compWall.nComps);
        for (short i = 0; i < attrib.compWall.nComps; i++) {
            CachedLayer cl;
            cl.buildingMaterial = (*defs.cwall_compItems)[i].buildingMaterial;
            cl.fillThick = (*defs.cwall_compItems)[i].fillThick;
            cl.flagBits = (*defs.cwall_compItems)[i].flagBits;
            cachedLayers.Push (std::move (cl));
        }
        ACAPI_DisposeAttrDefsHdls (&defs);
        cache.Put (key, std::move (cachedLayers));
    }
    const GS::Array<CachedLayer> &layers = cache[key];
    ParamComposite param_composite = {};
    param_composite.composite.SetCapacity (layers.GetSize ());
    short layerNum = 1; // Счетчик для номеров слоев
    for (const auto &cLayer : layers) {
        ParamValueComposite layer = {};
        layer.inx = cLayer.buildingMaterial;
        layer.fillThick = cLayer.fillThick;
        layer.num = layerNum++;
        layer.structype = cLayer.flagBits;
        layer.length = length;
        layer.width = width;
        param_composite.composite.Push (std::move (layer));
        ParamHelpers::GetAttributeValues (cLayer.buildingMaterial, params, paramsAdd);
    }
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamComposite &c = cIt->value;
#else
        ParamComposite &c = *cIt->value;
#endif
        c.composite = param_composite.composite;
    }
    return true;
}

// --------------------------------------------------------------------
// Получение данных из сложного профиля
// --------------------------------------------------------------------

bool ParamHelpers::ComponentsProfileStructure (ProfileVectorImage &profileDescription,
                                               ParamDictValue &params,
                                               ParamDictComposite &paramcomposite,
                                               ParamDictValue &paramsAdd,
                                               double &width,
                                               double &length,
                                               bool &needReadQuantities) {
#if !defined(AC_22) && !defined(AC_23)
    #if defined(TESTING)
    DBprnt ("        ComponentsProfileStructure");
    #endif
    ConstProfileVectorImageIterator profileDescriptionIt (profileDescription);
    GS::HashTable<short, OrientedSegments> lines;    // Для хранения точки начала сечения и линии сечения
    GS::HashTable<short, GS::Array<Sector>> segment; // Для хранения отрезков линий сечения и последующего объединения
    GS::HashTable<short, ParamComposite> param_composite; // Словарь составов для чтения
    GS::Array<ParamValueComposite> composite_all;
    GS::Array<Sector> resSectors;
    composite_all.SetCapacity (paramcomposite.GetSize ());
    // Получаем список перьев в параметрах
    for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
    #if defined(AC_28) || defined(AC_29)
        GS::UniString rawName = cIt->key;
        ParamComposite &paramc = cIt->value;
    #else
        GS::UniString rawName = *cIt->key;
        ParamComposite &paramc = *cIt->value;
    #endif
        short pen = paramc.composite_pen;
        if (pen < 0 && !needReadQuantities)
            needReadQuantities = true;
        OrientedSegments s;
        GS::Array<Sector> segments;
        lines.Put (pen, std::move (s));
        segment.Put (pen, std::move (segments));
        ParamComposite p;
        param_composite.Put (pen, std::move (p));
    }
    #if defined(TESTING)
    if (needReadQuantities)
        DBprnt ("        Quantities ProfileStructure");
    #endif
    bool hasLine = !lines.IsEmpty ();
    bool profilehasLine = false;
    composite_all.Clear ();
    // Ищем полилинию с нужным цветом
    while (!profileDescriptionIt.IsEOI ()) {
        switch (profileDescriptionIt->item_Typ) {
        case SyArc: // Указателем начала линии служит окружность с тем же пером
        {
            const Sy_ArcType *pSyArc = static_cast<const Sy_ArcType *> (profileDescriptionIt);
            short pen = pSyArc->GetExtendedPen ().GetIndex ();
            if (auto *l = lines.GetPtr (pen)) {
                Point2D s = {pSyArc->origC};
                l->start = std::move (s);
            }
        } break;
        case SyLine: // Поиск линий-сечений
        {
            const Sy_LinType *pSyPolyLine = static_cast<const Sy_LinType *> (profileDescriptionIt);
            short pen = pSyPolyLine->GetExtendedPen ().GetIndex ();
            if (auto *s = segment.GetPtr (pen)) {
                Sector line = {pSyPolyLine->begC, pSyPolyLine->endC};
                s->Push (std::move (line));
                profilehasLine = true;
            }
        } break;
        }
        ++profileDescriptionIt;
    }

    // Если линии сечения не найдены - создадим парочку - вертикальную и горизонтальную
    if (!profilehasLine) {
        Point2D p1 = {-1000, 0};
        Point2D p2 = {1000, 0};
        Sector cut1 = {p1, p2};
        OrientedSegments d;
        d.start = p2;
        d.cut_start = p1;
        d.cut_direction = Geometry::SectorVector (cut1);
        lines.Put (20, std::move (d));
        Point2D p3 = {0, -1000};
        Point2D p4 = {0, 1000};
        Sector cut2 = {p3, p4};
        OrientedSegments d2;
        d2.start = p3;
        d2.cut_start = p4;
        d2.cut_direction = Geometry::SectorVector (cut2);
        lines.Put (6, std::move (d2));
        ParamComposite p;
        param_composite.Put (20, std::move (p));
        param_composite.Put (6, std::move (p));
    } else {
        // Проходим по сегментам, соединяем их в одну линию
        for (GS::HashTable<short, GS::Array<Sector>>::PairIterator cIt = segment.EnumeratePairs (); cIt != NULL;
             ++cIt) {
    #if defined(AC_28) || defined(AC_29)
            GS::Array<Sector> &segment = cIt->value;
            auto *pstart = lines.GetPtr (cIt->key);
    #else
            GS::Array<Sector> &segment = *cIt->value;
            auto *pstart = lines.GetPtr (*cIt->key);
    #endif
            if (pstart == nullptr)
                continue;
            Sector cutline;
            double max_r = 0;
            double min_r = 300000;
            for (UInt32 j = 0; j < segment.GetSize (); j++) {
                double r = Geometry::Dist (pstart->start, segment[j].c1);
                if (r > max_r) {
                    cutline.c1 = segment[j].c1;
                    max_r = r;
                }
                if (r < min_r) {
                    cutline.c2 = segment[j].c1;
                    min_r = r;
                }
                r = Geometry::Dist (pstart->start, segment[j].c2);
                if (r > max_r) {
                    cutline.c1 = segment[j].c2;
                    max_r = r;
                }
                if (r < min_r) {
                    cutline.c2 = segment[j].c2;
                    min_r = r;
                }
            }
    #if defined(AC_28) || defined(AC_29)
            if (auto *l = lines.GetPtr (cIt->key)) {
                l->cut_start = cutline.c2;
                l->cut_direction = Geometry::SectorVector (cutline);
            }
    #else
            if (auto *l = lines.GetPtr (*cIt->key)) {
                l->cut_start = cutline.c2;
                l->cut_direction = Geometry::SectorVector (cutline);
            }
    #endif
        }
    }
    bool hasData = false;
    ConstProfileVectorImageIterator profileDescriptionIt1 (profileDescription);
    Point2D startp = {-10000, 0};
    while (!profileDescriptionIt1.IsEOI ()) {
        switch (profileDescriptionIt1->item_Typ) {
        case SyHatch: {
            const HatchObject &syHatch = profileDescriptionIt1;
            short structype = 0;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            const ProfileItem profileItemInfo = syHatch.GetProfileItem ();
            if (profileItemInfo.IsFinish ())
                structype = APICWallComp_Finish;
            if (profileItemInfo.IsCore ())
                structype = APICWallComp_Core;
    #else
            const ProfileItem *profileItemInfo = syHatch.GetProfileItemPtr ();
            if (profileItemInfo->IsFinish ())
                structype = APICWallComp_Finish;
            if (profileItemInfo->IsCore ())
                structype = APICWallComp_Core;
    #endif
            Geometry::MultiPolygon2D result;

            // Получаем полигон штриховки
            if (syHatch.ToPolygon2D (result, HatchObject::VertexAndEdgeData::Omit) == NoError) {
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        #if defined(AC_28) || defined(AC_29)
                API_AttributeIndex constrinxL =
                    ACAPI_CreateAttributeIndex ((GS::Int32)syHatch.GetBuildMatIdx ().ToGSAttributeIndex_Deprecated ());
        #else
                API_AttributeIndex constrinxL =
                    ACAPI_CreateAttributeIndex ((GS::Int32)syHatch.GetBuildMatIdx ().ToGSAttributeIndex ());
        #endif
    #else
                API_AttributeIndex constrinxL = (API_AttributeIndex)syHatch.GetBuildMatIdx ();
    #endif
                // Проходим по полигонам
                for (UInt32 i = 0; i < result.GetSize (); i++) {
                    // Запись всех слоёв
                    double area_fill = result[i].CalcArea ();
                    double perimetr = 0;
                    USize ncont = result[i].GetContourNum ();
                    if (ncont == 1) {
                        perimetr = result[i].CalcPerimeter ();
                    } else {
                        for (UIndex cIndex = 1; cIndex <= ncont; ++cIndex) {
                            typename Geometry::Polygon2D::ConstContourIterator cIt =
                                result[i].GetContourIterator (cIndex);
                            perimetr = fmax (perimetr, result[i].CalcContourPerimeter (cIt));
                        }
                    }
                    Point2D centr = result[i].GetCenter ();
                    if (needReadQuantities) {
                        double th = 0;
                        for (UIndex edgeIndex = 1; edgeIndex <= result[i].GetEdgeNum (); ++edgeIndex) {
                            typename Geometry::Polygon2D::ConstEdgeIterator edgeIt =
                                result[i].GetEdgeIterator (edgeIndex);
                            Sector edge;
                            GenArc _a;
                            result[i].GetSector (edgeIt, edge, _a);
                            GS::Optional<Point2D> ppoint = edge.ProjectPoint (centr);
                            if (ppoint.HasValue ()) {
                                Sector edge_norm = {centr, ppoint.Get ()};
                                Vector2D cut_direction = Geometry::SectorVector (edge_norm);
                                resSectors.Clear ();
                                bool h = result[i].Intersect (centr, cut_direction, &resSectors);
                                for (const auto &sec : resSectors) {
                                    double fillThickL = sec.GetLength ();
                                    if (is_equal (th, 0)) {
                                        th = fillThickL;
                                    } else {
                                        th = fmin (th, fillThickL);
                                    }
                                }
                            }
                        }
                        if (!is_equal (th, 0)) {
                            ParamValueComposite layer = {};
                            layer.inx = constrinxL;
                            layer.fillThick = th;
                            layer.rfromstart = Geometry::Dist (startp, centr);
                            layer.area_fill = area_fill;
                            if (ncont == 1) {
                                layer.width = 0.5 * perimetr - th;
                            } else {
                                layer.width = perimetr;
                            }
                            if (is_equal (layer.width, 0))
                                layer.width = width;
                            layer.length = length;
                            layer.structype = structype;
                            ParamHelpers::GetAttributeValues (constrinxL, params, paramsAdd);
                            composite_all.Push (std::move (layer));
                        } else {
    #if defined(TESTING)
                            DBprnt ("ERR is_equal (th, 0) ====================");
    #endif
                        }
                    }
                    // Находим пересечения каждого полигона с линиями
                    for (GS::HashTable<short, OrientedSegments>::PairIterator cIt = lines.EnumeratePairs ();
                         cIt != NULL;
                         ++cIt) {
    #if defined(AC_28) || defined(AC_29)
                        OrientedSegments &l = cIt->value;
                        short pen = cIt->key;
    #else
                        OrientedSegments &l = *cIt->value;
                        short pen = *cIt->key;
    #endif
                        if (pen < 0)
                            continue;
                        auto *comp = param_composite.GetPtr (pen);
                        if (comp == nullptr)
                            continue;
                        resSectors.Clear ();
                        bool h = result[i].Intersect (l.cut_start, l.cut_direction, &resSectors);
                        for (const auto &sec : resSectors) {
                            double fillThickL = sec.GetLength ();
                            double rfromstart = Geometry::Dist (
                                l.start, sec.GetMidPoint ()); // Расстояние до окружности(начала порядка слоёв)
                            ParamValueComposite layer = {};
                            layer.inx = constrinxL;
                            layer.fillThick = fillThickL;
                            layer.rfromstart = rfromstart;
                            layer.area_fill = area_fill;
                            layer.structype = structype;
                            comp->composite.Push (std::move (layer));
                            ParamHelpers::GetAttributeValues (constrinxL, params, paramsAdd);
                            hasData = true;
                        }
                    }
                }
            } else {
    #if defined(TESTING)
                DBprnt ("ERR == syHatch.ToPolygon2D ====================");
    #endif
            }
        } break;
        }
        ++profileDescriptionIt1;
    }
    if (needReadQuantities && !composite_all.IsEmpty ()) {
        if (auto *p = param_composite.GetPtr (-1)) {
            p->composite = composite_all;
            hasData = true;
        }
        if (auto *p = param_composite.GetPtr (-2)) {
            p->composite = composite_all;
            hasData = true;
        }
    }
    if (hasData) {
        GS::Array<ParamValueComposite> paramout = {};
        for (ParamDictComposite::PairIterator cIt = paramcomposite.EnumeratePairs (); cIt != NULL; ++cIt) {
    #if defined(AC_28) || defined(AC_29)
            ParamComposite &ct = cIt->value;
    #else
            ParamComposite &ct = *cIt->value;
    #endif
            auto *cf = param_composite.GetPtr (ct.composite_pen);
            if (cf == nullptr)
                continue;
            // Номер пера меньше нуля это признак использования состава при подсчёте кол-ва. Порядок там не важен.
            if (cf->composite_pen < 0) {
                ct.composite = cf->composite;
            } else {
                // Теперь нам надо отсортировать слои по параметру rfromstart
                std::map<double, ParamValueComposite> comps = {};
                for (const auto &comp : cf->composite) {
                    comps[comp.rfromstart] = comp;
                }
                paramout.Clear ();
                for (std::map<double, ParamValueComposite>::iterator k = comps.begin (); k != comps.end (); ++k) {
                    paramout.Push (k->second);
                }
                for (UInt32 i = 0; i < paramout.GetSize (); i++) {
                    paramout[i].num = i + 1;
                }
                ct.composite = paramout;
            }
        }
    }
    return hasData;
#else
    return false;
#endif
}

// --------------------------------------------------------------------
// Вытаскивает всё, что может, из информации о составе элемента
// --------------------------------------------------------------------
bool ParamHelpers::Components (const API_Element &element,
                               ParamDictValue &params,
                               ParamDictValue &paramsAdd,
                               ParamDictComposite &paramcomposite,
                               bool &needReadQuantities) {
#if defined(TESTING)
    DBprnt ("        Components");
#endif
    API_ModelElemStructureType structtype = API_BasicStructure;
    API_AttributeIndex constrinx = {};
    double fillThick = 0, width = 0, length = 0;
    double fillThick_min = 0;
    // Отделка колонн
    API_AttributeIndex constrinx_ven = {};
    double fillThick_ven = 0;
    short structype_ven = 0;
    API_Elem_Head elemhead = element.header;
    // Получаем данные о составе конструкции. Т.к. для разных типов элементов
    // информация храница в разных местах - запишем всё в одни переменные
    API_ElemTypeID eltype = GetElemTypeID (elemhead);
    API_ElementMemo memo = {};
    GSErrCode err = NoError;
    switch (eltype) {
#ifndef AC_22
    case API_ColumnSegmentID:
        structtype = element.columnSegment.assemblySegmentData.modelElemStructureType;
        if (structtype == API_BasicStructure) {
            constrinx = element.columnSegment.assemblySegmentData.buildingMaterial;
            fillThick = element.columnSegment.assemblySegmentData.nominalHeight;
            width = element.columnSegment.assemblySegmentData.nominalWidth;
            constrinx_ven = element.columnSegment.venBuildingMaterial;
            fillThick_ven = element.columnSegment.venThick;
            if (element.columnSegment.venType == APIVeneer_Core)
                structype_ven = APICWallComp_Core;
            if (element.columnSegment.venType == APIVeneer_Finish)
                structype_ven = APICWallComp_Finish;
        }
        if (structtype == API_ProfileStructure)
            constrinx = element.columnSegment.assemblySegmentData.profileAttr;
        break;
    case API_BeamSegmentID:
        structtype = element.beamSegment.assemblySegmentData.modelElemStructureType;
        if (structtype == API_BasicStructure) {
            constrinx = element.beamSegment.assemblySegmentData.buildingMaterial;
            fillThick = element.beamSegment.assemblySegmentData.nominalHeight;
            width = element.beamSegment.assemblySegmentData.nominalWidth;
        }
        if (structtype == API_ProfileStructure)
            constrinx = element.beamSegment.assemblySegmentData.profileAttr;
        break;
#endif
    case API_ColumnID:
#ifdef AC_22
        constrinx = element.column.buildingMaterial;
#else
        if (element.header.guid == APINULLGuid) {
            return false;
        }
        if (element.column.nSegments == 1) {
            BNZeroMemory (&memo, sizeof (API_ElementMemo));
            err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_ColumnSegment);
            if (err == NoError && memo.columnSegments != nullptr) {
                elemhead = memo.columnSegments[0].head;
                structtype = memo.columnSegments[0].assemblySegmentData.modelElemStructureType;
                if (structtype == API_BasicStructure) {
                    constrinx = memo.columnSegments[0].assemblySegmentData.buildingMaterial;
                    fillThick = memo.columnSegments[0].assemblySegmentData.nominalHeight;
                    width = memo.columnSegments[0].assemblySegmentData.nominalWidth;
                    length = element.column.height;
                    constrinx_ven = memo.columnSegments[0].venBuildingMaterial;
                    fillThick_ven = memo.columnSegments[0].venThick;
                    if (memo.columnSegments[0].venType == APIVeneer_Core)
                        structype_ven = APICWallComp_Core;
                    if (memo.columnSegments[0].venType == APIVeneer_Finish)
                        structype_ven = APICWallComp_Finish;
                }
                if (structtype == API_ProfileStructure)
                    constrinx = memo.columnSegments[0].assemblySegmentData.profileAttr;
            } else {
                msg_rep ("ParamHelpers::Components", "ACAPI_Element_GetMemo - ColumnSegment", err, element.header.guid);
                ACAPI_DisposeElemMemoHdls (&memo);
                return false;
            }
        } else {
            msg_rep ("ParamHelpers::Components", "Multisegment column not supported", NoError, element.header.guid);
            return false;
        }
#endif
        break;
    case API_BeamID:
#ifdef AC_22
        constrinx = element.beam.buildingMaterial;
#else
        if (element.header.guid == APINULLGuid) {
            return false;
        }
        if (element.beam.nSegments == 1) {
            BNZeroMemory (&memo, sizeof (API_ElementMemo));
            err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_BeamSegment);
            if (err == NoError && memo.beamSegments != nullptr) {
                elemhead = memo.beamSegments[0].head;
                structtype = memo.beamSegments[0].assemblySegmentData.modelElemStructureType;
                if (structtype == API_BasicStructure) {
                    width = memo.beamSegments[0].assemblySegmentData.nominalWidth;
                    constrinx = memo.beamSegments[0].assemblySegmentData.buildingMaterial;
                    fillThick = memo.beamSegments[0].assemblySegmentData.nominalHeight;
                }
                if (element.beam.isFlipped) {
                    length = sqrt (
                        (element.beam.endC.x - element.beam.begC.x) * (element.beam.endC.x - element.beam.begC.x) +
                        (element.beam.endC.y - element.beam.begC.y) * (element.beam.endC.y - element.beam.begC.y));
                } else {
                    length = sqrt (
                        (element.beam.begC.x - element.beam.endC.x) * (element.beam.begC.x - element.beam.endC.x) +
                        (element.beam.begC.y - element.beam.endC.y) * (element.beam.begC.y - element.beam.endC.y));
                }
                if (structtype == API_ProfileStructure)
                    constrinx = memo.beamSegments[0].assemblySegmentData.profileAttr;
            } else {
                msg_rep ("ParamHelpers::Components", "ACAPI_Element_GetMemo - BeamSegment", err, element.header.guid);
                ACAPI_DisposeElemMemoHdls (&memo);
                return false;
            }
        } else {
            msg_rep ("ParamHelpers::Components", "Multisegment beam not supported", NoError, element.header.guid);
            return false;
        }
#endif
        break;
    case API_WallID:
        structtype = element.wall.modelElemStructureType;
        width = element.wall.height;
        if (element.wall.flipped) {
            length = sqrt ((element.wall.endC.x - element.wall.begC.x) * (element.wall.endC.x - element.wall.begC.x) +
                           (element.wall.endC.y - element.wall.begC.y) * (element.wall.endC.y - element.wall.begC.y));
        } else {
            length = sqrt ((element.wall.begC.x - element.wall.endC.x) * (element.wall.begC.x - element.wall.endC.x) +
                           (element.wall.begC.y - element.wall.endC.y) * (element.wall.begC.y - element.wall.endC.y));
        }
        if (structtype == API_CompositeStructure)
            constrinx = element.wall.composite;
        if (structtype == API_BasicStructure)
            constrinx = element.wall.buildingMaterial;
        if (structtype == API_ProfileStructure)
            constrinx = element.wall.profileAttr;
        fillThick = element.wall.thickness;
        break;
    case API_SlabID:
        structtype = element.slab.modelElemStructureType;
        if (structtype == API_CompositeStructure)
            constrinx = element.slab.composite;
        if (structtype == API_BasicStructure)
            constrinx = element.slab.buildingMaterial;
        fillThick = element.slab.thickness;
        break;
    case API_RoofID:
        structtype = element.roof.shellBase.modelElemStructureType;
        if (structtype == API_CompositeStructure)
            constrinx = element.roof.shellBase.composite;
        if (structtype == API_BasicStructure)
            constrinx = element.roof.shellBase.buildingMaterial;
        fillThick = element.roof.shellBase.thickness;
        break;
    case API_ShellID:
        structtype = element.shell.shellBase.modelElemStructureType;
        if (structtype == API_CompositeStructure)
            constrinx = element.shell.shellBase.composite;
        if (structtype == API_BasicStructure)
            constrinx = element.shell.shellBase.buildingMaterial;
        fillThick = element.shell.shellBase.thickness;
        break;
    case API_MeshID:
        constrinx = element.mesh.buildingMaterial;
        fillThick = 0;
        fillThick_min = element.mesh.skirtLevel;
        BNZeroMemory (&memo, sizeof (API_ElementMemo));
        err = ACAPI_Element_GetMemo (element.header.guid, &memo, APIMemoMask_MeshPolyZ);
        if (err == NoError && memo.meshPolyZ != nullptr) {
            Int32 nMeshLevels = BMGetHandleSize ((GSHandle)memo.meshPolyZ) / sizeof (double);
            for (Int32 i = 1; i < nMeshLevels; i++) {
                fillThick = fmax (fillThick, element.mesh.skirtLevel + (*memo.meshPolyZ)[i]);
                fillThick_min = fmin (fillThick_min, element.mesh.skirtLevel + (*memo.meshPolyZ)[i]);
            }
        } else {
            fillThick = element.mesh.skirtLevel;
            fillThick_min = element.mesh.skirtLevel;
        }
        break;
    case API_MorphID:
        constrinx = element.morph.buildingMaterial;
        fillThick = 0;
        structtype = API_CompositeStructure;
        if (!needReadQuantities)
            return false;
        break;
    case API_ObjectID:
        structtype = API_CompositeStructure;
        if (!needReadQuantities)
            return false;
        break;
    default:
        return false;
        break;
    }
    ACAPI_DisposeElemMemoHdls (&memo);

    // Типов вывода слоёв может быть насколько - для сложных профилей, для учёта несущих/ненесущих слоёв
    // Получим словарь исключительно с определениями состава
    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        if (!param.fromMaterial)
            continue;
        param.eltype = eltype;
        if (!param.rawName.Contains ("{@material:layers"))
            continue;
        if (paramcomposite.ContainsKey (param.rawName))
            continue;
        ParamComposite p = {};
        p.eltype = eltype;
        p.templatestring = param.val.uniStringValue;
        p.composite_type = structtype;
        p.composite_pen = param.composite_pen;
        p.fromGuid = element.header.guid;
        p.hasFormula = param.val.hasFormula;
        p.fromQuantity = param.fromQuantity;
        paramcomposite.Put (param.rawName, std::move (p));
    }

    // Если ничего нет - слои нам всё равно нужны
    if (paramcomposite.IsEmpty () && (structtype == API_BasicStructure || structtype == API_CompositeStructure)) {
        ParamComposite p = {};
        p.fromGuid = element.header.guid;
        p.composite_pen = 20;
        p.composite_type = structtype;
        p.eltype = eltype;
        paramcomposite.Put ("{@material:layers,20}", std::move (p));
    }

    if (needReadQuantities && eltype == API_ObjectID)
        return true;
    bool hasData = false;

    // Получим индексы строительных материалов и толщины
    if (structtype == API_BasicStructure) {
        hasData = ParamHelpers::ComponentsBasicStructure (constrinx,
                                                          fillThick,
                                                          constrinx_ven,
                                                          fillThick_ven,
                                                          fillThick_min,
                                                          params,
                                                          paramcomposite,
                                                          paramsAdd,
                                                          structype_ven,
                                                          width,
                                                          length);
    }
    if (structtype == API_CompositeStructure)
        hasData = ParamHelpers::ComponentsCompositeStructure (
            elemhead.guid, constrinx, params, paramcomposite, paramsAdd, width, length);
#ifndef AC_23
    if (structtype == API_ProfileStructure) {
        API_ElementMemo memo = {};
        UInt64 mask = APIMemoMask_StretchedProfile;
        GSErrCode err = ACAPI_Element_GetMemo (elemhead.guid, &memo, mask);
        if (err != NoError) {
            ACAPI_DisposeElemMemoHdls (&memo);
            msg_rep ("ParamHelpers::Components", "err", err, element.header.guid);
            return false;
        }
        if (memo.stretchedProfile == nullptr) {
            ACAPI_DisposeElemMemoHdls (&memo);
            msg_rep ("ParamHelpers::Components", "Profile not found, missing attribute", NoError, element.header.guid);
            return false;
        }
        ProfileVectorImage profileDescription = *memo.stretchedProfile;
        hasData = ParamHelpers::ComponentsProfileStructure (
            profileDescription, params, paramcomposite, paramsAdd, width, length, needReadQuantities);
        ACAPI_DisposeElemMemoHdls (&memo);
    }
#endif
    return hasData;
}

// --------------------------------------------------------------------
// Заполнение данных для одного слоя
// --------------------------------------------------------------------
bool ParamHelpers::GetAttributeValues (const API_AttributeIndex &constrinx,
                                       ParamDictValue &params,
                                       ParamDictValue &paramsAdd) {
#if defined(AC_27) || defined(AC_28) || defined(AC_29)
    Int32 constrinxvalue = constrinx.ToInt32_Deprecated ();
#else
    Int32 constrinxvalue = (Int32)constrinx;
#endif
    GS::UniString attribsuffix = GS::UniString::Printf ("%d", constrinxvalue);

    bool isClassRead = false;   // была попытка чтения классификации
    bool isClassReadOk = false; // Классификация прочитана
    bool isBmatAttribReadOk = false;

    struct PendingAdd {
        API_Guid guid;
        ParamValue value;
    };

    GS::Array<PendingAdd> pendingAdds;
    GS::UniString name = "";
    API_Attribute attrib = {};
    GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs = {};
    GS::Array<GS::UniString> systemNamesLower;
    API_ClassificationItem cl = {};
    GS::UniString name_fill = "";
    API_Attribute attribt_fill = {};
    GS::UniString k = "{@material:cutfill_inx}";
    if (const ParamValue *foundParam = params.GetPtr (k)) {
        ParamValue pvalue_bmat = {};
        pvalue_bmat.rawName = k;
        pvalue_bmat.name = "cutfill_inx";
        pvalue_bmat.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, constrinxvalue);
        pvalue_bmat.fromMaterial = true;
        ParamHelpers::AddParamValue2ParamDict (foundParam->fromGuid, pvalue_bmat, params);
    }

    k = "{@material:bmat_inx}";
    if (const ParamValue *foundParam = params.GetPtr (k)) {
        ParamValue pvalue_bmat = {};
        pvalue_bmat.rawName = k;
        pvalue_bmat.name = "bmat_inx";
        pvalue_bmat.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
        ParamHelpers::ConvertIntToParamValue (pvalue_bmat, pvalue_bmat.name, constrinxvalue);
        pvalue_bmat.fromMaterial = true;
        ParamHelpers::AddParamValue2ParamDict (foundParam->fromGuid, pvalue_bmat, params);
    }

    auto getAttribute = [] (API_AttrTypeID typeID,
                            const API_AttributeIndex &index,
                            GS::UniString &outName,
                            API_Attribute &outAttrib) -> bool {
        outAttrib = {};
        outAttrib.header.typeID = typeID;
        outAttrib.header.index = index;
        outAttrib.header.uniStringNamePtr = &outName;
        GSErrCode err = ACAPI_Attribute_Get (&outAttrib);
        if (err != NoError) {
            msg_rep ("materialString::GetAttributeValues", "ACAPI_Attribute_Get", err, APINULLGuid);
            return false;
        }
        return true;
    };

    auto ensureClassificationsRead = [&] () -> bool {
        if (isClassRead)
            return isClassReadOk;
        isClassRead = true;
        if (!isBmatAttribReadOk) {
            if (!getAttribute (API_BuildingMaterialID, constrinx, name, attrib)) {
                return false;
            } else {
                isBmatAttribReadOk = true;
            }
        }
        GSErrCode err = ACAPI_Attribute_GetClassificationItems (attrib.header, systemItemPairs);
        if (err != NoError) {
            msg_rep ("materialString::GetAttributeValues", "ACAPI_Attribute_GetClassificationItems", err, APINULLGuid);
            isClassReadOk = false;
        } else {
            isClassReadOk = !systemItemPairs.IsEmpty ();
            if (isClassReadOk) {
                systemNamesLower.SetCapacity (systemItemPairs.GetSize ());
                for (const auto &s : systemItemPairs) {
                    GS::UniString nm = ClassificationFunc::GetSystemName (s.first);
                    nm.SetToLowerCase ();
                    systemNamesLower.Push (std::move (nm));
                }
            }
        }
        return isClassReadOk;
    };
    // === Добавление классификаций, если необходимо
    GS::UniString clname = "{@property:material class name";
    GS::UniString clId = "{@property:material class id";
    GS::UniString cldesc = "{@property:material class description";
    GS::UniString keyId = clId + BRACEEND;
    GS::UniString keyName = clname + BRACEEND;
    GS::UniString keyDesc = cldesc + BRACEEND;
    bool hasId = params.ContainsKey (keyId);
    bool hasName = params.ContainsKey (keyName);
    bool hasDesc = params.ContainsKey (keyDesc);

    if ((hasId || hasName || hasDesc) && ensureClassificationsRead ()) {
        cl = ClassificationFunc::FindClass (systemItemPairs[0]);
        if (hasId && !cl.id.IsEmpty ()) {
            ParamValue pvalue = params.Get (keyId);
            ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.id);
            pvalue.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
            pvalue.fromMaterial = true;
            pvalue.fromAttribDefinition = false;
            ParamHelpers::AddParamValue2ParamDict (pvalue.fromGuid, pvalue, params);
        }
        if (hasName && !cl.name.IsEmpty ()) {
            ParamValue pvalue = params.Get (keyName);
            ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.name);
            pvalue.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
            pvalue.fromMaterial = true;
            pvalue.fromAttribDefinition = false;
            ParamHelpers::AddParamValue2ParamDict (pvalue.fromGuid, pvalue, params);
        }
        if (hasDesc && !cl.description.IsEmpty ()) {
            ParamValue pvalue = params.Get (keyDesc);
            ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.description);
            pvalue.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
            pvalue.fromMaterial = true;
            pvalue.fromAttribDefinition = false;
            ParamHelpers::AddParamValue2ParamDict (pvalue.fromGuid, pvalue, params);
        }
    }

    // Определения и свойства для элементов
    bool flag_find = false;
    GS::Array<API_PropertyDefinition> propertyDefinitions;
    GS::Array<GS::UniString> sr;
    GS::Array<GS::UniString> local_scratch;

    for (ParamDictValue::PairIterator cIt = params.EnumeratePairs (); cIt != NULL; ++cIt) {
#if defined(AC_28) || defined(AC_29)
        ParamValue &param = cIt->value;
#else
        ParamValue &param = *cIt->value;
#endif
        // пропуск уже прочитанного
        if (param.isValid) {
            continue;
        }

        // пропуск прочитанного с ошибкой
        if (param.rawName.Contains (CharENTER)) {
#if defined(TESTING)
            if (!param.rawName.Contains (SYNCNAME))
                DBprnt ("GetAttributeValues err - param.rawName.Contains (CharENTER)", param.rawName);
#endif
            continue;
        } else if (param.definition.name.Contains (CharENTER)) {
#if defined(TESTING)
            if (!param.rawName.Contains (SYNCNAME))
                DBprnt ("GetAttributeValues err - param.definition.name.Contains (CharENTER)", param.rawName);
#endif
            continue;
        }

        if (param.rawName.Contains (SEMICOLON)) {
            bool hasIdLocal = param.rawName.Contains (clId);
            bool hasNameLocal = param.rawName.Contains (clname);
            bool hasDescLocal = param.rawName.Contains (cldesc);
            if (hasIdLocal || hasNameLocal || hasDescLocal) {
                if (ensureClassificationsRead ()) {
                    ParamValue pvalue = param;
                    pvalue.rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
                    GS::Pair<API_Guid, API_Guid> classitem = systemItemPairs[0];
                    GS::UniString systemnameindes = "";
                    sr.Clear ();

                    UInt32 nsr = StringSplt (param.rawName, SEMICOLON, sr, true, &local_scratch);
                    if (nsr > 1) {
                        systemnameindes = sr[1];
                    } else {
                        continue;
                    }
                    systemnameindes.ReplaceAll (BRACEEND, EMPTYSTRING);
                    systemnameindes.Trim ();
                    systemnameindes.SetToLowerCase ();
                    for (UIndex idx = 0; idx < systemItemPairs.GetSize (); ++idx) {
                        if (systemnameindes == systemNamesLower[idx]) {
                            classitem = systemItemPairs[idx];
                            break;
                        }
                    }
                    cl = ClassificationFunc::FindClass (classitem);
                    if (hasIdLocal && !cl.id.IsEmpty ()) {
                        ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.id);
                        pendingAdds.Push ({pvalue.fromGuid, pvalue});
                    } else if (hasNameLocal && !cl.name.IsEmpty ()) {
                        ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.name);
                        pendingAdds.Push ({pvalue.fromGuid, pvalue});
                    } else if (hasDescLocal && !cl.description.IsEmpty ()) {
                        ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.rawName, cl.description);
                        pendingAdds.Push ({pvalue.fromGuid, pvalue});
                    }
                    pvalue.fromMaterial = true;
                    pvalue.fromAttribDefinition = false;
                    flag_find = true;
                }
                continue;
            }
        }

        if (!param.fromAttribDefinition)
            continue;

        // пропуск пустых ствойств
        if (param.definition.guid == APINULLGuid) {
#if defined(TESTING)
            DBprnt ("GetAttributeValues err - param.definition.guid == APINULLGuid", param.rawName);
#endif
            continue;
        }

        GS::UniString rawName = param.rawName;
        rawName.ReplaceAll (BRACEEND, CharENTER + attribsuffix + BRACEEND);
        if (const auto *p = params.GetPtr (rawName)) {
            if (p->isValid) {
                continue; // Это уже прочитанное валидное свойство
            } else {
#if defined(TESTING)
                if (!param.rawName.Contains (SYNCNAME))
                    DBprnt ("GetAttributeValues err - !p->isValid", rawName);
#endif
            }
        }

        // Обработка свойства штриховки
        if (param.rawName.Contains ("buildingmaterialproperties/building material cutfill")) {
            if (!isBmatAttribReadOk) {
                if (!getAttribute (API_BuildingMaterialID, constrinx, name, attrib)) {
#if defined(TESTING)
                    DBprnt ("GetAttributeValues err - getAttribute API_BuildingMaterialID", attribsuffix);
#endif
                } else {
                    isBmatAttribReadOk = true;
                }
            }
            if (isBmatAttribReadOk &&
                getAttribute (API_FilltypeID, attrib.buildingMaterial.cutFill, name_fill, attribt_fill)) {
                ParamValue pvalue = {};
                pvalue.name = param.name + CharENTER + attribsuffix;
                pvalue.rawName = rawName;
                ParamHelpers::ConvertStringToParamValue (pvalue, pvalue.name, name_fill);
                pvalue.fromMaterial = true;
                pendingAdds.Push ({param.fromGuid, std::move (pvalue)});
                flag_find = true;
            } else {
#if defined(TESTING)
                DBprnt ("GetAttributeValues err - getAttribute API_FilltypeID", param.name);
#endif
            }
            continue;
        }
        propertyDefinitions.Push (param.definition);
    }

    // Применяем все отложенные добавления
    for (auto &pa : pendingAdds) {
        ParamHelpers::AddParamValue2ParamDict (pa.guid, pa.value, params);
    }

#ifndef AC_22
    if (propertyDefinitions.IsEmpty ())
        return flag_find;
    if (!isBmatAttribReadOk) {
        if (!getAttribute (API_BuildingMaterialID, constrinx, name, attrib)) {
    #if defined(TESTING)
            DBprnt ("GetAttributeValues err - getAttribute API_BuildingMaterialID", attribsuffix);
    #endif
            return false;
        } else {
            isBmatAttribReadOk = true;
        }
    }
    GS::Array<API_Property> properties;
    GSErrCode error = ACAPI_Attribute_GetPropertyValues (attrib.header, propertyDefinitions, properties);
    if (error != NoError) {
        msg_rep ("materialString::GetAttributeValues", "ACAPI_Attribute_GetPropertyValues", error, APINULLGuid);
        return flag_find;
    };
    for (auto &property : properties) {
        property.definition.name = property.definition.name + CharENTER + attribsuffix;
        GS::UniString val = PropertyHelpers::ToString (property);
        if (val.Count (STRINGPROC) > 1 || (val.Contains (BRACESTART) && val.Contains (BRACEEND))) {
            if (ParamHelpers::ParseParamNameMaterial (val, paramsAdd)) {
                property.value.singleVariant.variant.uniStringValue = val;
                property.isDefault = false;
            }
        }
    }
    return (ParamHelpers::AddProperty (params, properties, APINULLGuid) || flag_find);
#endif
    return flag_find;
}
