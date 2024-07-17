//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"ClassificationFunction.hpp"
#include	"Helpers.hpp"

namespace ClassificationFunc
{
GSErrCode ClassificationFunc::GetAllClassification ()
{
    GSErrCode err = NoError;
    GS::Array<API_ClassificationSystem> systems;
    err = ACAPI_Classification_GetClassificationSystems (systems);
    if (err != NoError) {
        msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystems", err, APINULLGuid);
        return err;
    }
    for (UIndex i = 0; i < systems.GetSize (); ++i) {
        GS::Array<API_ClassificationItem> allItems;
        GS::Array<API_ClassificationItem> rootItems;
        err = ACAPI_Classification_GetClassificationSystemRootItems (systems[i].guid, rootItems);
        if (err == NoError) {
            for (UIndex j = 0; j < rootItems.GetSize (); ++j) {
                allItems.Push (rootItems[j]);
                GatherAllDescendantOfClassification (rootItems[j], allItems);
            }
        } else {
            msg_rep ("ClassificationFunc::GetAllClassification", "ACAPI_Classification_GetClassificationSystemRootItems", err, systems[i].guid);
        }
    }
    return err;
}
void GatherAllDescendantOfClassification (const API_ClassificationItem& item, GS::Array<API_ClassificationItem>& allDescendant)
{
    GSErrCode err = NoError;
    GS::Array<API_ClassificationItem> directChildren;
    err = ACAPI_Classification_GetClassificationItemChildren (item.guid, directChildren);
    if (err == NoError) {
        for (UIndex i = 0; i < directChildren.GetSize (); ++i) {
            allDescendant.Push (directChildren[i]);
            GatherAllDescendantOfClassification (directChildren[i], allDescendant);
        }
    } else {
        msg_rep ("ClassificationFunc::GatherAllDescendantOfClassification", "ACAPI_Classification_GetClassificationItemChildren", err, item.guid);
    }
}
}
