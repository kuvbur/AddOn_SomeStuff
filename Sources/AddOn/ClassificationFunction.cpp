//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"ClassificationFunction.hpp"
#include	"Helpers.hpp"
#include	"Propertycache.hpp"
namespace ClassificationFunc
{

const GS::UniString autoclassname = "@some_stuff_class@"; // Ключ автокласса

// -----------------------------------------------------------------------------
// Получение словаря со всеми классами во всех системах классифкации
// -----------------------------------------------------------------------------
GSErrCode GetAllClassification (SystemDict& systemdict)
{
    #if defined(TESTING)
    DBprnt ("GetAllClassification start");
    #endif
    GSErrCode err = NoError;
    GS::Array<API_ClassificationSystem> systems = {};
    err = ACAPI_Classification_GetClassificationSystems (systems);
    if (err != NoError) {
        msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystems", err, APINULLGuid);
        return err;
    }
    for (const auto& system : systems) {
        GS::UniString systemname = system.name.ToLowerCase ();
        systemname.Trim ();

        GS::UniString systemname_full = systemname + " v" + system.editionVersion.ToLowerCase ();
        systemname_full.Trim ();

        bool has_systemname = systemdict.ContainsKey (systemname);
        bool has_systemname_full = systemdict.ContainsKey (systemname_full);
        bool has_autoclassname = systemdict.ContainsKey (autoclassname);

        if (!has_systemname || !has_systemname_full) {
            GS::Array<API_ClassificationItem> allItems;
            GS::Array<API_ClassificationItem> rootItems;
            ClassificationDict classifications = {};
            err = ACAPI_Classification_GetClassificationSystemRootItems (system.guid, rootItems);
            if (err == NoError) {
                for (const auto& item : rootItems) {
                    API_ClassificationItem parent = {};
                    AddClassificationItem (item, parent, classifications, system);
                    GatherAllDescendantOfClassification (item, classifications, system);
                }
            } else {
                msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystemRootItems", err, system.guid);
                continue;
            }
            if (!classifications.IsEmpty ()) {
                API_ClassificationItem parent = {};
                API_ClassificationItem item = {};
                AddClassificationItem (item, parent, classifications, system);
                if (!has_systemname) systemdict.Put (systemname, classifications);
                if (!has_systemname_full) systemdict.Put (systemname_full, classifications);
                if (classifications.ContainsKey (autoclassname) && !has_autoclassname) {
                    ClassificationDict autoclassifications = {};
                    autoclassifications.Put (autoclassname, classifications.Get (autoclassname));
                    systemdict.Put (autoclassname, autoclassifications);
                }
            }
        }
    }
    #if defined(TESTING)
    DBprnt ("GetAllClassification end");
    #endif
    return err;
}
void GatherAllDescendantOfClassification (const API_ClassificationItem& item, ClassificationDict& classifications, const  API_ClassificationSystem& system)
{
    GSErrCode err = NoError;
    GS::Array<API_ClassificationItem> directChildren = {};
    err = ACAPI_Classification_GetClassificationItemChildren (item.guid, directChildren);
    if (err == NoError) {
        for (const auto& children : directChildren) {
            AddClassificationItem (children, item, classifications, system);
            GatherAllDescendantOfClassification (children, classifications, system);
        }
    } else {
        msg_rep ("ClassificationFunc::GatherAllDescendantOfClassification", "ACAPI_Classification_GetClassificationItemChildren", err, item.guid);
    }
}

void AddClassificationItem (const API_ClassificationItem& item, const  API_ClassificationItem& parent, ClassificationDict& classifications, const API_ClassificationSystem& system)
{
    GS::UniString itemname = item.id.ToLowerCase ();
    GS::UniString desc = item.description.ToLowerCase ();
    if (itemname.IsEmpty ()) itemname = "@system@";
    if (!classifications.ContainsKey (itemname)) {
        ClassificationValues classificationitem = {};
        classificationitem.item = item;
        classificationitem.system = system;
        classificationitem.itemname = itemname;
        classificationitem.parentname = parent.id.ToLowerCase ();
        classifications.Put (itemname, classificationitem);
    }

    if (desc.ToLowerCase ().Contains ("some_stuff_class") || desc.ToLowerCase ().Contains ("somestuff_class") || desc.ToLowerCase ().Contains ("somestuffclass")) {
        ClassificationValues classificationitem = {};
        classificationitem.item = item;
        classificationitem.system = system;
        classificationitem.itemname = autoclassname;
        classificationitem.parentname = "";
        classifications.Put (autoclassname, classificationitem);
    }
}

// -----------------------------------------------------------------------------
// Получение полного имени класса с чётом родительских классов
// -----------------------------------------------------------------------------
void GetFullName (const API_ClassificationItem& item, const ClassificationDict& classifications, GS::UniString& fullname)
{
    GS::UniString itemname = item.id.ToLowerCase ();
    if (classifications.ContainsKey (itemname)) {
        if (fullname.IsEmpty ()) {
            fullname = classifications.Get (itemname).item.id;
        } else {
            fullname = classifications.Get (itemname).item.id + "/" + fullname;
        }
        GS::UniString parentname = classifications.Get (itemname).parentname;
        if (!parentname.IsEmpty () && classifications.ContainsKey (parentname)) {
            GetFullName (classifications.Get (parentname).item, classifications, fullname);
        }
    }
}

bool ReadSystemDict ()
{
    auto& cache = PROPERTYCACHE ();
    if (cache.isClassification_OK) return true;
    if (!cache.isClassificationRead) cache.ReadClassification ();
    return cache.isClassification_OK;
}

// -----------------------------------------------------------------------------
// Поиск класса по ID в заданной классификации, возвращает Guid класса
// -----------------------------------------------------------------------------
API_Guid ClassificationFunc::FindClass (const GS::UniString& systemname, const GS::UniString& classname)
{
    if (!ReadSystemDict ()) return APINULLGuid;
    ClassificationFunc::SystemDict& systemdict = PROPERTYCACHE ().systemdict;

    auto* classDict = systemdict.GetPtr (systemname);
    if (classDict == nullptr) return APINULLGuid;

    auto* itemPtr = classDict->GetPtr (classname);
    if (itemPtr == nullptr) return APINULLGuid;

    return itemPtr->item.guid;
}

GS::UniString ClassificationFunc::GetSystemName (const API_Guid& systemguid)
{

    if (!ReadSystemDict ()) return GS::UniString ();

    auto& reverseDict = PROPERTYCACHE ().reversesystemdict;

    const auto* gPtr = reverseDict.GetPtr (systemguid);
    if (gPtr == nullptr) return GS::UniString ();

    const auto* namePtr = gPtr->GetPtr (APINULLGuid);
    if (namePtr == nullptr) return GS::UniString ();

    return *namePtr;
}


API_ClassificationItem FindClass (const GS::Pair<API_Guid, API_Guid>& classitem)
{

    auto& cache = PROPERTYCACHE ();

    const auto* systemDict = cache.reversesystemdict.GetPtr (classitem.first);
    if (systemDict == nullptr) return {};

    const auto* classNamePtr = systemDict->GetPtr (classitem.second);
    if (classNamePtr == nullptr) return {};

    const auto* systemNamePtr = systemDict->GetPtr (APINULLGuid);
    if (systemNamePtr == nullptr) return {};

    const auto* dictPtr = cache.systemdict.GetPtr (*systemNamePtr);
    if (dictPtr == nullptr) return {};

    const auto* itemPtr = dictPtr->GetPtr (*classNamePtr);
    if (itemPtr == nullptr) return {};

    return itemPtr->item;
}

// -----------------------------------------------------------------------------
// Назначение автокласса (класса с описанием some_stuff_class) элементу без классификации
// -----------------------------------------------------------------------------
void SetAutoclass (const API_Guid elemGuid)
{
    if (!ReadSystemDict ()) return;

    auto& systemdict = PROPERTYCACHE ().systemdict;

    const auto* classDict = systemdict.GetPtr (autoclassname);
    if (classDict == nullptr) return;

    const auto* entry = classDict->GetPtr (autoclassname);
    if (entry == nullptr) return;

    const API_Guid targetGuid = entry->item.guid;
    GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
    GSErrCode err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);

    if (err != NoError) {
        msg_rep ("SetAutoclass", "ACAPI_Element_GetClassificationItems", err, elemGuid);
        return;
    }

    if (systemItemPairs.IsEmpty ()) {
        err = ACAPI_Element_AddClassificationItem (elemGuid, targetGuid);
        if (err != NoError) {
            msg_rep ("SetAutoclass", "ACAPI_Element_AddClassificationItem", err, elemGuid);
        }
    }
    return;
}
}
