// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Friends
{

class REDPOINTEOSAPI_API FGetFriends
{
    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_BEGIN(
        Friends,
        GetFriendsCount,
        EOS_FRIENDS_GETFRIENDSCOUNT_API_LATEST,
        GetFriendAtIndex,
        EOS_FRIENDS_GETFRIENDATINDEX_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
    };

    typedef EOS_EpicAccountId ResultEntry;

    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()