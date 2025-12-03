// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/NullRuntimePlatformPresenceService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2047259675, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;

TOptional<bool> FNullRuntimePlatformPresenceService::IsLocalUserOnline(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    // @note: Platforms which do not provide a non-null implementation of IRuntimePlatformPresenceService always allow
    // players to be advertised via cross-platform presence.
    return TOptional<bool>();
}

void FNullRuntimePlatformPresenceService::UpdateLocalUserPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserState &InDesiredPresenceState,
    const FOnLocalUserPresenceUpdated &InCompletionDelegate)
{
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented());
}

void FNullRuntimePlatformPresenceService::UpsertRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented());
}

void FNullRuntimePlatformPresenceService::DeleteRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &InRoomId,
    bool bIsParty,
    const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate)
{
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented());
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived &FNullRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteReceived(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->PlatformStorage[CallContext->PlatformHandle].OnRoomAdvertisementInviteReceivedDelegate;
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted &FNullRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteAccepted(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->PlatformStorage[CallContext->PlatformHandle].OnRoomAdvertisementInviteAcceptedDelegate;
}

TOptional<EPresenceUserActivityState> FNullRuntimePlatformPresenceService::GetNonUnifiedFriendPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId)
{
    return TOptional<EPresenceUserActivityState>();
}

IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged &FNullRuntimePlatformPresenceService::
    OnNonUnifiedFriendPresenceChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    return this->PlatformStorage[CallContext->PlatformHandle].OnNonUnifiedFriendPresenceChangedDelegate;
}

void FNullRuntimePlatformPresenceService::SendRoomInviteToNonUnifiedFriend(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    const FRoomIdRef &RoomId,
    const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate)
{
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("Null runtime platform presence service does not implement SendRoomInviteToNonUnifiedFriend.")));
}

bool FNullRuntimePlatformPresenceService::TryOpenRoomInviteRecipientSelection(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &RoomId)
{
    return false;
}

TSharedRef<IRuntimePlatformPresenceService> NullPresenceService = MakeShared<FNullRuntimePlatformPresenceService>();

}

#endif

REDPOINT_EOS_CODE_GUARD_END()