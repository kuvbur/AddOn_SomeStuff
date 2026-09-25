//------------ kuvbur 2022 ------------
#pragma once

#include "ACAPinc.h"

namespace SyncDialogs {

    struct OtherDbTarget {
        API_DatabaseInfo dbInfo = {};
        bool hasStory = false;
        short storyIndex = 0;
        GS::UniString name;
        GS::Array<API_Guid> guids;
    };

    void AddOtherDbTarget (GS::Array<OtherDbTarget> &targets,
                           const API_DatabaseInfo &dbInfo,
                           bool hasStory,
                           short storyIndex,
                           const GS::UniString &name,
                           const API_Guid &guid);

    GS::UniString GetOtherStoryName (short storyIndex);

    void ShowOtherDbDialog (const GS::Array<OtherDbTarget> &targets);

} // namespace SyncDialogs
