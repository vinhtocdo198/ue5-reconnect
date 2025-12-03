// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

class REDPOINTEOSAPI_API FCopyMemberInfo
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_BEGIN(
        LobbyDetails,
        CopyMemberInfo,
        EOS_LOBBYDETAILS_COPYMEMBERINFO_API_LATEST,
        MemberInfo)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
    };

    class Result
    {
    public:
        EOS_ProductUserId UserId;
        Common::EOnlinePlatformType Platform;
        bool bAllowsCrossplay;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()