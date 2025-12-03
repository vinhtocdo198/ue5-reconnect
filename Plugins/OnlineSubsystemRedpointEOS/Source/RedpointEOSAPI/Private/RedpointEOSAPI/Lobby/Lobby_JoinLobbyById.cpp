// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/JoinLobbyById.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(JoinLobbyById);

void FJoinLobbyById::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
    NativeOptions.bPresenceEnabled = *Options.bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
    {
        auto &LocalRTCOptions = Allocator.Allocate<EOS_Lobby_LocalRTCOptions>();
        LocalRTCOptions.ApiVersion = EOS_LOBBY_LOCALRTCOPTIONS_API_LATEST;
        LocalRTCOptions.Flags = *Options.RTCRoomJoinFlags;
        LocalRTCOptions.bUseManualAudioInput = *Options.bRTCRoomUseManualAudioInput;
        LocalRTCOptions.bUseManualAudioOutput = *Options.bRTCRoomUseManualAudioOutput;
        LocalRTCOptions.bLocalAudioDeviceInputStartsMuted = *Options.bRTCRoomLocalAudioDeviceInputStartsMuted;
        NativeOptions.LocalRTCOptions = &LocalRTCOptions;
    }
    NativeOptions.bCrossplayOptOut = *Options.bCrossplayOptOut ? EOS_TRUE : EOS_FALSE;
#if EOS_VERSION_AT_LEAST(1, 17, 0)
    NativeOptions.RTCRoomJoinActionType = *Options.RTCRoomJoinActionType;
#endif
}

void FJoinLobbyById::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()