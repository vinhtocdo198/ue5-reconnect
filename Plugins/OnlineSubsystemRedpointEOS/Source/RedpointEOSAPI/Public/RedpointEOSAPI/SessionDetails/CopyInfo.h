// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionDetails
{

class REDPOINTEOSAPI_API FCopyInfo
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_BEGIN(
        SessionDetails,
        CopyInfo,
        EOS_SESSIONDETAILS_COPYINFO_API_LATEST,
        Info)

    class Options
    {
    };

    class Result
    {
    public:
        FString SessionId;
        FString HostAddress;
        uint32 NumOpenPublicConnections;
        EOS_ProductUserId OwnerUserId;
        FString OwnerServerClientId;

        FString BucketId;
        uint32 NumPublicConnections;
        bool bAllowJoinInProgress;
        EOS_EOnlineSessionPermissionLevel PermissionLevel;
        bool bInvitesAllowed;
        bool bSanctionsEnabled;
        TArray<Common::EOnlinePlatformType> AllowedPlatformIds;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()