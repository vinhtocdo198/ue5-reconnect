// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/JoinRTCRoom.h"

#if EOS_VERSION_AT_LEAST(1, 17, 0)
#include "RedpointEOSAPI/Templates/Impl/Async.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if EOS_VERSION_AT_LEAST(1, 17, 0)

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(JoinRTCRoom);

void FJoinRTCRoom::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId);
    NativeOptions.LocalUserId = *Options.LocalUserId;
    {
        auto &LocalRTCOptions = Allocator.Allocate<EOS_Lobby_LocalRTCOptions>();
        LocalRTCOptions.ApiVersion = EOS_LOBBY_LOCALRTCOPTIONS_API_LATEST;
        LocalRTCOptions.Flags = *Options.RTCRoomJoinFlags;
        LocalRTCOptions.bUseManualAudioInput = *Options.bRTCRoomUseManualAudioInput;
        LocalRTCOptions.bUseManualAudioOutput = *Options.bRTCRoomUseManualAudioOutput;
        LocalRTCOptions.bLocalAudioDeviceInputStartsMuted = *Options.bRTCRoomLocalAudioDeviceInputStartsMuted;
        NativeOptions.LocalRTCOptions = &LocalRTCOptions;
    }
}

void FJoinRTCRoom::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()