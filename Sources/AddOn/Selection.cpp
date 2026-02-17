//------------ kuvbur 2026 ------------
#include	"ACAPinc.h"
#include    "Helpers.hpp"
#include	"Selection.hpp"

SelectionSingleton& GetInstance ()
{
    static SelectionSingleton instance;
    return instance;
}

SelectionSingleton& (*SELECTION)() = GetInstance;

void SelectElement (API_NotifyEventID notify)
{
    API_DatabaseInfo homedatabaseInfo;
    BNZeroMemory (&homedatabaseInfo, sizeof (API_DatabaseInfo));
    GSErrCode err = NoError;
    #if defined(AC_27) || defined(AC_28) || defined(AC_29)
    err = ACAPI_Database_GetCurrentDatabase (&homedatabaseInfo);
    #else
    err = ACAPI_Database (APIDb_GetCurrentDatabaseID, &homedatabaseInfo, nullptr);
    #endif
    API_DatabaseUnId selDB = SELECTION ().GetDB ();
    API_DatabaseUnId currDB = homedatabaseInfo.databaseUnId;
    bool sameDB = (selDB == currDB);
    #ifdef TESTING
    if (sameDB) {
        DBprnt ("Selection start === DB is same");
    } else {
        DBprnt ("Selection start === Different DB");
    }
    #endif
    switch (notify) {
        // Добавление выделения
        case APINotify_New:
            #ifdef TESTING
            DBprnt ("Selection       === APINotify_New");
            #endif
            // Очищаем выборку, если БД не менялась
            if (sameDB) {
                SELECTION ().Clear ();
                SELECTION ().Update ();
            }
            break;
            // Очистка выделения при закрытии
        case APINotify_Close:
            #ifdef TESTING
            DBprnt ("Selection       === APINotify_Close");
            #endif
            SELECTION ().Clear ();
            break;
            // Очистка выделения
        case APINotify_NewAndReset:
            #ifdef TESTING
            DBprnt ("Selection       === APINotify_NewAndReset");
            #endif
            if (sameDB) SELECTION ().Clear ();
            break;
            // Смена окна
        case APINotify_ChangeWindow:
            #ifdef TESTING
            DBprnt ("Selection       === APINotify_ChangeWindow");
            #endif
            if (!sameDB) {
                SELECTION ().Select ();
                SELECTION ().SetDB (currDB);
            }
            break;
        default:
            return;
            break;
    }
    #ifdef TESTING
    selDB = SELECTION ().GetDB ();
    sameDB = (selDB == currDB);
    if (sameDB) {
        DBprnt ("Selection   end === DB is same");
    } else {
        DBprnt ("Selection   end === Different DB");
    }
    #endif
}
