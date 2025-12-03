// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/RTC/JoinRoomOptions.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FJoinLobby
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, JoinLobby, EOS_LOBBY_JOINLOBBY_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<TReleasableRef<EOS_HLobbyDetails>> LobbyDetails;
        ParamHelpers::TRequired<bool> bPresenceEnabled;
        ParamHelpers::TRequired<RTC::EJoinRoomOptions::Type> RTCRoomJoinFlags;
        ParamHelpers::TRequired<bool> bRTCRoomUseManualAudioInput;
        ParamHelpers::TRequired<bool> bRTCRoomUseManualAudioOutput;
        ParamHelpers::TRequired<bool> bRTCRoomLocalAudioDeviceInputStartsMuted;
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