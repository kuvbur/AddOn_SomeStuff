//------------ kuvbur 2022 ------------
#include	<stdlib.h> /* atoi */
#include	<time.h>
#include	"APIEnvir.h"
#include	"ACAPinc.h"
#include	"Revision.hpp"

// -----------------------------------------------------------------------------
// Get the Changes from the current Document Revision of the given Layout
// -----------------------------------------------------------------------------

void		Do_GetLayoutCurrentRevisionChanges(void)
{
	API_DatabaseInfo dbInfo;
	BNZeroMemory(&dbInfo, sizeof(API_DatabaseInfo));
	GSErrCode err = ACAPI_Database(APIDb_GetCurrentDatabaseID, &dbInfo);
	if (err != NoError) {
		ACAPI_WriteReport("Do_GetLayoutCurrentRevisionChanges: error occured!", false);
		return;
	}

	if (dbInfo.typeID != APIWind_LayoutID) {
		ACAPI_WriteReport("Current database not a layout database!", false);
		return;
	}

	GS::Array<API_RVMChange> changes;
	err = ACAPI_Database(APIDb_GetRVMLayoutCurrentRevisionChangesID, &(dbInfo.databaseUnId), &changes);
	if (err != NoError) {
		ACAPI_WriteReport("Do_GetLayoutCurrentRevisionChanges: error occured!", false);
		return;
	}

	char buffer[256];

	if (changes.IsEmpty()) {
		sprintf(buffer, "There are no changes in current revision of {%s} layout!", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
		ACAPI_WriteReport(buffer, false);
		return;
	}

	sprintf(buffer, "# Changes in current revision of {%s} layout:", APIGuid2GSGuid(dbInfo.databaseUnId.elemSetId).ToUniString().ToCStr().Get());
	ACAPI_WriteReport(buffer, false);

	for (auto& change : changes.AsConst()) {
		sprintf(buffer, "ID: %s, Description: %s", change.id.ToCStr().Get(), change.description.ToCStr().Get());
		ACAPI_WriteReport(buffer, false);
	}
}