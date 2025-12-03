// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Lobby/JoinLobby.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

class REDPOINTEOSAPI_API FJoinLobbyById
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Lobby, JoinLobbyById, EOS_LOBBY_JOINLOBBYBYID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> LobbyId;
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

    typedef FJoinLobby::Result Result;

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()