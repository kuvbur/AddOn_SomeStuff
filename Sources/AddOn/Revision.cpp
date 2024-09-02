//------------ kuvbur 2022 ------------
#include	"ACAPinc.h"
#include	"APIEnvir.h"
#include	"Revision.hpp"
#include	<stdlib.h> /* atoi */
#include	<time.h>

typedef struct
{
    GS::Array<API_Guid> guids;
} Revision;

typedef std::map<std::string, Revision, doj::alphanum_less<std::string>> RevisionDict;

void SetRevision (void)
{
    //GSErrCode err = NoError;
    //GS::Array<API_DatabaseUnId>		dbases;
    //err = ACAPI_Database (APIDb_GetLayoutDatabasesID, nullptr, &dbases);
    //if (err != NoError) return;


    //API_DatabaseInfo dbInfo;
    //BNZeroMemory (&dbInfo, sizeof (API_DatabaseInfo));
    //err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &dbInfo);
    //if (err != NoError) return;
    //if (dbInfo.typeID != APIWind_LayoutID) return;
    //API_ElemTypeID elementType = API_ChangeMarkerID;
    //GS::Array<API_Guid>	guidArray;
    //err = ACAPI_Element_GetElemList (elementType, &guidArray, APIFilt_IsEditable | APIFilt_HasAccessRight | APIFilt_InMyWorkspace | APIFilt_OnVisLayer);
    //if (guidArray.IsEmpty ()) return;
    //API_Element element = {};
    //for (UInt32 i = 0; i < guidArray.GetSize (); i++) {
    //    element.header.guid = guidArray[i];
    //    err = ACAPI_Element_Get (&element);

    //    if (err != NoError) {
    //    }
    //}

    //GS::Array<API_RVMChange> changes;
    //err = ACAPI_Database(APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &changes);
    //if (err != NoError) {
    //	ACAPI_WriteReport("Do_GetLayoutCurrentRevisionChanges: error occured!", false);
    //	return;
    //}

    //char buffer[256];

    //if (changes.IsEmpty()) {
    //	sprintf(buffer, "There are no changes in current revision of {%s} layout!", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
    //	ACAPI_WriteReport(buffer, false);
    //	return;
    //}

    //sprintf(buffer, "# Changes in current revision of {%s} layout:", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
    //ACAPI_WriteReport(buffer, false);

    //for (auto& change : changes.AsConst()) {
    //	sprintf(buffer, "ID: %s, Description: %s", change.id.ToCStr().Get(), change.description.ToCStr().Get());
    //	ACAPI_WriteReport(buffer, false);
    //}
}
