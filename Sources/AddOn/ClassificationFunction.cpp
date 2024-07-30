//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"ClassificationFunction.hpp"
#include	"Helpers.hpp"

namespace ClassificationFunc
{
GSErrCode GetAllClassification (SystemDict& systemdict)
{
    GSErrCode err = NoError;
    GS::Array<API_ClassificationSystem> systems;
    err = ACAPI_Classification_GetClassificationSystems (systems);
    if (err != NoError) {
        msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystems", err, APINULLGuid);
        return err;
    }
    for (UIndex i = 0; i < systems.GetSize (); ++i) {
        GS::UniString systemname = systems[i].name.ToLowerCase ();
        systemname.Trim ();
        if (!systemdict.ContainsKey (systemname)) {
            GS::Array<API_ClassificationItem> allItems;
            GS::Array<API_ClassificationItem> rootItems;
            ClassificationDict classifications;
            err = ACAPI_Classification_GetClassificationSystemRootItems (systems[i].guid, rootItems);
            if (err == NoError) {
                for (UIndex j = 0; j < rootItems.GetSize (); ++j) {
                    API_ClassificationItem parent;
                    AddClassificationItem (rootItems[j], parent, classifications, systems[i]);
                    GatherAllDescendantOfClassification (rootItems[j], classifications, systems[i]);
                }
            } else {
                msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystemRootItems", err, systems[i].guid);
            }
            if (!classifications.IsEmpty ()) {
                API_ClassificationItem parent, item;
                AddClassificationItem (item, parent, classifications, systems[i]);
                systemdict.Add (systemname, classifications);
                GS::UniString autoclassname = "@some_stuff_class@";
                if (classifications.ContainsKey (autoclassname)) {
                    ClassificationDict autoclassifications;
                    autoclassifications.Add (autoclassname, classifications.Get (autoclassname));
                    systemdict.Add (autoclassname, autoclassifications);
                }
            }
        }
    }
    return err;
}
void GatherAllDescendantOfClassification (const API_ClassificationItem& item, ClassificationDict& classifications, const  API_ClassificationSystem& system)
{
    GSErrCode err = NoError;
    GS::Array<API_ClassificationItem> directChildren;
    err = ACAPI_Classification_GetClassificationItemChildren (item.guid, directChildren);
    if (err == NoError) {
        for (UIndex i = 0; i < directChildren.GetSize (); ++i) {
            AddClassificationItem (directChildren[i], item, classifications, system);
            GatherAllDescendantOfClassification (directChildren[i], classifications, system);
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
        ClassificationValues classificationitem;
        classificationitem.item = item;
        classificationitem.system = system;
        classificationitem.itemname = itemname;
        classificationitem.parentname = parent.id.ToLowerCase ();
        classifications.Add (itemname, classificationitem);
    }
    if (desc.Contains ("some_stuff_class")) {
        ClassificationValues classificationitem;
        classificationitem.item = item;
        classificationitem.system = system;
        classificationitem.itemname = "@some_stuff_class@";
        classificationitem.parentname = "";
        classifications.Add ("@some_stuff_class@", classificationitem);
    }
}
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

API_Guid FindClass (const SystemDict& systemdict, GS::UniString& systemname, GS::UniString& classname)
{
    if (systemdict.IsEmpty ()) return APINULLGuid;
    if (!systemdict.ContainsKey (systemname)) return APINULLGuid;
    API_Guid classgiud = APINULLGuid;
    if (systemdict.Get (systemname).ContainsKey (classname)) {
        classgiud = systemdict.Get (systemname).Get (classname).item.guid;
        return classgiud;
    }
    return classgiud;
}

void SetAutoclass (SystemDict& systemdict, const API_Guid elemGuid)
{
    if (systemdict.IsEmpty ()) GetAllClassification (systemdict);
    GS::UniString autoclassname = "@some_stuff_class@";
    if (!systemdict.ContainsKey (autoclassname)) return;
    if (!systemdict.Get (autoclassname).ContainsKey (autoclassname)) return;
    API_ClassificationItem item = systemdict.Get (autoclassname).Get (autoclassname).item;
    API_ClassificationSystem system = systemdict.Get (autoclassname).Get (autoclassname).system;
    GSErrCode err = NoError;
    GS::Array<GS::Pair<API_Guid, API_Guid>> systemItemPairs;
    err = ACAPI_Element_GetClassificationItems (elemGuid, systemItemPairs);
    bool needChangeClass = false;
    if (systemItemPairs.IsEmpty ()) needChangeClass = true;
    if (needChangeClass) {
        err = ACAPI_Element_AddClassificationItem (elemGuid, item.guid);
    }
    if (err != NoError) msg_rep ("SetAutoclass", "ACAPI_Element_AddClassificationItem", err, elemGuid);
    return;
}
}