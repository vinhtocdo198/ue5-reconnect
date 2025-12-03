// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/ObjectSyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

class REDPOINTEOSAPI_API FGetMembers
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_BEGIN(
        LobbyDetails,
        GetMemberCount,
        EOS_LOBBYDETAILS_GETMEMBERCOUNT_API_LATEST,
        GetMemberByIndex,
        EOS_LOBBYDETAILS_GETMEMBERBYINDEX_API_LATEST)

    class Options
    {
    };

    typedef EOS_ProductUserId ResultEntry;

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()