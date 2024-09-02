//------------ kuvbur 2022 ------------
#include    "CommonFunction.hpp"
#include    "StringConversion.hpp"
#include    <cmath>
#include    <limits>
#include    <math.h>

// -----------------------------------------------------------------------------
// Проверка языка Архикада. Для INT возвращает 1000
// -----------------------------------------------------------------------------
Int32 isEng ()
{
    GSErrCode err = NoError;
    API_ServerApplicationInfo AppInfo;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_AddOnIdentification_Application (&AppInfo);
#else
    err = ACAPI_Environment (APIEnv_ApplicationID, &AppInfo);
#endif // AC_27
    if (err != NoError) return 0;
    if (!AppInfo.language.IsEqual ("RUS")) return 1000;
    return 0;
}

// -----------------------------------------------------------------------------
// Вывод сообщения в отчёт
// -----------------------------------------------------------------------------
void msg_rep (const GS::UniString& modulename, const GS::UniString& reportString, const GSErrCode& err, const API_Guid& elemGuid)
{
    GS::UniString error_type = "";
    if (err != NoError) {
        switch (err) {
            case APIERR_GENERAL:
                error_type = "APIERR_GENERAL - General error code";
                break;
            case APIERR_MEMFULL:
                error_type = "APIERR_MEMFULL Insufficient memory.";
                break;
            case APIERR_CANCEL:
                error_type = "APIERR_CANCEL The operation has been canceled by the user, in case of a long process.";
                break;
            case APIERR_BADID:
                error_type = "APIERR_BADID The passed identifier is not a valid one, or valid, but not proper for the given operation.";
                break;
            case APIERR_BADINDEX:
                error_type = "APIERR_BADINDEX The passed index is out of range.";
                break;
            case APIERR_BADNAME:
                error_type = "APIERR_BADNAME The passed name is not proper or not found in the existing list.";
                break;
            case APIERR_BADPARS:
                error_type = "APIERR_BADPARS The passed parameters are inconsistent.";
                break;
            case APIERR_BADPOLY:
                error_type = "APIERR_BADPOLY The passed polygon cannot be interpreted.";
                break;
            case APIERR_BADDATABASE:
                error_type = "APIERR_BADDATABASE The command cannot be executed on the current database.";
                break;
            case APIERR_BADWINDOW:
                error_type = "APIERR_BADWINDOW The command cannot be executed while the current window is active.";
                break;
            case APIERR_BADKEYCODE:
                error_type = "APIERR_BADKEYCODE The key code cannot be found in the listing database.";
                break;
            case APIERR_BADPLATFORMSIGN:
                error_type = "APIERR_BADPLATFORMSIGN The passed platform sign is not valid";
                break;
            case APIERR_BADPLANE:
                error_type = "APIERR_BADPLANE The plane equation is incorrect.";
                break;
            case APIERR_BADUSERID:
                error_type = "APIERR_BADUSERID The passed user ID(TeamWork client) is not valid.";
                break;
            case APIERR_BADVALUE:
                error_type = "APIERR_BADVALUE The passed autotext value is not valid";
                break;
            case APIERR_BADELEMENTTYPE:
                error_type = "APIERR_BADELEMENTTYPE The function cannot be applied to the passed element type";
                break;
            case APIERR_IRREGULARPOLY:
                error_type = "APIERR_IRREGULARPOLY The passed polygon or polyline is irregular.See API_RegularizedPoly.";
                break;
            case APIERR_BADEXPRESSION:
                error_type = "The passed expression string is syntactically incorrect.";
                break;
            case -2130313100:
                error_type = "The passed guid is invalid or valid, but not proper for the given operation..";
                break;
            case APIERR_NO3D:
                error_type = "There is no 3D information assigned to the passed element.";
                break;
            case APIERR_NOMORE:
                error_type = "No more database items can be returned.";
                break;
            case APIERR_NOPLAN:
                error_type = "There is no open project.The operation cannot be executed without an open project.";
                break;
            case APIERR_NOLIB:
                error_type = "No library was loaded.The operation cannot be executed without a loaded library.Can be returned by ACAPI_LibPart_Create.";
                break;
            case APIERR_NOLIBSECT:
                error_type = "The requested LibPart section is not found.";
                break;
            case APIERR_NOSEL:
                error_type = "No selection.The operation cannot be executed without any element selected.";
                break;
            case APIERR_NOTEDITABLE:
                error_type = "The referenced element is not editable.";
                break;
            case APIERR_NOTSUBTYPEOF:
                error_type = "The specified first library part unique ID does not refer to a subtype of the second unique ID.See APIAny_CheckLibPartSubtypeOfID.";
                break;
            case APIERR_NOTEQUALMAIN:
                error_type = "The main GUID parts of the specified two library part unique IDs are not equal.See APIAny_CompareLibPartUnIdsID.";
                break;
            case APIERR_NOTEQUALREVISION:
                error_type = "The main GUID parts of the specified two library part unique IDs are equal but their revision IDs differ.See APIAny_CompareLibPartUnIdsID.";
                break;
            case APIERR_NOTEAMWORKPROJECT:
                error_type = "There is no open project, or not in Teamwork mode.";
                break;
            case APIERR_NOUSERDATA:
                error_type = "Attempt to get user data assigned to an element, but there isn’t any.";
                break;
            case APIERR_MOREUSER:
                error_type = "The user data cannot be assigned to the element, since there is no free storage block avaliable.";
                break;
            case APIERR_LINKEXIST:
                error_type = "The link already exists.";
                break;
            case APIERR_LINKNOTEXIST:
                error_type = "The link doesn’t exist.";
                break;
            case APIERR_WINDEXIST:
                error_type = "The window to be opened already exists.";
                break;
            case APIERR_WINDNOTEXIST:
                error_type = "The referenced window does not exist.";
                break;
            case APIERR_UNDOEMPTY:
                error_type = "No undoable entry has got into the opened undo operation.";
                break;
            case APIERR_REFERENCEEXIST:
                error_type = "The reference already exists.";
                break;
            case APIERR_NAMEALREADYUSED:
                error_type = "The resource must have a unique name but the specified one is already taken.";
                break;
            case APIERR_ATTREXIST:
                error_type = "The attribute already exists.";
                break;
            case APIERR_DELETED:
                error_type = "Reference to a deleted, purged or non - existent database item.";
                break;
            case APIERR_LOCKEDLAY:
                error_type = "The referenced layer is locked.";
                break;
            case APIERR_HIDDENLAY:
                error_type = "The referenced layer is hidden.";
                break;
            case APIERR_INVALFLOOR:
                error_type = "The passed floor index is out of range.";
                break;
            case APIERR_NOTMINE:
                error_type = "The database item is not in the user’s workspace.";
                break;
            case APIERR_NOACCESSRIGHT:
                error_type = "Can’t access / create / modify / delete an item in a teamwork server.";
                break;
#if defined(AC_22) || defined(AC_23)
            case APIERR_BADPROPERTYFORELEM:
                error_type = "The property for the passed element or attribute is not available.";
                break;
            case APIERR_BADCLASSIFICATIONFORELEM:
                error_type = "Can’t set the classification for the passed element or attribute.";
                break;
#else
            case APIERR_BADPROPERTY:
                error_type = "The property for the passed element or attribute is not available.";
                break;
            case APIERR_BADCLASSIFICATION:
                error_type = "Can’t set the classification for the passed element or attribute.";
                break;
#endif // AC_22 or AC_23
            case APIERR_MODULNOTINSTALLED:
                error_type = "The referenced add - on is not installed.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDMINE:
                error_type = "The target add - on is the caller add - on.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDNOTSUPPORTED:
                error_type = "The referenced command is not supported by the target add - on.For more details see the Communication Manager.";
                break;
            case APIERR_MODULCMDVERSNOTSUPPORTED:
                error_type = "The requested command version is newer than the version of the command that the target add - on can support.For more details see the Communication Manager.";
                break;
            case APIERR_NOMODULEDATA:
                error_type = "No custom data section is saved into the project file identified by the add - on’s unique ID.See ACAPI_ModulData_Get and ACAPI_ModulData_GetInfo.";
                break;
            case APIERR_PAROVERLAP:
                error_type = "Two or more paragraphs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
                break;
            case APIERR_PARMISSING:
                error_type = "Number of paragraphs – the size of paragraphs handle – is zero.";
                break;
            case APIERR_PAROVERFLOW:
                error_type = "Paragraph end offset is run over the content length.";
                break;
            case APIERR_PARIMPLICIT:
                error_type = "The content string contains line end character(CR) at invalid position(inside the paragraph range).";
                break;
            case APIERR_RUNOVERLAP:
                error_type = "Two or more runs are overlapped.The end offset of one is greater than the beginner offset of the next one.";
                break;
            case APIERR_RUNMISSING:
                error_type = "Number of runs – the size of run pointer – is zero.";
                break;
            case APIERR_RUNOVERFLOW:
                error_type = "Run end offset is run over the content length.";
                break;
            case APIERR_RUNIMPLICIT:
                error_type = "The beginner offset of one is greater than the end offset of the previous one.";
                break;
            case APIERR_RUNPROTECTED:
                error_type = "Attempted to overwrite a protected text run(not used yet).";
                break;
            case APIERR_EOLOVERLAP:
                error_type = "The EOL array is not a monotonous ascendant sequence.";
                break;
            case APIERR_TABOVERLAP:
                error_type = "The tabulator array is not a monotonous ascendant sequence.";
                break;
            case APIERR_NOTINIT:
                error_type = "The command needs initialization by an other API call.";
                break;
            case APIERR_NESTING:
                error_type = "The API function is not reentrant.Nesting occurred.";
                break;
            case APIERR_NOTSUPPORTED:
                error_type = "The command is not supported by the server application.It is not environment dependent.The server application cannot execute the command generally.";
                break;
            case APIERR_REFUSEDCMD:
                error_type = "The passed identifier is not subject to the operation.";
                break;
            case APIERR_REFUSEDPAR:
                error_type = "The command cannot be executed with the passed parameters.";
                break;
            case APIERR_READONLY:
                error_type = "The specified location is read - only.Can be returned by ACAPI_LibPart_Create.";
                break;
            case APIERR_SERVICEFAILED:
                error_type = "The invoked Teamwork service has failed.";
                break;
            case APIERR_COMMANDFAILED:
                error_type = "The invoked undoable command threw an exception.Can be returned by ACAPI_CallUndoableCommand.";
                break;
            case APIERR_NEEDSUNDOSCOPE:
                error_type = "The called command should be encapsulated in a ACAPI_CallUndoableCommand scope.";
                break;
            case APIERR_MISSINGCODE:
                error_type = "The function is not implemented yet.";
                break;
            case APIERR_MISSINGDEF:
                error_type = "The originating library part file is missing.The document name is still filled.";
                break;
            default:
                break;
        }
    }
    if (elemGuid != APINULLGuid) {
        error_type = "GUID: " + APIGuid2GSGuid (elemGuid).ToUniString () + " " + error_type;
        API_Elem_Head	elem_head = {};
        elem_head.guid = elemGuid;
        if (ACAPI_Element_GetHeader (&elem_head) == NoError) {
            GS::UniString elemName;

#if defined(AC_27) || defined(AC_28)
            if (ACAPI_Element_GetElemTypeName (elem_head.type, elemName) == NoError) {
#else
#ifdef AC_26
            if (ACAPI_Goodies_GetElemTypeName (elem_head.type, elemName) == NoError) {
#else
            if (ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) elem_head.typeID, &elemName) == NoError) {
#endif
#endif
                error_type = error_type + " type:" + elemName;
            }
            API_Attribute layer;
            BNZeroMemory (&layer, sizeof (API_Attribute));
            layer.header.typeID = API_LayerID;
            layer.header.index = elem_head.layer;
            if (ACAPI_Attribute_Get (&layer) == NoError) error_type = error_type + " layer:" + layer.header.name;
        }
    }
    GS::UniString msg = modulename + ": " + reportString + " " + error_type + "\n";
    ACAPI_WriteReport (msg, false);
    if (err != NoError) {
        msg = "== SMSTF ERR ==" + msg;
    }
    DBPrintf (msg.ToCStr ());
}


// --------------------------------------------------------------------
// Отмечает заданный пункт активным/неактивным
// --------------------------------------------------------------------
void	MenuItemCheckAC (short itemInd, bool checked)
{
    API_MenuItemRef itemRef;
    GSFlags         itemFlags;

    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    itemRef.menuResID = ID_ADDON_MENU + isEng ();
    itemRef.itemIndex = itemInd;

    itemFlags = 0;
#if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_GetMenuItemFlags (&itemRef, &itemFlags);
#else
    ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
    if (checked)
        itemFlags |= API_MenuItemChecked;
    else
        itemFlags &= ~API_MenuItemChecked;
#if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_SetMenuItemFlags (&itemRef, &itemFlags);
#else
    ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
    return;
}

// -----------------------------------------------------------------------------
// Получить массив Guid выбранных элементов
// Версия без чтения настроек
// -----------------------------------------------------------------------------
GS::Array<API_Guid>	GetSelectedElements2 (bool assertIfNoSel /* = true*/, bool onlyEditable /*= true*/)
{
    GSErrCode            err;
    API_SelectionInfo    selectionInfo;
    GS::UniString errorString = "Empty";
#ifdef AC_22
    API_Neig** selNeigs;
#else
    GS::Array<API_Neig>  selNeigs;
#endif
    err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, onlyEditable);
    BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
    if (err == APIERR_NOSEL || selectionInfo.typeID == API_SelEmpty) {
        if (assertIfNoSel) {
            DGAlert (DG_ERROR, "Error", errorString, "", "Ok");
        }
    }
    if (err != NoError) {
#ifdef AC_22
        BMKillHandle ((GSHandle*) &selNeigs);
#endif // AC_22
        return GS::Array<API_Guid> ();
    }
    GS::Array<API_Guid> guidArray;
#ifdef AC_22
    USize nSel = BMGetHandleSize ((GSHandle) selNeigs) / sizeof (API_Neig);
    for (USize i = 0; i < nSel; i++) {
        guidArray.Push ((*selNeigs)[i].guid);
    }
    BMKillHandle ((GSHandle*) &selNeigs);
#else
    for (const API_Neig& neig : selNeigs) {

        // Получаем список связанных элементов
        guidArray.Push (neig.guid);
    }
    return guidArray;
#endif // AC_22
}

// -----------------------------------------------------------------------------
// Вызов функции для выбранных элементов
//	(функция должна принимать в качетве аргумента API_Guid
// Версия без чтения настроек
// -----------------------------------------------------------------------------
void CallOnSelectedElem2 (void (*function)(const API_Guid&), bool assertIfNoSel /* = true*/, bool onlyEditable /* = true*/, GS::UniString & funcname)
{
    GS::Array<API_Guid> guidArray = GetSelectedElements2 (assertIfNoSel, onlyEditable);
    if (!guidArray.IsEmpty ()) {
        long time_start = clock ();
        GS::UniString subtitle ("working...");
        GS::Int32 nPhase = 1;
#if defined(AC_27) || defined(AC_28)
        bool showPercent = true;
        Int32 maxval = guidArray.GetSize ();
        ACAPI_ProcessWindow_InitProcessWindow (&funcname, &nPhase);
#else
        ACAPI_Interface (APIIo_InitProcessWindowID, &funcname, &nPhase);
#endif
        for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
#if defined(AC_27) || defined(AC_28)
            if (i % 10 == 0) ACAPI_ProcessWindow_SetNextProcessPhase (&subtitle, &maxval, &showPercent);
#else
            if (i % 10 == 0) ACAPI_Interface (APIIo_SetNextProcessPhaseID, &subtitle, &i);
#endif
            function (guidArray[i]);
#if defined(AC_27) || defined(AC_28)
            if (ACAPI_ProcessWindow_IsProcessCanceled ()) return;
#else
            if (ACAPI_Interface (APIIo_IsProcessCanceledID, nullptr, nullptr)) return;
#endif
        }
        long time_end = clock ();
        GS::UniString time = GS::UniString::Printf (" %d ms", (time_end - time_start) / 1000);
        GS::UniString intString = GS::UniString::Printf (" %d qty", guidArray.GetSize ());
        msg_rep (funcname + " Selected", intString + time, NoError, APINULLGuid);
#if defined(AC_27) || defined(AC_28)
        ACAPI_ProcessWindow_CloseProcessWindow ();
#else
        ACAPI_Interface (APIIo_CloseProcessWindowID, nullptr, nullptr);
#endif
    } else if (!assertIfNoSel) {
        function (APINULLGuid);
    }
}

// -----------------------------------------------------------------------------
// Получение типа объекта по его API_Guid
// -----------------------------------------------------------------------------
GSErrCode GetTypeByGUID (const API_Guid & elemGuid, API_ElemTypeID & elementType)
{
    GSErrCode		err = NoError;
    API_Elem_Head elem_head;
    BNZeroMemory (&elem_head, sizeof (API_Elem_Head));
    elem_head.guid = elemGuid;
    err = ACAPI_Element_GetHeader (&elem_head);
    if (err != NoError) {
        msg_rep ("GetTypeByGUID", "", err, elemGuid);
        return err;
    }
#if defined AC_26 || defined AC_27 || defined AC_28
    elementType = elem_head.type.typeID;
#else
    elementType = elem_head.typeID;
#endif
    return err;
}

#if defined AC_26 || defined AC_27 || defined AC_28
// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemType elemType, char* elemStr)
{
    GS::UniString	ustr;
    GSErrCode	err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_Element_GetElemTypeName (elemType, ustr);
#else
    err = ACAPI_Goodies_GetElemTypeName (elemType, ustr);
#endif
    if (err == NoError) {
        CHTruncate (ustr.ToCStr (), elemStr, ELEMSTR_LEN - 1);
        return true;
    }
    return false;
}
#else
// -----------------------------------------------------------------------------
// Получение названия типа элемента
// -----------------------------------------------------------------------------
bool GetElementTypeString (API_ElemTypeID typeID, char* elemStr)
{
    GS::UniString	ustr;
    GSErrCode	err = ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*) typeID, &ustr);
    if (err == NoError) {
        CHTruncate (ustr.ToCStr (), elemStr, ELEMSTR_LEN - 1);
        return true;
    }
    return false;
}
#endif // !AC_26

// -----------------------------------------------------------------------------
// Получить полное имя свойства (включая имя группы)
// -----------------------------------------------------------------------------
GSErrCode GetPropertyFullName (const API_PropertyDefinition & definision, GS::UniString & name)
{
    if (definision.groupGuid == APINULLGuid) return APIERR_BADID;
    GSErrCode error = NoError;
    if (definision.name.Contains ("ync_name")) {
        name = definision.name;
    } else {
        API_PropertyGroup group;
        group.guid = definision.groupGuid;
        error = ACAPI_Property_GetPropertyGroup (group);
        if (error == NoError) {
            name = group.name + "/" + definision.name;
        } else {
            msg_rep ("GetPropertyFullName", "ACAPI_Property_GetPropertyGroup " + definision.name, error, APINULLGuid);
        }
    }
    return error;
}


// -----------------------------------------------------------------------------
// Удаление данных аддона из элемента
// -----------------------------------------------------------------------------
void DeleteElementUserData (const API_Guid & elemguid)
{
    API_Elem_Head	tElemHead = {};
    tElemHead.guid = elemguid;
    API_ElementUserData userData = {};
    GSErrCode err = ACAPI_Element_GetUserData (&tElemHead, &userData);
    if (err == NoError && userData.dataHdl != nullptr) {
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_UserData_DeleteUserData (&tElemHead);
#else
        err = ACAPI_Element_DeleteUserData (&tElemHead);
#endif
        msg_rep ("Del user data", " ", NoError, APINULLGuid);
    }
    BMKillHandle (&userData.dataHdl);
    GS::Array<API_Guid> setGuids;
    err = ACAPI_ElementSet_Identify (elemguid, &setGuids);
    if (err == NoError) {
        USize nSet = setGuids.GetSize ();
        if (nSet > 0) {
            for (UIndex i = 0; i < nSet; i++) {
                err = ACAPI_ElementSet_Delete (setGuids[i]);
                if (err != NoError) {
                    DBPRINTF ("Delete Element Set error: %d\n", err);
                }
            }
            GS::UniString intString = GS::UniString::Printf (" %d", nSet);
            msg_rep ("Del set", intString, NoError, APINULLGuid);
        }
    }
}

// -----------------------------------------------------------------------------
// Удаление данных аддона из всех элементов
// -----------------------------------------------------------------------------
void DeleteElementsUserData ()
{
    GSErrCode err = NoError;
    GS::Array<API_Guid> addonelemList;
    err = ACAPI_AddOnObject_GetObjectList (&addonelemList);
    USize ngl = addonelemList.GetSize ();
    if (ngl > 0) {
        for (UIndex ii = 0; ii < ngl; ii++) {
            err = ACAPI_AddOnObject_DeleteObject (addonelemList[ii]);
        }
        GS::UniString intString = GS::UniString::Printf (" %d", ngl);
        msg_rep ("Del addon obj", intString, NoError, APINULLGuid);
    }
    GS::Array<API_Guid> elemList;
    ACAPI_Element_GetElemList (API_ZombieElemID, &elemList, APIFilt_IsEditable | APIFilt_HasAccessRight);
    USize ng = elemList.GetSize ();
    if (err == NoError) {
        ACAPI_CallUndoableCommand ("Delete Element Set",
                                   [&]() -> GSErrCode {
            for (UIndex ii = 0; ii < ng; ii++) {
                DeleteElementUserData (elemList[ii]);
            }
            return NoError;
        });
    }
}

// -----------------------------------------------------------------------------
// Включение и разблокирование всех слоёв
// -----------------------------------------------------------------------------
void UnhideUnlockAllLayer (void)
{
    API_Attribute		attrib;
    GSErrCode			err;
#if defined(AC_27) || defined(AC_28)
    UInt32 count, i;
    err = ACAPI_Attribute_GetNum (API_LayerID, count);
#else
    API_AttributeIndex count, i;
    err = ACAPI_Attribute_GetNum (API_LayerID, &count);
#endif
    if (err != NoError) msg_rep ("UnhideUnlockAllLayer", "ACAPI_Attribute_GetNum", err, APINULLGuid);
    if (err == NoError) {
        for (i = 2; i <= count; i++) {
            BNZeroMemory (&attrib, sizeof (API_Attribute));
            attrib.header.typeID = API_LayerID;
#if defined(AC_27) || defined(AC_28)
            attrib.header.index = ACAPI_CreateAttributeIndex (i);
#else
            attrib.header.index = i;
#endif
            err = ACAPI_Attribute_Get (&attrib);
            if (err != NoError) msg_rep ("UnhideUnlockAllLayer", "ACAPI_Attribute_Get", err, APINULLGuid);
            if (err == NoError) {
                bool flag_write = false;
                if (attrib.header.flags & APILay_Hidden) {
                    attrib.layer.head.flags |= !APILay_Hidden;
                    flag_write = true;
                }
                if (attrib.header.flags & APILay_Locked) {
                    attrib.layer.head.flags |= !APILay_Locked;
                    flag_write = true;
                }
                if (flag_write) {
                    err = ACAPI_Attribute_Modify (&attrib, NULL);
                    if (err != NoError) msg_rep ("UnhideUnlockAllLayer", attrib.header.name, err, APINULLGuid);
                }
            }
        }
    }
    return;
}

// -----------------------------------------------------------------------------
// Резервируем, разблокируем, вообщем - делаем элемент редактируемым
// Единственное, что может нас остановить - объект находится в модуле.
// -----------------------------------------------------------------------------
bool ReserveElement (const API_Guid & objectId, GSErrCode & err)
{
    (void) err;

    // Проверяем - на находится ли объект в модуле
    API_Elem_Head	tElemHead;
    BNZeroMemory (&tElemHead, sizeof (API_Elem_Head));
    tElemHead.guid = objectId;
    if (ACAPI_Element_GetHeader (&tElemHead) != NoError) return false;
    if (tElemHead.hotlinkGuid != APINULLGuid) return false; // С объектами в модуле сделать ничего не получится

    // Проверяем - зарезервирован ли объект и резервируем, если надо
#if defined(AC_27) || defined(AC_28)
    if (ACAPI_Teamwork_HasConnection () && !ACAPI_Element_Filter (objectId, APIFilt_InMyWorkspace)) {
#else
    if (ACAPI_TeamworkControl_HasConnection () && !ACAPI_Element_Filter (objectId, APIFilt_InMyWorkspace)) {
#endif
#if defined(AC_24) || defined(AC_23)
        GS::PagedArray<API_Guid>	elements;
#else
        GS::Array<API_Guid>	elements;
#endif // AC_24

        GS::HashTable<API_Guid, short>  conflicts;
        elements.Push (objectId);
#if defined(AC_27) || defined(AC_28)
        ACAPI_Teamwork_ReserveElements (elements, &conflicts, true);
#else
        ACAPI_TeamworkControl_ReserveElements (elements, &conflicts);
#endif
        if (!conflicts.IsEmpty ()) return false; // Не получилось зарезервировать
    }
    if (ACAPI_Element_Filter (objectId, APIFilt_HasAccessRight)) {
        if (ACAPI_Element_Filter (objectId, APIFilt_IsEditable)) {
            if (ACAPI_Element_Filter (objectId, APIFilt_InMyWorkspace)) {
                return true;; // Зарезервировали
            }
        }
    };
    return false; // Не получилось зарезервировать
}


// --------------------------------------------------------------------
// Проверка наличия дробной части, возвращает ЛОЖЬ если дробная часть есть
// --------------------------------------------------------------------
bool check_accuracy (double val, double tolerance)
{
    if (std::isinf (val) || std::isnan (val)) return true;
    val = std::fabs (val * 1000.0);
    if (val < std::numeric_limits<double>::epsilon ()) return true;
    double reciprocal = std::round ((1 / tolerance)); // Коэффицент домножения для заданной точности
    double val_round = std::round (val * reciprocal) / reciprocal; // Приведённое к заданной точности значение
    if (val_round < std::numeric_limits<double>::epsilon () && val>tolerance) {
        return false;
    }
    double val_correct1 = std::fabs (val_round - std::round (val_round));
    double val_correct2 = std::fabs (val_round - std::floor (val_round));
    bool bval1 = val_correct1 < tolerance;
    bool bval2 = val_correct2 < tolerance;
    return bval1 && bval2;
}

// --------------------------------------------------------------------
// Сравнение double c учётом точности
// --------------------------------------------------------------------
bool is_equal (double x, double y)
{
    return std::fabs (x - y) < std::numeric_limits<double>::epsilon ();
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const std::string & ignoreval, const GS::UniString & val)
{
    GS::UniString unignoreval = GS::UniString (ignoreval.c_str (), GChCode);
    return CheckIgnoreVal (unignoreval, val);
}

bool CheckIgnoreVal (const GS::UniString & ignoreval, const GS::UniString & val)
{
    if (ignoreval.IsEmpty ()) return false;
    if ((ignoreval.ToLowerCase () == "empty" || ignoreval.ToLowerCase () == u8"пусто") && val.GetLength () < 1) {
        return true;
    }
    if (val == ignoreval) {
        return true;
    }
    return false;
}

// --------------------------------------------------------------------
// Содержит ли значения элементиз списка игнорируемых
// --------------------------------------------------------------------
bool CheckIgnoreVal (const GS::Array<GS::UniString>&ignorevals, const GS::UniString & val)
{
    if (ignorevals.GetSize () > 0) {
        for (UInt32 i = 0; i < ignorevals.GetSize (); i++) {
            if (CheckIgnoreVal (ignorevals[i], val)) return true;
        }
    }
    return false;
}

// --------------------------------------------------------------------
// Перевод строки в число
// --------------------------------------------------------------------
bool UniStringToDouble (const GS::UniString & var, double& x)
{
    if (var.IsEmpty ()) return false;
    GS::UniString var_clear = var;
    var_clear.Trim ();
    var_clear.ReplaceAll (",", ".");
    std::string var_str = var_clear.ToCStr (0, MaxUSize, GChCode).Get ();
    int n = sscanf (var_str.c_str (), "%lf", &x);
    if (n <= 0) {
        var_clear.ReplaceAll (".", ",");
        var_str = var_clear.ToCStr (0, MaxUSize, GChCode).Get ();
        n = sscanf (var_str.c_str (), "%lf", &x);
    }
    return n > 0;
}

// --------------------------------------------------------------------
// Округлить целое n вверх до ближайшего целого числа, кратного k
// --------------------------------------------------------------------
Int32 ceil_mod (Int32 n, Int32 k)
{
    if (!k) return 0;
    Int32 tmp = abs (n % k);
    if (tmp) n += (n > -1 ? (abs (k) - tmp) : (tmp));
    return n;
}

// --------------------------------------------------------------------
// Перевод метров, заданных типом double в мм Int32
// --------------------------------------------------------------------
Int32 DoubleM2IntMM (const double& value)
{
    double param_real = round (value * 1000.0) / 1000.0;
    if (value - param_real > 0.001) param_real += 0.001;
    param_real = param_real * 1000.0;
    Int32 param_int = ceil_mod ((GS::Int32) param_real, 1);
    return param_int;
}

// -----------------------------------------------------------------------------
// Замена \n на перенос строки
// -----------------------------------------------------------------------------
void ReplaceCR (GS::UniString & val, bool clear)
{
    GS::UniString p = "\\n";
    if (val.Contains (p)) {
        if (!clear) {
            for (UInt32 i = 0; i < val.Count (p); i++) {
                UIndex inx = val.FindFirst (p);
                val.ReplaceFirst (p, "");
                val.SetChar (inx, CharCR);
            }
        } else {
            val.ReplaceAll (p, "");
        }
    }
}

// -----------------------------------------------------------------------------
// Дополнение строки заданным количеством пробелов или табуляций
// -----------------------------------------------------------------------------
void GetNumSymbSpase (GS::UniString & outstring, GS::UniChar symb, char charrepl)
{

    //Ищем указание длины строки
    Int32 stringlen = 0;
    GS::UniString part = "";
    if (outstring.Contains (symb)) {
        part = outstring.GetSubstring (symb, ' ', 0);
        if (!part.IsEmpty () && part.GetLength () < 4)
            stringlen = std::atoi (part.ToCStr ());
        if (stringlen > 0) part = symb + part;
    }
    if (stringlen > 0) {
        Int32 modlen = outstring.GetLength () - part.GetLength () - 1;
        Int32 addspace = stringlen - modlen;
        if (modlen > stringlen) {
            addspace = modlen % stringlen;
        }
        outstring.ReplaceAll (part + ' ', GS::UniString::Printf ("%s", std::string (addspace, charrepl).c_str ()));
    }
}

// -----------------------------------------------------------------------------
// Замена символов \\TAB и др. на юникод
// -----------------------------------------------------------------------------
void ReplaceSymbSpase (GS::UniString & outstring)
{
    GetNumSymbSpase (outstring, '~', ' ');
    GetNumSymbSpase (outstring, '@', CharTAB);
    outstring.ReplaceAll ("\\TAB", u8"\u0009");
    outstring.ReplaceAll ("\\CRLF", u8"\u000D\u000A");
    outstring.ReplaceAll ("\\CR", u8"\u000D");
    outstring.ReplaceAll ("\\LF", u8"\u000A");
    outstring.ReplaceAll ("\\PS", u8"\u2029");
    outstring.ReplaceAll ("\\LS", u8"\u2028");
    outstring.ReplaceAll ("\\NEL", u8"\u0085");
    outstring.ReplaceAll ("\\NL", u8"\u2424");
}


// -----------------------------------------------------------------------------
// Проверка статуса и получение ID пользователя Teamwork
// -----------------------------------------------------------------------------
GSErrCode IsTeamwork (bool& isteamwork, short& userid)
{
    isteamwork = false;
    API_ProjectInfo projectInfo = {};
    GSErrCode err = NoError;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_ProjectOperation_Project (&projectInfo);
#else
    err = ACAPI_Environment (APIEnv_ProjectID, &projectInfo);
#endif
    if (err == NoError) {
        isteamwork = projectInfo.teamwork;
        userid = projectInfo.userId;
    }
    return err;
}

// -----------------------------------------------------------------------------
// Вычисление выражений, заключённых в < >
// Что не может вычислить - заменит на пустоту
// -----------------------------------------------------------------------------
bool EvalExpression (GS::UniString & unistring_expression)
{
    if (unistring_expression.IsEmpty ()) return false;
    if (!unistring_expression.Contains ('<')) return false;
    GS::UniString texpression = unistring_expression;
    GS::UniString part = "";
    GS::UniString part_clean = "";
    GS::UniString stringformat = "";
    FormatString fstring;
    FormatString fstring_def = FormatStringFunc::ParseFormatString (".3m");
    bool flag_change = true;
    bool change_delim = true;
    while (unistring_expression.Contains ('<') && unistring_expression.Contains ('>') && flag_change) {
        GS::UniString expression_old = unistring_expression;
        part = unistring_expression.GetSubstring ('<', '>', 0);
        part_clean = part;
        // Ищем строку-формат
        stringformat = "";
        fstring = fstring_def;
        if (unistring_expression.Contains ('.')) {
            texpression = unistring_expression;
            FormatStringFunc::ReplaceMeters (texpression);
            if (texpression.Contains ('m')) {
                UInt32 n_start = texpression.FindFirst (part) + part.GetLength (); // Индекс начала поиска строки-формата
                GS::UniString stringformat_ = texpression.GetSubstring ('>', 'm', n_start) + 'm'; // Предположительно, строка-формат
                if (stringformat_.Contains ('.') && !stringformat_.Contains (' ')) {
                    // Проверим, не обрезали ли лишнюю m
                    UInt32 n_end = n_start + stringformat_.GetLength ();
                    if (n_end + 1 < texpression.GetLength ()) {
                        if (texpression.GetSubstring (n_end + 1, 1) == "m") {
                            n_end = n_end + 1;
                        }
                    }
                    stringformat = unistring_expression.GetSubstring (n_start + 1, n_end - n_start);
                    fstring = FormatStringFunc::ParseFormatString (stringformat);
                }
            }
        }
        typedef double T;
        typedef exprtk::expression<T>   expression_t;
        typedef exprtk::parser<T>       parser_t;
        std::string expression_string (part_clean.ToCStr (0, MaxUSize, GChCode).Get ());
        expression_t expression;
        parser_t parser;
        parser.compile (expression_string, expression);
        const T result = expression.value ();
        GS::UniString rezult_txt = FormatStringFunc::NumToString (result, fstring);
        unistring_expression.ReplaceAll ("<" + part + ">" + stringformat, rezult_txt);
        if (expression_old.IsEqual (unistring_expression)) flag_change = false;
    }
    return (!unistring_expression.IsEmpty ());
}

// -----------------------------------------------------------------------------
// Toggle a checked menu item
// -----------------------------------------------------------------------------
bool MenuInvertItemMark (short menuResID, short itemIndex)
{
    API_MenuItemRef		itemRef;
    GSFlags				itemFlags;
    BNZeroMemory (&itemRef, sizeof (API_MenuItemRef));
    itemRef.menuResID = menuResID;
    itemRef.itemIndex = itemIndex;
    itemFlags = 0;
#if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_GetMenuItemFlags (&itemRef, &itemFlags);
#else
    ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
    if ((itemFlags & API_MenuItemChecked) == 0)
        itemFlags |= API_MenuItemChecked;
    else
        itemFlags &= ~API_MenuItemChecked;
#if defined(AC_27) || defined(AC_28)
    ACAPI_MenuItem_SetMenuItemFlags (&itemRef, &itemFlags);
#else
    ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
#endif
    return (bool) ((itemFlags & API_MenuItemChecked) != 0);
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
GS::UniString StringUnic (const GS::UniString & instring, const GS::UniString & delim)
{
    if (!instring.Contains (delim)) return instring;
    GS::Array<GS::UniString> partstring;
    GS::UniString outsting = "";
    UInt32 n = StringSpltUnic (instring, delim, partstring);
    for (UInt32 i = 0; i < n; i++) {
        outsting = outsting + partstring[i];
        if (i < n - 1) outsting = outsting + delim;
    }
    return outsting;
}

// -----------------------------------------------------------------------------
// Возвращает уникальные вхождения текста
// -----------------------------------------------------------------------------
UInt32 StringSpltUnic (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring)
{
    if (!instring.Contains (delim)) {
        partstring.Push (instring);
        return 1;
    }
    GS::Array<GS::UniString> tpartstring;
    UInt32 n = StringSplt (instring, delim, tpartstring);
    std::map<std::string, int, doj::alphanum_less<std::string> > unic = {};
    for (UInt32 i = 0; i < n; i++) {
        std::string s = tpartstring[i].ToCStr (0, MaxUSize, GChCode).Get ();
        unic[s];
    }
    UInt32 nout = 0;
    for (std::map<std::string, int, doj::alphanum_less<std::string> >::iterator k = unic.begin (); k != unic.end (); ++k) {
        std::string s = k->first;
        GS::UniString unis = GS::UniString (s.c_str (), GChCode);
        partstring.Push (unis);
        nout = nout + 1;
    }
    return nout;
}


// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring)
{
    if (!instring.Contains (delim)) {
        partstring.Push (instring);
        return 1;
    }
    GS::Array<GS::UniString> parts;
    GS::UniString tinstring = instring;
    UInt32 npart = instring.Split (delim, &parts);
    UInt32 n = 0;
    for (UInt32 i = 0; i < npart; i++) {
        GS::UniString part = parts[i];
        if (!part.IsEmpty ()) {
            part.Trim ('\r');
            part.Trim ('\n');
            part.Trim ();
            if (!part.IsEmpty ()) {
                partstring.Push (part);
                n += 1;
            }
        }
    }
    return n;
}

// -----------------------------------------------------------------------------
// Делит строку по разделителю, возвращает кол-во частей
// Записывает в массив только части, содержащие строку filter
// -----------------------------------------------------------------------------
UInt32 StringSplt (const GS::UniString & instring, const GS::UniString & delim, GS::Array<GS::UniString>&partstring, const GS::UniString & filter)
{
    if (!instring.Contains (delim) || !instring.Contains (filter)) {
        partstring.Push (instring);
        return 1;
    }
    GS::Array<GS::UniString> parts;
    UInt32 n = 0;
    UInt32 npart = StringSplt (instring, delim, parts);
    for (UInt32 i = 0; i < npart; i++) {
        if (parts[i].Contains (filter)) {
            partstring.Push (parts.Get (i));
            n += 1;
        }
    }
    return n;
}


// -----------------------------------------------------------------------------
// Возвращает elemType и elemGuid для корректного чтение параметров элементов навесной стены
// -----------------------------------------------------------------------------
void GetGDLParametersHead (const API_Element & element, const API_Elem_Head & elem_head, API_ElemTypeID & elemType, API_Guid & elemGuid)
{
#if defined AC_26 || defined AC_27 || defined AC_28
    switch (elem_head.type.typeID) {
#else
    switch (elem_head.typeID) {
#endif // AC_26
        case API_CurtainWallPanelID:
            elemGuid = element.cwPanel.symbolID;
            elemType = API_ObjectID;
            break;
        case API_CurtainWallJunctionID:
            elemGuid = element.cwJunction.symbolID;
            elemType = API_ObjectID;
            break;
        case API_CurtainWallAccessoryID:
            elemGuid = element.cwAccessory.symbolID;
            elemType = API_ObjectID;
            break;
        default:
            UNUSED_VARIABLE (element);
            elemGuid = elem_head.guid;
#if defined AC_26 || defined AC_27 || defined AC_28
            elemType = elem_head.type.typeID;
#else
            elemType = elem_head.typeID;
#endif
            break;
    }
    return;
}

// -----------------------------------------------------------------------------
// Возвращает список параметров API_AddParType
// -----------------------------------------------------------------------------
GSErrCode GetGDLParameters (const API_ElemTypeID & elemType, const API_Guid & elemGuid, API_AddParType * *&params)
{
    GSErrCode	err = NoError;
    API_ParamOwnerType	apiOwner = {};
    API_GetParamsType	apiParams = {};
    BNZeroMemory (&apiOwner, sizeof (API_ParamOwnerType));
    BNZeroMemory (&apiParams, sizeof (API_GetParamsType));

    if (elemType == API_RailingToprailID
        || elemType == API_RailingHandrailID
        || elemType == API_RailingRailID
        || elemType == API_RailingPostID
        || elemType == API_RailingInnerPostID
        || elemType == API_RailingBalusterID
        || elemType == API_RailingPanelID
        || elemType == API_RailingNodeID
        || elemType == API_RailingToprailEndID
        || elemType == API_RailingHandrailEndID
        || elemType == API_RailingRailEndID
        || elemType == API_RailingToprailConnectionID
        || elemType == API_RailingHandrailConnectionID
        || elemType == API_RailingRailConnectionID
        || elemType == API_RailingEndFinishID) {
        API_ElementMemo	memo = {};
        err = ACAPI_Element_GetMemo (elemGuid, &memo, APIMemoMask_AddPars);
        params = memo.params;
        return err;
    }

#if defined(AC_27) || defined(AC_28)
    if (elemType == API_ExternalElemID) {
        API_ElementMemo	memo = {};
        err = ACAPI_Element_GetMemo (elemGuid, &memo, APIMemoMask_AddPars);
        params = memo.params;
        return err;
    }
#endif
    apiOwner.guid = elemGuid;
#if defined AC_26 || defined AC_27 || defined AC_28
    apiOwner.type.typeID = elemType;
#else
    apiOwner.typeID = elemType;
#endif
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_OpenParameters (&apiOwner);
#else
    err = ACAPI_Goodies (APIAny_OpenParametersID, &apiOwner, nullptr);
#endif
    if (err != NoError) {
        msg_rep ("GetGDLParameters", "APIAny_OpenParametersID", err, elemGuid);
        return err;
    }
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_GetActParameters (&apiParams);
#else
    err = ACAPI_Goodies (APIAny_GetActParametersID, &apiParams);
#endif
    if (err != NoError) {
        msg_rep ("GetGDLParameters", "APIAny_GetActParametersID", err, elemGuid);
#if defined(AC_27) || defined(AC_28)
        err = ACAPI_LibraryPart_CloseParameters ();
#else
        err = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
        if (err != NoError) msg_rep ("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
        return err;
    }
    params = apiParams.params;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_LibraryPart_CloseParameters ();
#else
    err = ACAPI_Goodies (APIAny_CloseParametersID);
#endif
    if (err != NoError) msg_rep ("GetGDLParameters", "APIAny_CloseParametersID", err, elemGuid);
    return err;
}


// --------------------------------------------------------------------
// Получение списка GUID панелей, рам и аксессуаров навесной стены
// --------------------------------------------------------------------
GSErrCode GetRElementsForCWall (const API_Guid & cwGuid, GS::Array<API_Guid>&elementsSymbolGuids)
{
    API_Element      element = {};
    element.header.guid = cwGuid;
    GSErrCode err = ACAPI_Element_Get (&element);
    if (err != NoError || !element.header.hasMemo) {
        return err;
    }
    API_ElementMemo	memo = {};
    UInt64 mask = APIMemoMask_CWallFrames | APIMemoMask_CWallPanels | APIMemoMask_CWallJunctions | APIMemoMask_CWallAccessories;
    err = ACAPI_Element_GetMemo (cwGuid, &memo, mask);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    bool isDegenerate = false;
    const GSSize nPanels = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallPanels)) / sizeof (API_CWPanelType);
    if (nPanels > 0) {
        for (Int32 idx = 0; idx < nPanels; ++idx) {
#if defined(AC_27) || defined(AC_28)
            err = ACAPI_CurtainWall_IsCWPanelDegenerate (&memo.cWallPanels[idx].head.guid, &isDegenerate);
#else
            err = ACAPI_Database (APIDb_IsCWPanelDegenerateID, (void*) (&memo.cWallPanels[idx].head.guid), &isDegenerate);
#endif
            if (err == NoError && !isDegenerate && memo.cWallPanels[idx].hasSymbol && !memo.cWallPanels[idx].hidden) {
                elementsSymbolGuids.Push (std::move (memo.cWallPanels[idx].head.guid));
            }
        }
    }
    const GSSize nWallFrames = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallFrames)) / sizeof (API_CWFrameType);
    if (nWallFrames > 0) {
        for (Int32 idx = 0; idx < nWallFrames; ++idx) {
            if (memo.cWallFrames[idx].hasSymbol && !memo.cWallFrames[idx].deleteFlag && memo.cWallFrames[idx].objectType != APICWFrObjectType_Invisible) {
                elementsSymbolGuids.Push (std::move (memo.cWallFrames[idx].head.guid));
            }
        }
    }
    const GSSize nWallJunctions = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallJunctions)) / sizeof (API_CWJunctionType);
    if (nWallJunctions > 0) {
        for (Int32 idx = 0; idx < nWallJunctions; ++idx) {
            if (memo.cWallJunctions[idx].hasSymbol) {
                elementsSymbolGuids.Push (std::move (memo.cWallJunctions[idx].head.guid));
            }
        }
    }
    const GSSize nWallAccessories = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.cWallAccessories)) / sizeof (API_CWAccessoryType);
    if (nWallAccessories > 0) {
        for (Int32 idx = 0; idx < nWallAccessories; ++idx) {
            if (memo.cWallAccessories[idx].hasSymbol) {
                elementsSymbolGuids.Push (std::move (memo.cWallAccessories[idx].head.guid));
            }
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return err;
}

// --------------------------------------------------------------------
// Получение списка GUID элементов ограждения
// --------------------------------------------------------------------
GSErrCode GetRElementsForRailing (const API_Guid & elemGuid, GS::Array<API_Guid>&elementsGuids)
{
    API_Element      element = {};
    element.header.guid = elemGuid;
    GSErrCode err = ACAPI_Element_Get (&element);
    if (err != NoError) {
        return err;
    }
    API_ElementMemo	memo = {};

    UInt64 mask = APIMemoMask_RailingNode | APIMemoMask_RailingSegment | APIMemoMask_RailingPost | APIMemoMask_RailingInnerPost | APIMemoMask_RailingRail | APIMemoMask_RailingHandrail | APIMemoMask_RailingToprail | APIMemoMask_RailingPanel | APIMemoMask_RailingBaluster | APIMemoMask_RailingPattern | APIMemoMask_RailingBalusterSet | APIMemoMask_RailingRailEnd | APIMemoMask_RailingHandrailEnd | APIMemoMask_RailingToprailEnd | APIMemoMask_RailingRailConnection | APIMemoMask_RailingHandrailConnection | APIMemoMask_RailingToprailConnection;
    err = ACAPI_Element_GetMemo (elemGuid, &memo, mask);
    if (err != NoError) {
        ACAPI_DisposeElemMemoHdls (&memo);
        return err;
    }
    GSSize n = 0;
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingRailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingRailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingHandrailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingHandrailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingToprailConnections)) / sizeof (API_RailingRailConnectionType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingToprailConnections[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingRailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingRailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingHandrailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingHandrailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingToprailEnds)) / sizeof (API_RailingRailEndType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingToprailEnds[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingPosts)) / sizeof (API_RailingPostType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingPosts[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingRails)) / sizeof (API_RailingRailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingRails[idx].visible) elementsGuids.Push (std::move (memo.railingRails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingToprails)) / sizeof (API_RailingToprailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingToprails[idx].visible) elementsGuids.Push (std::move (memo.railingToprails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingHandrails)) / sizeof (API_RailingHandrailType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingHandrails[idx].visible) elementsGuids.Push (std::move (memo.railingHandrails[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingInnerPosts)) / sizeof (API_RailingInnerPostType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingInnerPosts[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingBalusters)) / sizeof (API_RailingBalusterType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            elementsGuids.Push (std::move (memo.railingBalusters[idx].head.guid));
        }
    }
    n = BMGetPtrSize (reinterpret_cast<GSPtr>(memo.railingPanels)) / sizeof (API_RailingPanelType);
    if (n > 0) {
        for (Int32 idx = 0; idx < n; ++idx) {
            if (memo.railingPanels[idx].visible) elementsGuids.Push (std::move (memo.railingPanels[idx].head.guid));
        }
    }
    ACAPI_DisposeElemMemoHdls (&memo);
    return err;
}

// --------------------------------------------------------------------
// Возвращает координаты заданной точки после трансформации матрицей
// --------------------------------------------------------------------
API_Coord3D GetWordCoord3DTM (const API_Coord3D vtx, const API_Tranmat & tm)
{
    API_Coord3D	trCoord;	// world coordinates 
    trCoord.x = tm.tmx[0] * vtx.x + tm.tmx[1] * vtx.y + tm.tmx[2] * vtx.z + tm.tmx[3];
    trCoord.y = tm.tmx[4] * vtx.x + tm.tmx[5] * vtx.y + tm.tmx[6] * vtx.z + tm.tmx[7];
    trCoord.z = tm.tmx[8] * vtx.x + tm.tmx[9] * vtx.y + tm.tmx[10] * vtx.z + tm.tmx[11];
    return trCoord;
}

Point2D GetWordPoint2DTM (const Point2D vtx, const API_Tranmat & tm)
{
    API_Coord3D c = GetWordCoord3DTM ({ vtx.x, vtx.y, 0 }, tm);
    return { c.x, c.y };
}

// -----------------------------------------------------------------------------
// Ask the user to click a point
// -----------------------------------------------------------------------------

bool	ClickAPoint (const char* prompt, Point2D * c)
{
    API_GetPointType	pointInfo = {};
    GSErrCode			err;
    CHTruncate (prompt, pointInfo.prompt, sizeof (pointInfo.prompt));
    pointInfo.changeFilter = false;
    pointInfo.changePlane = false;
#if defined(AC_27) || defined(AC_28)
    err = ACAPI_UserInput_GetPoint (&pointInfo);
#else
    err = ACAPI_Interface (APIIo_GetPointID, &pointInfo, nullptr);
#endif
    if (err != NoError) {
        return false;
    }
    c->x = pointInfo.pos.x;
    c->y = pointInfo.pos.y;
    return true;
}		// ClickAPoint

namespace FormatStringFunc
{

// -----------------------------------------------------------------------------
// Обработка количества нулей и единиц измерения в имени свойства
// Удаляет из имени paramName найденные единицы измерения
// Возвращает строку для скармливания функции NumToStig
// -----------------------------------------------------------------------------
GS::UniString GetFormatString (GS::UniString& paramName)
{
    GS::UniString formatstring = "";
    Int32 iseng = isEng ();
    if (!paramName.Contains (".")) return formatstring;
    GS::UniString meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, MeterStringID, ACAPI_GetOwnResModule ());
    if (!paramName.Contains ('m') && !paramName.Contains (meterString)) return formatstring;
    GS::Array<GS::UniString> partstring;
    UInt32 n = StringSplt (paramName, ".", partstring);
    if (n > 1) {
        formatstring = partstring[n - 1];
        if (formatstring.Contains ('m') || formatstring.Contains (meterString)) {
            if (formatstring.Contains (CharENTER)) {
                UIndex attribinx = formatstring.FindLast (CharENTER);
                formatstring = formatstring.GetSubstring (0, attribinx);
            }
            paramName.ReplaceAll ('.' + formatstring, "");
            ReplaceMeters (formatstring, iseng);
        }
    }
    return formatstring;
}

void ReplaceMeters (GS::UniString& formatstring)
{
    Int32 iseng = isEng ();
    ReplaceMeters (formatstring, iseng);
}

void ReplaceMeters (GS::UniString& formatstring, Int32& iseng)
{
    GS::UniString meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, MeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "m");
    meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, DMeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "d");
    meterString = RSGetIndString (ID_ADDON_STRINGS + iseng, CMeterStringID, ACAPI_GetOwnResModule ());
    formatstring.ReplaceAll (meterString, "c");
}


// -----------------------------------------------------------------------------
// Возвращает словарь строк-форматов для типов данных согласно настройкам Рабочей среды проекта
// -----------------------------------------------------------------------------
FormatStringDict GetFotmatStringForMeasureType ()
{
    FormatStringDict fdict = {};
    // Получаем данные об округлении и типе расчёта
    API_CalcUnitPrefs unitPrefs1;
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProjectSetting_GetPreferences (&unitPrefs1, APIPrefs_CalcUnitsID);
#else
    ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs1, (void*) APIPrefs_CalcUnitsID);
#endif
    API_WorkingUnitPrefs unitPrefs;
#if defined(AC_27) || defined(AC_28)
    ACAPI_ProjectSetting_GetPreferences (&unitPrefs, APIPrefs_WorkingUnitsID);
#else
    ACAPI_Environment (APIEnv_GetPreferencesID, &unitPrefs, (void*) APIPrefs_WorkingUnitsID);
#endif
    FormatString fstring = {};
    fstring.needRound = unitPrefs1.useDisplayedValues;

    fstring.n_zero = 2; fstring.stringformat = "2";
    fdict.Add (API_PropertyUndefinedMeasureType, fstring);

    fstring.n_zero = 2; fstring.stringformat = "2";
    fdict.Add (API_PropertyDefaultMeasureType, fstring);

    fstring.n_zero = unitPrefs.areaDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.areaDecimals);
    fdict.Add (API_PropertyAreaMeasureType, fstring);

    fstring.n_zero = unitPrefs.lenDecimals; fstring.stringformat = GS::UniString::Printf ("0%dmm", unitPrefs.lenDecimals);
    fdict.Add (API_PropertyLengthMeasureType, fstring);

    fstring.n_zero = unitPrefs.volumeDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.volumeDecimals);
    fdict.Add (API_PropertyVolumeMeasureType, fstring);

    fstring.n_zero = unitPrefs.angleDecimals; fstring.stringformat = GS::UniString::Printf ("0%d", unitPrefs.angleDecimals);
    fdict.Add (API_PropertyAngleMeasureType, fstring);
    return fdict;
}

// -----------------------------------------------------------------------------
// Извлекает из строки информацио о единицах измерении и округлении
// -----------------------------------------------------------------------------
FormatString ParseFormatString (const GS::UniString& stringformat)
{
    int n_zero = 3;
    Int32 krat = 0; // Крутность округления
    double koeff = 1; //Коэфф. увеличения
    bool trim_zero = true; //Требуется образать нули после запятой
    bool needround = false; //Требуется округлить численное значение для вычислений
    GS::UniString delimetr = ","; // Разделитель дробной части
    FormatString format;
    format.stringformat = stringformat;
    format.isEmpty = true;
    if (!stringformat.IsEmpty ()) {
        GS::UniString outstringformat = stringformat;
        if (stringformat.Contains (".")) {
            outstringformat.ReplaceAll (".", "");
            format.stringformat.ReplaceAll (".", "");
        }
        ReplaceMeters (outstringformat);
        if (outstringformat.Contains ("mm")) {
            n_zero = 0;
            koeff = 1000;
            outstringformat.ReplaceAll ("mm", "");
        }
        if (outstringformat.Contains ("cm")) {
            n_zero = 1;
            koeff = 100;
            outstringformat.ReplaceAll ("cm", "");
        }
        if (outstringformat.Contains ("dm")) {
            n_zero = 2;
            koeff = 10;
            outstringformat.ReplaceAll ("dm", "");
        }
        if (outstringformat.Contains ("gm")) {
            koeff = 1 / 100;
            outstringformat.ReplaceAll ("gm", "");
        }
        if (outstringformat.Contains ("km")) {
            koeff = 1 / 1000;
            outstringformat.ReplaceAll ("km", "");
        }
        if (outstringformat.Contains ("m")) {
            n_zero = 3;
            outstringformat.ReplaceAll ("m", "");
        }
        if (outstringformat.Contains ("p")) {
            delimetr = ".";
            outstringformat.ReplaceAll ("p", "");
        }
        if (outstringformat.Contains ("r")) {
            needround = true;
            outstringformat.ReplaceAll ("r", "");
        }

        // Принудительный вывод заданного кол-ва нулей после запятой
        if (outstringformat.Contains ("0")) {
            outstringformat.ReplaceAll ("0", "");
            outstringformat.Trim ();
            if (!outstringformat.IsEmpty ()) trim_zero = false;
        }
        if (!outstringformat.IsEmpty ()) {
            // Кратность округления
            if (outstringformat.Contains ("/")) {
                GS::Array<GS::UniString> params;
                UInt32 nparam = StringSplt (outstringformat, "/", params);
                if (params.GetSize () > 0) n_zero = std::atoi (params[0].ToCStr ());
                if (params.GetSize () > 1) krat = std::atoi (params[0].ToCStr ());
            } else {
                n_zero = std::atoi (outstringformat.ToCStr ());
            }
        }
        format.isEmpty = false;
        format.isRead = true;
    }
    format.needRound = needround;
    format.delimetr = delimetr;
    format.n_zero = n_zero;
    format.krat = krat;
    format.koeff = koeff;
    format.trim_zero = trim_zero;
    return format;
}


// -----------------------------------------------------------------------------
// Переводит число в строку согласно настройкам строки-формата
// -----------------------------------------------------------------------------
// TODO Придумать более изящную обработку округления
GS::UniString NumToString (const double& var, const FormatString& stringformat)
{
    if (fabs (var) < 0.00000001) return "0";
    GS::UniString out = "";
    Int32 n_zero = stringformat.n_zero;
    Int32 krat = stringformat.krat;
    double koeff = stringformat.koeff;
    bool trim_zero = stringformat.trim_zero;
    GS::UniString delimetr = stringformat.delimetr;
    double outvar = var * koeff;
    outvar = round (outvar * pow (10, n_zero)) / pow (10, n_zero);
    if (krat > 0) outvar = ceil_mod ((GS::Int32) var, krat);
    out = GS::UniString::Printf ("%f", outvar);
    out.ReplaceAll (".", delimetr);
    out.ReplaceAll (",", delimetr);
    out.TrimRight ('0');
    if (trim_zero) {
        out.TrimRight (delimetr.GetChar (0));
    } else {
        Int32 addzero = n_zero - (out.GetLength () - out.FindFirst (delimetr.GetChar (0)) - 1);
        if (addzero > 0) {
            for (Int32 i = 0; i < addzero; i++) {
                out = out + "0";
            }
        }
    }
    return out;
}
}
