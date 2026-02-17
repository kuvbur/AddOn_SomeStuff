//------------ kuvbur 2022 ------------
#pragma once
#if !defined (ADDON_HPP)
#define	ADDON_HPP
#include "Helpers.hpp"
#define	 Menu_MonAll		1
#define	 Menu_SyncAll		2
#define	 Menu_SyncSelect	3
#define	 Menu_wallS			4
#define	 Menu_widoS			5
#define	 Menu_objS			6
#define	 Menu_cwallS		7
#define	 Menu_ReNum			8
#define	 Menu_Sum			9
#define	 Menu_RunParam		10
#define	 Menu_Spec		11
#define	 Menu_ShowSub		12
#define	 Menu_SetRevision	13
#define	 Menu_SetSub	14
#define	 Menu_RoomBook	15
#define	 Menu_AutoProfile	16
#define	 Menu_AutoLay	17

static const Int32 MonAll_CommandID = 1;
static const Int32 SyncAll_CommandID = 2;
static const Int32 SyncSelect_CommandID = 3;
static const Int32 wallS_CommandID = 4;
static const Int32 widoS_CommandID = 5;
static const Int32 objS_CommandID = 6;
static const Int32 cwallS_CommandID = 7;
static const Int32 ReNum_CommandID = 8;
static const Int32 Sum_CommandID = 9;
static const Int32 RunParam_CommandID = 10;
static const Int32 Spec_CommandID = 11;
static const Int32 ShowSub_CommandID = 12;
static const Int32 SetRevision_CommandID = 13;
static const Int32 SetSub_CommandID = 14;
static const Int32 RoomBook_CommandID = 15;
static const Int32 Auto3D_CommandID = 16;
static const Int32 AutoLay_CommandID = 17;

void SelectElement (API_NotifyEventID notify);

struct SelectionSingleton
{
    GS::Array<API_Neig> elements;
    int size;
    API_DatabaseUnId db;
    SelectionSingleton ()
    {
        elements.Clear ();
        size = 0;
        #ifdef TESTING
        DBprnt (size, "Selection       === Init");
        #endif
    }

    void SetDB (API_DatabaseUnId& hbd)
    {
        #ifdef TESTING
        DBprnt (size, "Selection       === SetDB");
        #endif
        db = hbd;
    }

    API_DatabaseUnId GetDB ()
    {
        return db;
    }

    bool IsEmpty ()
    {
        return elements.IsEmpty ();
    }

    bool Update ()
    {
        #ifdef TESTING
        DBprnt (size, "Selection       === Update start");
        #endif
        API_SelectionInfo selectionInfo;
        GS::Array<API_Neig> selNeigs = {};
        GSErrCode err = NoError;
        err = ACAPI_Selection_Get (&selectionInfo, &selNeigs, false);
        BMKillHandle ((GSHandle*) &selectionInfo.marquee.coords);
        if (err != NoError) msg_rep ("Selection", "ACAPI_Selection_Get", err, APINULLGuid);
        if (err != NoError) return false;
        if (selectionInfo.typeID == API_SelEmpty) return false;
        for (const API_Neig& neig : selNeigs) {
            API_Neig n = neig;
            #if defined(AC_27) || defined(AC_28) || defined(AC_29) || defined(AC_26)
            API_ElemType type = Neig_To_ElemID (neig.neigID);
            API_ElemTypeID typeID = type.typeID;
            #else
            API_ElemTypeID typeID = Neig_To_ElemID (neig.neigID);
            #endif
            if (typeID == API_SectElemID) {
                API_Guid elemGuid_;
                GetParentGUIDSectElem (neig.guid, elemGuid_, typeID);
                n.guid = elemGuid_;
            }
            switch (typeID) {
                case API_WallID:                    n.neigID = APINeig_Wall;                n.inIndex = 1;    break;
                case API_ColumnID:                    n.neigID = APINeig_Colu;                n.inIndex = 0;    break;
                case API_BeamID:                    n.neigID = APINeig_Beam;                n.inIndex = 1;    break;
                case API_WindowID:                    n.neigID = APINeig_WindHole;            n.inIndex = 0;    break;
                case API_DoorID:                    n.neigID = APINeig_DoorHole;            n.inIndex = 0;    break;
                case API_ObjectID:                    n.neigID = APINeig_Symb;                n.inIndex = 1;    break;
                case API_LampID:                    n.neigID = APINeig_Light;                n.inIndex = 1;    break;
                case API_SlabID:                    n.neigID = APINeig_Ceil;                n.inIndex = 1;    break;
                case API_RoofID:                    n.neigID = APINeig_Roof;                n.inIndex = 1;    break;
                case API_MeshID:                    n.neigID = APINeig_Mesh;                n.inIndex = 1;    break;
                case API_DimensionID:                n.neigID = APINeig_DimOn;                n.inIndex = 1;    break;
                case API_RadialDimensionID:            n.neigID = APINeig_RadDim;            n.inIndex = 1;    break;
                case API_LevelDimensionID:            n.neigID = APINeig_LevDim;            n.inIndex = 1;    break;
                case API_AngleDimensionID:            n.neigID = APINeig_AngDimOn;            n.inIndex = 1;    break;
                case API_TextID:                    n.neigID = APINeig_Word;                n.inIndex = 1;    break;
                case API_LabelID:                    n.neigID = APINeig_Label;                n.inIndex = 1;    break;
                case API_ZoneID:                    n.neigID = APINeig_Room;                n.inIndex = 1;    break;
                case API_HatchID:                    n.neigID = APINeig_Hatch;                n.inIndex = 1;    break;
                case API_LineID:                    n.neigID = APINeig_Line;                n.inIndex = 1;    break;
                case API_PolyLineID:                n.neigID = APINeig_PolyLine;            n.inIndex = 1;    break;
                case API_ArcID:                        n.neigID = APINeig_Arc;                n.inIndex = 1;    break;
                case API_CircleID:                    n.neigID = APINeig_Circ;                n.inIndex = 1;    break;
                case API_SplineID:                    n.neigID = APINeig_Spline;            n.inIndex = 1;    break;
                case API_HotspotID:                    n.neigID = APINeig_Hot;                n.inIndex = 1;    break;
                case API_CutPlaneID:                n.neigID = APINeig_CutPlane;            n.inIndex = 1;    break;
                case API_ElevationID:                n.neigID = APINeig_Elevation;            n.inIndex = 1;    break;
                case API_InteriorElevationID:        n.neigID = APINeig_InteriorElevation;    n.inIndex = 1;    break;
                case API_CameraID:                    n.neigID = APINeig_Camera;            n.inIndex = 1;    break;
                case API_PictureID:                    n.neigID = APINeig_PictObj;            n.inIndex = 1;    break;
                case API_DetailID:                    n.neigID = APINeig_Detail;            n.inIndex = 1;    break;
                case API_WorksheetID:                n.neigID = APINeig_Worksheet;            n.inIndex = 1;    break;
                case API_SectElemID:                n.neigID = APINeig_VirtSy;            n.inIndex = 1;    break;
                case API_DrawingID:                    n.neigID = APINeig_DrawingCenter;        n.inIndex = 1;    break;
                case API_CurtainWallID:                n.neigID = APINeig_CurtainWall;        n.inIndex = 1;    break;
                case API_CurtainWallSegmentID:        n.neigID = APINeig_CWSegment;            n.inIndex = 1;    break;
                case API_CurtainWallFrameID:        n.neigID = APINeig_CWFrame;            n.inIndex = 1;    break;
                case API_CurtainWallPanelID:        n.neigID = APINeig_CWPanel;            n.inIndex = 1;    break;
                case API_CurtainWallJunctionID:        n.neigID = APINeig_CWJunction;        n.inIndex = 1;    break;
                case API_CurtainWallAccessoryID:    n.neigID = APINeig_CWAccessory;        n.inIndex = 1;    break;
                case API_ShellID:                    n.neigID = APINeig_Shell;                n.inIndex = 1;    break;
                case API_SkylightID:                n.neigID = APINeig_SkylightHole;        n.inIndex = 0;    break;
                case API_MorphID:                    n.neigID = APINeig_Morph;                n.inIndex = 1;    break;
                case API_ChangeMarkerID:            n.neigID = APINeig_ChangeMarker;        n.inIndex = 1;    break;
                case API_GroupID:
                case API_HotlinkID:
                case API_CamSetID:
                default:
                    typeID = API_ZombieElemID;
            }
            if (typeID != API_ZombieElemID) {
                elements.Push (n);
            }
        }
        size = elements.GetSize ();
        #ifdef TESTING
        DBprnt (size, "Selection       === Update end");
        #endif
        return !elements.IsEmpty ();
    }

    void Clear ()
    {
        elements.Clear ();
        size = 0;
        #ifdef TESTING
        DBprnt (size, "Selection       === Clear end");
        #endif
    }

    void Select ()
    {
        #ifdef TESTING
        DBprnt (size, "Selection       === Set start");
        #endif
        GSErrCode err = NoError;
        if (elements.IsEmpty ()) {
            #if defined(AC_27) || defined(AC_28) || defined(AC_29)
            err = ACAPI_Selection_DeselectAll ();
            #else
            err = ACAPI_Element_DeselectAll ();
            #endif
            if (err != NoError) msg_rep ("Selection", "ACAPI_Element_DeselectAll", err, APINULLGuid);
            #ifdef TESTING
            DBprnt (size, "Selection       === Deselect");
            #endif
            return;
        }
        #if defined(AC_27) || defined(AC_28) || defined(AC_29)
        err = ACAPI_Selection_Select (elements, true);
        #else
        err = ACAPI_Element_Select (elements, true);
        #endif
        if (err != NoError) msg_rep ("Selection", "ACAPI_Element_Select", err, APINULLGuid);
        #ifdef TESTING
        DBprnt (size, "Selection       === Set end");
        #endif
    }
};

SelectionSingleton& GetInstance ();

extern SelectionSingleton& (*SELECTION)();

#if defined(AC_28) || defined(AC_29)
GSErrCode ElementEventHandlerProc (const API_NotifyElementType* elemType);

static GSErrCode ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);

static GSErrCode SelectionChangeHandlerProc (const API_Neig* selElemNeig);
#else
GSErrCode __ACENV_CALL	ElementEventHandlerProc (const API_NotifyElementType* elemType);

static GSErrCode __ACENV_CALL ProjectEventHandlerProc (API_NotifyEventID notifID, Int32 param);

static GSErrCode __ACENV_CALL	SelectionChangeHandlerProc (const API_Neig* selElemNeig);
#endif

void Do_ElementMonitor (bool& syncMon);

void SetPaletteMenuText (short paletteItemInd, Int32& bisEng);

void MenuSetState (SyncSettings& syncSettings);

static GSErrCode MenuCommandHandler (const API_MenuParams* menuParams);
#endif
