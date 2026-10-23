#pragma once

#include "roombook/Roombook.hpp"

#if defined(ServerMainVers_2400)
template <class Check> void CheckConnectedElementsCache (Check &&check) {
    using namespace Roombook;
    RoomSurfaceCache cache;
    const API_Guid owner = APIGuidFromString ("00000000-0000-0000-0000-000000000272");
    const API_Guid otherOwner = APIGuidFromString ("00000000-0000-0000-0000-000000000273");
    Int32 calls = 0;
    const auto success = [&] (GS::Array<API_Guid> &items) {
        ++calls;
        items.Push (otherOwner);
        return NoError;
    };
    const auto read = [&] (const API_Guid &guid,
                           API_ElemTypeID type,
                           API_ElemFilterFlags filter,
                           API_ElemVariationID variation,
                           const API_Guid &renovation,
                           RoomSurfaceCache *storage) {
        return ReadConnectedElementsCached (guid, type, filter, variation, renovation, storage, success);
    };
    const auto first = read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache);
    const auto repeated = read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache);
    check (calls == 1 && first.error == NoError && repeated.items.GetSize () == 1 && repeated.items[0] == otherOwner,
           "connected shared owner reads once");
    for (Int32 field = 0; field < 5; ++field) {
        const auto guid = field == 0 ? otherOwner : owner;
        const auto type = field == 1 ? API_DoorID : API_WindowID;
        const auto filter = field == 2 ? APIFilt_IsVisibleByRenovation : APIFilt_None;
        const auto variation = field == 3 ? static_cast<API_ElemVariationID> (static_cast<Int32> (APIVarId_Generic) + 1)
                                          : APIVarId_Generic;
        const auto renovation = field == 4 ? otherOwner : APINULLGuid;
        const Int32 before = calls;
        read (guid, type, filter, variation, renovation, &cache);
        read (guid, type, filter, variation, renovation, &cache);
        check (calls == before + 1, "connected each query field distinguishes key");
    }
    const auto errorRead = [&] (GS::Array<API_Guid> &items) {
        ++calls;
        items.Push (otherOwner);
        return APIERR_BADID;
    };
    const auto failed = ReadConnectedElementsCached (
        owner, API_OpeningID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache, errorRead);
    const Int32 afterError = calls;
    const auto failedAgain = read (owner, API_OpeningID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache);
    check (failed.error == APIERR_BADID && failedAgain.error == APIERR_BADID && calls == afterError &&
               failedAgain.items.GetSize () == 1 && failedAgain.items[0] == otherOwner,
           "connected partial error cached without becoming success");
    // Возвращённая копия не должна менять сохранённый результат.
    auto copy = failedAgain;
    copy.items.Clear ();
    check (read (owner, API_OpeningID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache).items.GetSize () == 1,
           "connected cached value isolated from consumer mutation");
    const auto empty = [&] (GS::Array<API_Guid> &) {
        ++calls;
        return NoError;
    };
    ReadConnectedElementsCached (otherOwner, API_OpeningID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache, empty);
    const Int32 afterEmpty = calls;
    const auto emptyAgain = read (otherOwner, API_OpeningID, APIFilt_None, APIVarId_Generic, APINULLGuid, &cache);
    check (emptyAgain.error == NoError && emptyAgain.items.IsEmpty () && calls == afterEmpty,
           "connected successful empty cached");
    const Int32 beforeDirect = calls;
    const auto direct = read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, nullptr);
    read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, nullptr);
    check (calls == beforeDirect + 2 && direct.error == first.error && direct.items == first.items,
           "connected nullptr reads every time and matches cached success");
    check (beforeDirect == 8 && calls == beforeDirect + 2,
           "connected exact callback reads for cached and direct queries");
    RoomSurfaceCache newRun;
    const Int32 beforeNewRun = calls;
    const auto newFirst = read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, &newRun);
    const auto newRepeated = read (owner, API_WindowID, APIFilt_None, APIVarId_Generic, APINULLGuid, &newRun);
    check (calls == beforeNewRun + 1 && newFirst.error == first.error && newRepeated.error == first.error &&
               newFirst.items == first.items && newRepeated.items == first.items,
           "connected new run reads once and caches an independent result");
}
#endif
