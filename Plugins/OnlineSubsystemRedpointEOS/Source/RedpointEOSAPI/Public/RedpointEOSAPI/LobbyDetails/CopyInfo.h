// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::LobbyDetails
{

class REDPOINTEOSAPI_API FCopyInfo
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_BEGIN(LobbyDetails, CopyInfo, EOS_LOBBYDETAILS_COPYINFO_API_LATEST, Info)

    class Options
    {
    };

    class Result
    {
    public:
        FString LobbyId;
        EOS_ProductUserId LobbyOwnerUserId;
        EOS_ELobbyPermissionLevel PermissionLevel;
        uint32 AvailableSlots;
        uint32 MaxMembers;
        bool bAllowInvites;
        FString BucketId;
        bool bAllowHostMigration;
        bool bRTCRoomEnabled;
        bool bAllowJoinById;
        bool bRejoinAfterKickRequiresInvite;
        bool bPresenceEnabled;
        TArray<Common::EOnlinePlatformType> AllowedPlatformIds;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()