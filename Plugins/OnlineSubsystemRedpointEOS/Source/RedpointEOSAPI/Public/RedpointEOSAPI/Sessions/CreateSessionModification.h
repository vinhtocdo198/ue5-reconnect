// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sessions
{

class REDPOINTEOSAPI_API FCreateSessionModification
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(
        Sessions,
        CreateSessionModification,
        EOS_SESSIONS_CREATESESSIONMODIFICATION_API_LATEST,
        SessionModification)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SessionName;
        ParamHelpers::TRequired<FString> BucketId;
        ParamHelpers::TRequired<uint32> MaxPlayers;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<bool> bPresenceEnabled;
        ParamHelpers::TRequired<FString> SessionId;
        ParamHelpers::TRequired<bool> bSanctionsEnabled;
        ParamHelpers::TRequired<TArray<Common::EOnlinePlatformType>> AllowedPlatformIds;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()