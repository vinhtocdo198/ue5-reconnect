// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/RTC/JoinRoomOptions.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FCreateLobby
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, CreateLobby, EOS_LOBBY_CREATELOBBY_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<uint32> MaxLobbyMembers;
        ParamHelpers::TRequired<EOS_ELobbyPermissionLevel> PermissionLevel;
        ParamHelpers::TRequired<bool> bPresenceEnabled;
        ParamHelpers::TRequired<bool> bAllowInvites;
        ParamHelpers::TRequired<FString> BucketId;
        ParamHelpers::TRequired<bool> bDisableHostMigration;
        ParamHelpers::TRequired<bool> bEnableRTCRoom;
        ParamHelpers::TRequired<RTC::EJoinRoomOptions::Type> RTCRoomJoinFlags;
        ParamHelpers::TRequired<bool> bRTCRoomUseManualAudioInput;
        ParamHelpers::TRequired<bool> bRTCRoomUseManualAudioOutput;
        ParamHelpers::TRequired<bool> bRTCRoomLocalAudioDeviceInputStartsMuted;
        ParamHelpers::TRequired<FString> LobbyId;
        ParamHelpers::TRequired<bool> bEnableJoinById;
        ParamHelpers::TRequired<bool> bRejoinAfterKickRequiresInvite;
        ParamHelpers::TRequired<TArray<Common::EOnlinePlatformType>> AllowedPlatformIds;
        ParamHelpers::TRequired<bool> bCrossplayOptOut;
#if EOS_VERSION_AT_LEAST(1, 17, 0)
        ParamHelpers::TRequired<EOS_ELobbyRTCRoomJoinActionType> RTCRoomJoinActionType;
#endif
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        FString LobbyId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()