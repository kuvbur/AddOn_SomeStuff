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
    GS::UniString itemname = item.name.ToLowerCase ();
    if (!classifications.ContainsKey (itemname)) {
        ClassificationValues classificationitem;
        classificationitem.item = item;
        classificationitem.parent = parent;
        classificationitem.system = system;
        classificationitem.system = system;
        classificationitem.itemname = itemname;
        classificationitem.parentname = parent.name.ToLowerCase ();
        classifications.Add (itemname, classificationitem);
    }
}
void GetFullName (const API_ClassificationItem& item, const ClassificationDict& classifications, GS::UniString& fullname)
{
    GS::UniString itemname = item.name.ToLowerCase ();
    if (classifications.ContainsKey (itemname)) {
        if (fullname.IsEmpty ()) {
            fullname = classifications.Get (itemname).item.name;
        } else {
            fullname = classifications.Get (itemname).item.name + "/" + fullname;
        }
        GS::UniString itemname = classifications.Get (itemname).parentname;
        //if (classifications.Get (item.guid).parent.guid != APINULLGuid) GetFullName (classifications.Get (item.guid).parent, classifications, fullname);
    }
}
}
