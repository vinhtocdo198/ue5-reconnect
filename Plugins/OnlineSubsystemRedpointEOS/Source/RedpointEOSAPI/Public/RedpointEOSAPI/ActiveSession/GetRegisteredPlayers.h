// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/ObjectSyncList.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::ActiveSession
{

class REDPOINTEOSAPI_API FGetRegisteredPlayers
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_BEGIN(
        ActiveSession,
        GetRegisteredPlayerCount,
        EOS_ACTIVESESSION_GETREGISTEREDPLAYERCOUNT_API_LATEST,
        GetRegisteredPlayerByIndex,
        EOS_ACTIVESESSION_GETREGISTEREDPLAYERBYINDEX_API_LATEST)

    class Options
    {
    };

    typedef EOS_ProductUserId ResultEntry;

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()