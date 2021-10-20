// *****************************************************************************
// Source code for the BrowserPalette class
// *****************************************************************************

// ---------------------------------- Includes ---------------------------------

#include	"LogShowPalette.hpp"

static const char*		siteURL = "http://home.sch.bme.hu/~lorantfyt/Selection_Test.html";
static const GS::Guid	paletteGuid ("{FEE27B6B-3873-4834-98B5-F0081AA4CD45}");

GS::Ref<LogShowPalette>	LogShowPalette::instance;

static GS::UniString GetStringFromJavaScriptVariable (GS::Ref<DG::JSBase> jsVariable)
{
	GS::Ref<DG::JSValue> jsValue = GS::DynamicCast<DG::JSValue> (jsVariable);
	if (DBVERIFY (jsValue != nullptr && jsValue->GetType () == DG::JSValue::STRING))
		return jsValue->GetString ();

	return GS::EmptyUniString;
}

template<class Type>
static GS::Ref<DG::JSBase> ConvertToJavaScriptVariable (const Type& cppVariable)
{
	return new DG::JSValue (cppVariable);
}

template<>
GS::Ref<DG::JSBase> ConvertToJavaScriptVariable (const LogShowPalette::ElementInfo& elemInfo)
{
	GS::Ref<DG::JSArray> js = new DG::JSArray ();
	js->AddItem (ConvertToJavaScriptVariable (elemInfo.guidStr));
	js->AddItem (ConvertToJavaScriptVariable (elemInfo.typeName));
	js->AddItem (ConvertToJavaScriptVariable (elemInfo.elemID));
	return js;
}

template<class Type>
static GS::Ref<DG::JSBase> ConvertToJavaScriptVariable (const GS::Array<Type>& cppArray)
{
	GS::Ref<DG::JSArray> newArray = new DG::JSArray ();
	for (const Type& item : cppArray) {
		newArray->AddItem (ConvertToJavaScriptVariable (item));
	}
	return newArray;
}

// --- Class definition: BrowserPalette ----------------------------------------

LogShowPalette::LogShowPalette () :
	DG::Palette (ACAPI_GetOwnResModule (), BrowserPaletteResId, ACAPI_GetOwnResModule (), paletteGuid),
	browser (GetReference (), BrowserId)
{
	Attach (*this);
	BeginEventProcessing ();
	InitBrowserControl ();
}

LogShowPalette::~LogShowPalette ()
{
	EndEventProcessing ();
}

bool LogShowPalette::HasInstance ()
{
	return instance != nullptr;
}

void LogShowPalette::CreateInstance ()
{
	DBASSERT (!HasInstance ());
	instance = new LogShowPalette ();
	ACAPI_KeepInMemory (true);
}

LogShowPalette&	LogShowPalette::GetInstance ()
{
	DBASSERT (HasInstance ());
	return *instance;
}

void LogShowPalette::Show ()
{
	DG::Palette::Show ();
	SetMenuItemCheckedState (true);
}

void LogShowPalette::Hide ()
{
	DG::Palette::Hide ();
	SetMenuItemCheckedState (false);
}

void LogShowPalette::InitBrowserControl ()
{
	browser.LoadURL (siteURL);
	RegisterACAPIJavaScriptObject ();
	UpdateSelectedElementsOnHTML ();
}

void  LogShowPalette::RegisterACAPIJavaScriptObject ()
{
	DG::JSObject* jsACAPI = new DG::JSObject ("ACAPI");

	jsACAPI->AddItem (new DG::JSFunction ("GetSelectedElements", [] (GS::Ref<DG::JSBase>) {
		return ConvertToJavaScriptVariable (GetSelectedElements ());
	}));

	jsACAPI->AddItem (new DG::JSFunction ("AddElementToSelection", [] (GS::Ref<DG::JSBase> param) {
		ModifySelection (GetStringFromJavaScriptVariable (param), AddToSelection);
		return ConvertToJavaScriptVariable (true);
	}));

	jsACAPI->AddItem (new DG::JSFunction ("RemoveElementFromSelection", [] (GS::Ref<DG::JSBase> param) {
		ModifySelection (GetStringFromJavaScriptVariable (param), RemoveFromSelection);
		return ConvertToJavaScriptVariable (true);
	}));

	browser.RegisterAsynchJSObject (jsACAPI);
}

void LogShowPalette::UpdateSelectedElementsOnHTML ()
{
	browser.ExecuteJS ("UpdateSelectedElements ()");
}

void LogShowPalette::SetMenuItemCheckedState (bool isChecked)
{
	API_MenuItemRef	itemRef = {};
	GSFlags			itemFlags = {};

	itemRef.menuResID = BrowserPaletteMenuResId;
	itemRef.itemIndex = BrowserPaletteMenuItemIndex;

	ACAPI_Interface (APIIo_GetMenuItemFlagsID, &itemRef, &itemFlags);
	if (isChecked)
		itemFlags |= API_MenuItemChecked;
	else
		itemFlags &= ~API_MenuItemChecked;
	ACAPI_Interface (APIIo_SetMenuItemFlagsID, &itemRef, &itemFlags);
}

void LogShowPalette::PanelResized (const DG::PanelResizeEvent& ev)
{
	BeginMoveResizeItems ();
	browser.Resize (ev.GetHorizontalChange (), ev.GetVerticalChange ());
	EndMoveResizeItems ();
}

void LogShowPalette::PanelCloseRequested (const DG::PanelCloseRequestEvent&, bool* accepted)
{
	Hide ();
	*accepted = true;
}

GS::Array<LogShowPalette::ElementInfo> LogShowPalette::GetSelectedElements ()
{
	API_SelectionInfo	selectionInfo;
	GS::Array<API_Neig>	selNeigs;
	ACAPI_Selection_Get (&selectionInfo, &selNeigs, false, false);
	BMKillHandle ((GSHandle*)&selectionInfo.marquee.coords);

	GS::Array<LogShowPalette::ElementInfo> selectedElements;
	for (const API_Neig& neig : selNeigs) {
		API_Elem_Head elemHead = {};
		elemHead.guid = neig.guid;
		ACAPI_Element_GetHeader (&elemHead);

		ElementInfo elemInfo;
		elemInfo.guidStr = APIGuidToString (elemHead.guid);
		ACAPI_Goodies (APIAny_GetElemTypeNameID, (void*)elemHead.typeID, &elemInfo.typeName);
		ACAPI_Database (APIDb_GetElementInfoStringID, &elemHead.guid, &elemInfo.elemID);
		selectedElements.Push (elemInfo);
	}
	return selectedElements;
}

void LogShowPalette::ModifySelection (const GS::UniString& elemGuidStr, LogShowPalette::SelectionModification modification)
{
	ACAPI_Element_Select ({ API_Neig (APIGuidFromString (elemGuidStr.ToCStr ().Get ())) }, modification == AddToSelection);
}

GSErrCode __ACENV_CALL	LogShowPalette::SelectionChangeHandler (const API_Neig*)
{
	if (LogShowPalette::HasInstance ())
		LogShowPalette::GetInstance ().UpdateSelectedElementsOnHTML ();
	return NoError;
}

GSErrCode __ACENV_CALL	LogShowPalette::PaletteControlCallBack (Int32, API_PaletteMessageID messageID, GS::IntPtr param)
{
	switch (messageID) {
		case APIPalMsg_OpenPalette:
			if (!HasInstance ())
				CreateInstance ();
			GetInstance ().Show ();
			break;

		case APIPalMsg_ClosePalette:
			if (!HasInstance ())
				break;
			GetInstance ().Hide ();
			break;

		case APIPalMsg_HidePalette_Begin:
			if (HasInstance () && GetInstance ().IsVisible ())
				GetInstance ().Hide ();
			break;

		case APIPalMsg_HidePalette_End:
			if (HasInstance () && !GetInstance ().IsVisible ())
				GetInstance ().Show ();
			break;

		case APIPalMsg_DisableItems_Begin:
			if (HasInstance () && GetInstance ().IsVisible ())
				GetInstance ().DisableItems ();
			break;

		case APIPalMsg_DisableItems_End:
			if (HasInstance () && GetInstance ().IsVisible ())
				GetInstance ().EnableItems ();
			break;

		case APIPalMsg_IsPaletteVisible:
			*(reinterpret_cast<bool*> (param)) = HasInstance () && GetInstance ().IsVisible ();
			break;

		default:
			break;
	}

	return NoError;
}

GSErrCode LogShowPalette::RegisterPaletteControlCallBack ()
{
	return ACAPI_RegisterModelessWindow (
					GS::CalculateHashValue (paletteGuid),
					PaletteControlCallBack,
					API_PalEnabled_FloorPlan + API_PalEnabled_Section + API_PalEnabled_Elevation +
					API_PalEnabled_InteriorElevation + API_PalEnabled_3D + API_PalEnabled_Detail +
					API_PalEnabled_Worksheet + API_PalEnabled_Layout + API_PalEnabled_DocumentFrom3D,
					GSGuid2APIGuid (paletteGuid));
}
