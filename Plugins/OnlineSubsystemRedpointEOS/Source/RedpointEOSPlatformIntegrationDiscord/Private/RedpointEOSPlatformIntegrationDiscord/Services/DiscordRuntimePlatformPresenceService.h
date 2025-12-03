// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Types/PlatformStorage.h"
#include "RedpointEOSPlatform/Types/UserStorage.h"
#include "discordpp.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && REDPOINT_EOS_DISCORD_ENABLED

namespace REDPOINT_EOS_FILE_NS_ID(207761676, Redpoint::EOS::Platform::Integration::Discord)
{
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

class FDiscordRuntimePlatformPresenceService : public IRuntimePlatformPresenceService,
                                               public TSharedFromThis<FDiscordRuntimePlatformPresenceService>
{
private:
    TSharedRef<discordpp::Client> DiscordClient;
    struct FUserStorage
    {
        IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived OnRoomAdvertisementInviteReceived;
        IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted OnRoomAdvertisementInviteAccepted;
        IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged OnNonUnifiedFriendPresenceChanged;
        discordpp::Activity Activity;
    };
    TUserStorage<FUserStorage> UserStorage;

public:
    FDiscordRuntimePlatformPresenceService(const TSharedRef<discordpp::Client> &InDiscordClient);
    UE_NONCOPYABLE(FDiscordRuntimePlatformPresenceService);
    virtual ~FDiscordRuntimePlatformPresenceService() override = default;

    virtual TOptional<bool> IsLocalUserOnline(const FRuntimePlatformServiceCallContextRef &CallContext) const override;
    virtual void UpdateLocalUserPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserState &InDesiredPresenceState,
        const FOnLocalUserPresenceUpdated &InCompletionDelegate) override;
    virtual void UpsertRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
        bool bIsParty,
        const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate) override;
    virtual void DeleteRoomAdvertisement(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &InRoomId,
        bool bIsParty,
        const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate) override;
    virtual FOnRoomAdvertisementInviteReceived &OnRoomAdvertisementInviteReceived(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;
    virtual FOnRoomAdvertisementInviteAccepted &OnRoomAdvertisementInviteAccepted(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;
    virtual TOptional<EPresenceUserActivityState> GetNonUnifiedFriendPresence(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId) override;
    virtual FOnNonUnifiedFriendPresenceChanged &OnNonUnifiedFriendPresenceChanged(
        const FRuntimePlatformServiceCallContextRef &CallContext) override;
    virtual void SendRoomInviteToNonUnifiedFriend(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const EOS_EExternalAccountType &FriendAccountType,
        const FString &FriendAccountId,
        const FRoomIdRef &RoomId,
        const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate) override;
    virtual bool TryOpenRoomInviteRecipientSelection(
        const FRuntimePlatformServiceCallContextRef &CallContext,
        const FRoomIdRef &RoomId) override;

    void OnLocalUserSignInAfterSystemStart(const FRuntimePlatformServiceCallContextRef &CallContext);
    void OnLocalUserSignOutBeforeSystemStop(const FRuntimePlatformServiceCallContextRef &CallContext);
};

}

namespace Redpoint::EOS::Platform::Integration::Discord
{
REDPOINT_EOS_FILE_NS_EXPORT(
    207761676,
    Redpoint::EOS::Platform::Integration::Discord,
    FDiscordRuntimePlatformPresenceService)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()