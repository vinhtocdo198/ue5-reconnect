// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Lobby/CreateLobby.h"

#include "RedpointEOSAPI/Templates/Impl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Lobby
{

REDPOINT_EOSSDK_API_CALL_ASYNC_IMPL(CreateLobby);

void FCreateLobby::MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.MaxLobbyMembers = *Options.MaxLobbyMembers;
    NativeOptions.PermissionLevel = *Options.PermissionLevel;
    NativeOptions.bPresenceEnabled = *Options.bPresenceEnabled ? EOS_TRUE : EOS_FALSE;
    NativeOptions.bAllowInvites = *Options.bAllowInvites ? EOS_TRUE : EOS_FALSE;
    NativeOptions.BucketId = Allocator.AsAnsi(*Options.BucketId);
    NativeOptions.bDisableHostMigration = *Options.bDisableHostMigration ? EOS_TRUE : EOS_FALSE;
    NativeOptions.bEnableRTCRoom = *Options.bEnableRTCRoom ? EOS_TRUE : EOS_FALSE;
    NativeOptions.LocalRTCOptions = nullptr;
    if (*Options.bEnableRTCRoom)
    {
        auto &LocalRTCOptions = Allocator.Allocate<EOS_Lobby_LocalRTCOptions>();
        LocalRTCOptions.ApiVersion = EOS_LOBBY_LOCALRTCOPTIONS_API_LATEST;
        LocalRTCOptions.Flags = *Options.RTCRoomJoinFlags;
        LocalRTCOptions.bUseManualAudioInput = *Options.bRTCRoomUseManualAudioInput;
        LocalRTCOptions.bUseManualAudioOutput = *Options.bRTCRoomUseManualAudioOutput;
        LocalRTCOptions.bLocalAudioDeviceInputStartsMuted = *Options.bRTCRoomLocalAudioDeviceInputStartsMuted;
        NativeOptions.LocalRTCOptions = &LocalRTCOptions;
    }
    NativeOptions.LobbyId = Allocator.AsAnsi(*Options.LobbyId, Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty);
    NativeOptions.bEnableJoinById = *Options.bEnableJoinById ? EOS_TRUE : EOS_FALSE;
    NativeOptions.bRejoinAfterKickRequiresInvite = *Options.bRejoinAfterKickRequiresInvite ? EOS_TRUE : EOS_FALSE;
    NativeOptions.AllowedPlatformIds = reinterpret_cast<const uint32_t *>(Allocator.AllocateArray(
        *Options.AllowedPlatformIds,
        NativeOptions.AllowedPlatformIdsCount,
        Templates::Util::EAllocationFlags::ReturnNullptrIfEmpty));
    NativeOptions.bCrossplayOptOut = *Options.bCrossplayOptOut ? EOS_TRUE : EOS_FALSE;
#if EOS_VERSION_AT_LEAST(1, 17, 0)
    NativeOptions.RTCRoomJoinActionType = *Options.RTCRoomJoinActionType;
#endif
}

void FCreateLobby::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
    Result.LobbyId = Converter.FromAnsi(NativeResult.LobbyId);
}

}

REDPOINT_EOS_CODE_GUARD_END()