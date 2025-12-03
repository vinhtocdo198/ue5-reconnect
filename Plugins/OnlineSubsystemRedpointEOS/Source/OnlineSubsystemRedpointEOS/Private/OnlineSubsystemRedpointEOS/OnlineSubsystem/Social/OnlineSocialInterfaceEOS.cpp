// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineSocialInterfaceEOS.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Error/ErrorConv.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Session/UniqueNetIdEOSSession.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/NonUnifiedUniqueNetId.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/Future.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSCore/Utils/SetArray.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSFriends/FriendSystem.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformPresenceService.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#include "RedpointEOSUserCache/UserInfo.h"
#if WITH_EDITOR
#include "Internationalization/TextLocalizationManager.h"
#endif
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4053231989, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::Error;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Session;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::UserCache;

FOnlineSocialInterfaceEOS::FOnlineSocialInterfaceEOS(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
    , LocalUserStates()
{
}

void FOnlineSocialInterfaceEOS::RegisterEvents()
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    IdentitySystem->OnUserSlotChanged().Add(IIdentitySystem::FOnUserSlotChanged::FDelegate::CreateSPLambda(
        this,
        [this](
            FIdentityUserSlot UserSlot,
            EIdentityUserSlotState OldUserSlotState,
            FIdentityUserId OldUserId,
            EIdentityUserSlotState NewUserSlotState,
            FIdentityUserId NewUserId,
            EIdentityUserSlotChangeReason ChangeReason) {
            if (OldUserSlotState == EIdentityUserSlotState::NotSignedIn &&
                NewUserSlotState == EIdentityUserSlotState::SignedIn)
            {
                this->LocalUserStates.Add(*GetUniqueNetIdRef(NewUserId), FLocalUserState());
                this->ResyncLocalUserState(GetUniqueNetIdRef(NewUserId), false);
            }
            else if (
                OldUserSlotState == EIdentityUserSlotState::SignedIn &&
                NewUserSlotState == EIdentityUserSlotState::NotSignedIn)
            {
                this->LocalUserStates.Remove(*GetUniqueNetIdRef(OldUserId));
            }
        }));

    auto FriendsSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendsSystem->OnFriendsChanged().Add(IFriendSystem::FOnFriendsChanged::FDelegate::CreateSPLambda(
        this,
        [this](const FIdentityUserId &LocalUserId, ERelationshipType ChangedRelationships) {
            auto LocalUniqueNetId = GetUniqueNetIdRef(LocalUserId);
            auto *LocalUserStatePtr = this->LocalUserStates.Find(LocalUniqueNetId);
            if (LocalUserStatePtr != nullptr)
            {
                this->ResyncLocalUserState(LocalUniqueNetId, true);
            }
        }));
    FriendsSystem->OnFriendPresenceChanged().Add(IFriendSystem::FOnFriendPresenceChanged::FDelegate::CreateSPLambda(
        this,
        [this](
            const FIdentityUserId &LocalUserId,
            const FIdentityUserId &FriendUserId,
            const FPresenceUserState &NewFriendPresenceState) {
            auto LocalUniqueNetId = GetUniqueNetIdRef(LocalUserId);
            auto *LocalUserStatePtr = this->LocalUserStates.Find(LocalUniqueNetId);
            if (LocalUserStatePtr == nullptr)
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Warning,
                    TEXT("Dropping friend presence update announcement from friend system because the local user "
                         "%s does not have friend state tracked."),
                    *GetUserIdString(LocalUserId));
                return;
            }
            auto &LocalUserState = *LocalUserStatePtr;
            auto *UnifiedFriend = LocalUserState.Unified.Find(GetUniqueNetIdRef(FriendUserId));
            if (UnifiedFriend == nullptr)
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Warning,
                    TEXT("Dropping friend presence update announcement from friend system because the remote user "
                         "%s does not have a unified friend object for the local user %s."),
                    *GetUserIdString(FriendUserId),
                    *GetUserIdString(LocalUserId));
                return;
            }
            auto NewPresence = this->CreatePresence(NewFriendPresenceState);
            (*UnifiedFriend)->PresenceState = *NewPresence;
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("Updated presence state in unified friend object for remote user %s."),
                *GetUserIdString(FriendUserId));
            this->TriggerOnPresenceReceivedDelegates(*GetUniqueNetIdRef(FriendUserId), NewPresence);
        }));
    FriendsSystem->OnNonUnifiedFriendPresenceChanged().Add(
        IFriendSystem::FOnNonUnifiedFriendPresenceChanged::FDelegate::CreateSPLambda(
            this,
            [this](
                const FIdentityUserId &LocalUserId,
                const FExternalUserInfoRef &NonUnifiedFriend,
                const EPresenceUserActivityState &NewFriendPresenceActivityState) {
                auto LocalUniqueNetId = GetUniqueNetIdRef(LocalUserId);
                auto *LocalUserStatePtr = this->LocalUserStates.Find(LocalUniqueNetId);
                if (LocalUserStatePtr == nullptr)
                {
                    return;
                }
                auto &LocalUserState = *LocalUserStatePtr;
                auto *ExternalId = LocalUserState.ExternalUsersToIds.Find(NonUnifiedFriend);
                if (ExternalId == nullptr)
                {
                    return;
                }
                auto *Object = LocalUserState.NonUnified.Find(*ExternalId);
                if (Object == nullptr)
                {
                    return;
                }
                auto NewPresence = MakeShared<FOnlineUserPresence>();
                switch (NewFriendPresenceActivityState)
                {
                case EPresenceUserActivityState::Offline:
                    NewPresence->Status.State = EOnlinePresenceState::Offline;
                    break;
                case EPresenceUserActivityState::Idle:
                    NewPresence->Status.State = EOnlinePresenceState::Away;
                    break;
                case EPresenceUserActivityState::DoNotDisturb:
                    NewPresence->Status.State = EOnlinePresenceState::DoNotDisturb;
                    break;
                case EPresenceUserActivityState::Online:
                default:
                    NewPresence->Status.State = EOnlinePresenceState::Online;
                    break;
                }
                (*Object)->PresenceState = *NewPresence;
                this->TriggerOnPresenceReceivedDelegates(**ExternalId, NewPresence);
            }));
}

void FOnlineSocialInterfaceEOS::ResyncLocalUserStates()
{
    for (auto &LocalUserKV : this->LocalUserStates)
    {
        this->ResyncLocalUserState(LocalUserKV.Key, true);
    }
}

void FOnlineSocialInterfaceEOS::ResyncLocalUserState(const FUniqueNetIdRef &LocalUniqueNetId, bool bFireEvents)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();

    auto *LocalUserStatePtr = this->LocalUserStates.Find(LocalUniqueNetId);
    if (LocalUserStatePtr == nullptr)
    {
        return;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    auto LocalUserId = GetAccountId(LocalUniqueNetId);
    int32 LocalUserNum = 0;
    auto IdentityUser = IdentitySystem->GetUser(LocalUserId);
    if (IdentityUser.IsValid())
    {
        LocalUserNum = IdentityUser->GetUserSlot();
    }

    TAccountIdMap<FUserInfoRef> UnifiedFriends;
    TArray<FExternalUserInfoRef> NonUnifiedFriends;
    auto Error = FriendSystem->GetFriends(LocalUserId, UnifiedFriends, NonUnifiedFriends);
    if (!Error.WasSuccessful())
    {
        if (!GIsAutomationTesting)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("Failed to synchronise friends (GetFriends call failed): %s"),
                *Error.ToLogString());
        }
        return;
    }

    TAccountIdMap<FUserInfoRef> InboundFriendRequests;
    TAccountIdMap<FUserInfoRef> OutboundFriendRequests;
    Error = FriendSystem->GetInboundFriendRequests(LocalUserId, InboundFriendRequests);
    if (!Error.WasSuccessful())
    {
        if (!GIsAutomationTesting)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("Failed to synchronise friends (GetInboundFriendRequests call failed): %s"),
                *Error.ToLogString());
        }
        return;
    }
    Error = FriendSystem->GetOutboundFriendRequests(LocalUserId, OutboundFriendRequests);
    if (!Error.WasSuccessful())
    {
        if (!GIsAutomationTesting)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("Failed to synchronise friends (GetOutboundFriendRequests call failed): %s"),
                *Error.ToLogString());
        }
        return;
    }

    TAccountIdMap<FUserInfoPtr> BlockedPlayers;
    TAccountIdMap<TTuple<FUserInfoRef, FDateTime>> RecentPlayers;
    Error = FriendSystem->GetBlockedPlayers(LocalUserId, BlockedPlayers);
    if (!Error.WasSuccessful())
    {
        if (!GIsAutomationTesting)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("Failed to synchronise friends (GetBlockedPlayers call failed): %s"),
                *Error.ToLogString());
        }
        return;
    }
    Error = FriendSystem->GetRecentPlayers(LocalUserId, RecentPlayers);
    if (!Error.WasSuccessful())
    {
        if (!GIsAutomationTesting)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Error,
                TEXT("Failed to synchronise friends (GetRecentPlayers call failed): %s"),
                *Error.ToLogString());
        }
        return;
    }

    TMap<UE::Online::FAccountId, TTuple<FUserInfoRef, FString>> ExpectedUnifiedIds;
    for (const auto &KV : UnifiedFriends)
    {
        ExpectedUnifiedIds.Add(KV.Key, TTuple<FUserInfoRef, FString>(KV.Value, TEXT("unified-friend")));
    }
    for (const auto &KV : InboundFriendRequests)
    {
        if (!ExpectedUnifiedIds.Contains(KV.Key))
        {
            ExpectedUnifiedIds.Add(KV.Key, TTuple<FUserInfoRef, FString>(KV.Value, TEXT("inbound-friend-puid-only")));
        }
    }
    for (const auto &KV : OutboundFriendRequests)
    {
        if (!ExpectedUnifiedIds.Contains(KV.Key))
        {
            ExpectedUnifiedIds.Add(KV.Key, TTuple<FUserInfoRef, FString>(KV.Value, TEXT("outbound-friend-puid-only")));
        }
    }
    for (const auto &KV : BlockedPlayers)
    {
        if (KV.Value.IsValid())
        {
            if (!ExpectedUnifiedIds.Contains(KV.Key))
            {
                ExpectedUnifiedIds.Add(
                    KV.Key,
                    TTuple<FUserInfoRef, FString>(KV.Value.ToSharedRef(), TEXT("blocked-player-puid-only")));
            }
        }
        else
        {
            // @todo: Need a temporary object for blocked users we couldn't query for so they're still in the list.
        }
    }
    for (const auto &KV : RecentPlayers)
    {
        if (!ExpectedUnifiedIds.Contains(KV.Key))
        {
            ExpectedUnifiedIds.Add(
                KV.Key,
                TTuple<FUserInfoRef, FString>(KV.Value.Get<0>(), TEXT("recent-player-puid-only")));
        }
    }

    // Add/construct any friend objects we need to for friends, invites, recent players and blocked players.
    for (const auto &UnifiedKV : ExpectedUnifiedIds)
    {
        auto *ExistingEntry = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (ExistingEntry == nullptr)
        {
            LocalUserState.Unified.Add(
                *GetUniqueNetIdRef(UnifiedKV.Key),
                MakeShared<FOnlineFriendUnifiedEOS>(
                    this->PlatformHandle,
                    LocalUserId,
                    UnifiedKV.Value.Get<0>(),
                    UnifiedKV.Value.Get<1>()));
        }
        else if ((*ExistingEntry)->GetSource() != UnifiedKV.Value.Get<1>())
        {
            // The source or user info may have changed, such as when 'recent-player-puid-only' or
            // 'inbound-friend-puid-only' becomes 'unified-friend'. We have to replace the underlying user info
            // reference when this happens, so that attribute lookups are correct (because a 'unified-friend' will have
            // a user info object with more attributes than a '*-puid-only' source. Replacing the source and user info
            // into the existing object means that we'll still fire events correctly below, as UpdateFriendAndSource
            // doesn't update any other state.
            (*ExistingEntry)->UpdateFriendAndSource(UnifiedKV.Value.Get<0>(), UnifiedKV.Value.Get<1>());
        }
    }

    bool bFriendsUpdated = false;
    bool bOutgoingInviteAdded = false;
    bool bBlockedPlayersUpdated = false;
    TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayersAdded;
    TArray<FUniqueNetIdRef> RemovedFriends;
    TArray<FUniqueNetIdRef> InvitesAborted;
    TArray<FUniqueNetIdRef> InvitesAccepted;
    TArray<FUniqueNetIdRef> InvitesRejected;
    TArray<FUniqueNetIdRef> InvitesReceived;
    TMap<FUniqueNetIdRef, TSharedRef<FOnlineUserPresence>> PresenceUpdated;

    // Remove friend objects that should no longer exist.
    TArray<FUniqueNetIdRef> ToRemove;
    for (const auto &UnifiedKV : LocalUserState.Unified)
    {
        if (!ExpectedUnifiedIds.Contains(GetAccountId(UnifiedKV.Key)))
        {
            if (UnifiedKV.Value->InviteState == EInviteStatus::Accepted)
            {
                bFriendsUpdated = true;
                RemovedFriends.Add(UnifiedKV.Key);
            }
            if (UnifiedKV.Value->InviteState == EInviteStatus::Blocked)
            {
                bBlockedPlayersUpdated = true;
            }
            if (UnifiedKV.Value->InviteState == EInviteStatus::PendingInbound)
            {
                // We know this is a cancellation since the entry would not be removed if the user simply got moved
                // to the mutual friends list.
                InvitesAborted.Add(UnifiedKV.Key);
            }
            if (UnifiedKV.Value->InviteState == EInviteStatus::PendingOutbound)
            {
                // We know this is a rejection since the entry would not be removed if the user simply got moved to
                // the mutual friends list.
                InvitesRejected.Add(UnifiedKV.Key);
            }

            ToRemove.Add(UnifiedKV.Key);
        }
    }
    for (const auto &Id : ToRemove)
    {
        LocalUserState.Unified.Remove(*Id);
    }

    // Update objects for mutual friends.
    for (const auto &UnifiedKV : UnifiedFriends)
    {
        auto *Object = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (Object != nullptr && (*Object)->InviteState != EInviteStatus::Accepted)
        {
            if ((*Object)->InviteState == EInviteStatus::Blocked)
            {
                bBlockedPlayersUpdated = true;
            }
            if ((*Object)->InviteState == EInviteStatus::PendingInbound)
            {
                // The local user is the one that accepted the invite, so no event to fire.
            }
            if ((*Object)->InviteState == EInviteStatus::PendingOutbound)
            {
                InvitesAccepted.Add(GetUniqueNetIdRef(UnifiedKV.Key));
            }
            (*Object)->InviteState = EInviteStatus::Accepted;
            bFriendsUpdated = true;
        }
    }

    // Update objects for inbound and outbound friend requests.
    for (const auto &UnifiedKV : InboundFriendRequests)
    {
        auto *Object = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (Object != nullptr && (*Object)->InviteState != EInviteStatus::PendingInbound)
        {
            if ((*Object)->InviteState == EInviteStatus::Blocked)
            {
                bBlockedPlayersUpdated = true;
            }
            if ((*Object)->InviteState == EInviteStatus::Accepted)
            {
                bFriendsUpdated = true;
            }
            (*Object)->InviteState = EInviteStatus::PendingInbound;

            // The local user just received this invite.
            InvitesReceived.Add(GetUniqueNetIdRef(UnifiedKV.Key));
        }
    }
    for (const auto &UnifiedKV : OutboundFriendRequests)
    {
        auto *Object = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (Object != nullptr && (*Object)->InviteState != EInviteStatus::PendingOutbound)
        {
            if ((*Object)->InviteState == EInviteStatus::Blocked)
            {
                bBlockedPlayersUpdated = true;
            }
            if ((*Object)->InviteState == EInviteStatus::Accepted)
            {
                bFriendsUpdated = true;
            }
            (*Object)->InviteState = EInviteStatus::PendingOutbound;
            bOutgoingInviteAdded = true;
        }
    }

    // Update objects for blocked players.
    for (const auto &UnifiedKV : BlockedPlayers)
    {
        auto *Object = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (Object != nullptr && (*Object)->InviteState != EInviteStatus::Blocked)
        {
            if ((*Object)->InviteState == EInviteStatus::Accepted)
            {
                bFriendsUpdated = true;
            }
            (*Object)->InviteState = EInviteStatus::Blocked;
            bBlockedPlayersUpdated = true;
        }
    }

    // Update objects for recent players.
    for (const auto &UnifiedKV : RecentPlayers)
    {
        auto *Object = LocalUserState.Unified.Find(GetUniqueNetIdRef(UnifiedKV.Key));
        if (Object != nullptr)
        {
            if (!(*Object)->LastSeen.IsSet())
            {
                RecentPlayersAdded.Add(*Object);
            }
            (*Object)->LastSeen = UnifiedKV.Value.Get<1>();

            // Check if a mutual friend is now only a recent player (if they go via another status, they won't still be
            // in 'Accepted' at this point)>
            if ((*Object)->InviteState == EInviteStatus::Accepted && !UnifiedFriends.Contains(UnifiedKV.Key))
            {
                (*Object)->InviteState = EInviteStatus::Unknown;
                bFriendsUpdated = true;
                RemovedFriends.Add(GetUniqueNetIdRef(UnifiedKV.Key));
            }

            // Check if an incoming friend request is now only a recent player (if they go via another status, they
            // won't still be in 'PendingInbound' at this point)>
            if ((*Object)->InviteState == EInviteStatus::PendingInbound &&
                !InboundFriendRequests.Contains(UnifiedKV.Key))
            {
                (*Object)->InviteState = EInviteStatus::Unknown;
                InvitesAborted.Add(GetUniqueNetIdRef(UnifiedKV.Key));
            }

            // Check if an outgoing friend request is now only a recent player (if they go via another status, they
            // won't still be in 'PendingOutbound' at this point)>
            if ((*Object)->InviteState == EInviteStatus::PendingOutbound &&
                !OutboundFriendRequests.Contains(UnifiedKV.Key))
            {
                (*Object)->InviteState = EInviteStatus::Unknown;
                InvitesRejected.Add(GetUniqueNetIdRef(UnifiedKV.Key));
            }

            // Check if a blocked player is now only a recent player (if they go via another status, they
            // won't still be in 'PendingOutbound' at this point)>
            if ((*Object)->InviteState == EInviteStatus::Blocked && !BlockedPlayers.Contains(UnifiedKV.Key))
            {
                (*Object)->InviteState = EInviteStatus::Unknown;
                bBlockedPlayersUpdated = true;
            }
        }
    }

    // Update presence for players.
    for (const auto &UnifiedKV : LocalUserState.Unified)
    {
        auto PresenceState = FriendSystem->GetUnifiedFriendPresence(LocalUserId, GetAccountId(UnifiedKV.Key));
        if (PresenceState.IsSet())
        {
            auto ConvertedPresence = this->CreatePresence(PresenceState.GetValue());
            if (ConvertedPresence->SessionId != UnifiedKV.Value->PresenceState.SessionId ||
                ConvertedPresence->bIsOnline != UnifiedKV.Value->PresenceState.bIsOnline ||
                ConvertedPresence->bIsPlaying != UnifiedKV.Value->PresenceState.bIsPlaying ||
                ConvertedPresence->bIsPlayingThisGame != UnifiedKV.Value->PresenceState.bIsPlayingThisGame ||
                ConvertedPresence->bIsJoinable != UnifiedKV.Value->PresenceState.bIsJoinable ||
                ConvertedPresence->bHasVoiceSupport != UnifiedKV.Value->PresenceState.bHasVoiceSupport ||
                ConvertedPresence->LastOnline != UnifiedKV.Value->PresenceState.LastOnline ||
                ConvertedPresence->Status.State != UnifiedKV.Value->PresenceState.Status.State ||
                ConvertedPresence->Status.StatusStr != UnifiedKV.Value->PresenceState.Status.StatusStr)
            {
                UnifiedKV.Value->PresenceState = *ConvertedPresence;
                PresenceUpdated.Add(UnifiedKV.Key, ConvertedPresence);
            }
        }
    }

    // Add any non-unified friend objects that should exist.
    TSet<FUniqueNetIdRef> NonUnifiedIds;
    for (const auto &NonUnifiedKV : NonUnifiedFriends)
    {
        auto *ExistingId = LocalUserState.ExternalUsersToIds.Find(NonUnifiedKV);
        if (ExistingId != nullptr)
        {
            NonUnifiedIds.Add(*ExistingId);
            if (LocalUserState.NonUnified.Contains(ExistingId->Get()) ||
                LocalUserState.Synthetic.Contains(ExistingId->Get()))
            {
                continue;
            }
        }

        auto SyntheticFriend = TryGetSyntheticFriend(LocalUserNum, NonUnifiedKV);
        if (SyntheticFriend.IsValid())
        {
            LocalUserState.Synthetic.Add(*SyntheticFriend->GetUserId(), SyntheticFriend.ToSharedRef());
            LocalUserState.ExternalUsersToIds.Add(NonUnifiedKV, SyntheticFriend->GetUserId());
            NonUnifiedIds.Add(SyntheticFriend->GetUserId());
            bFriendsUpdated = true;
        }
        else
        {
            auto NonUnifiedExternalId = GetPrimaryExternalUserInfoId(NonUnifiedKV);
            if (NonUnifiedExternalId.IsSet())
            {
                auto NonUnifiedId =
                    MakeShared<FNonUnifiedUniqueNetId>(NonUnifiedExternalId->Get<0>(), NonUnifiedExternalId->Get<1>());
                NonUnifiedIds.Add(NonUnifiedId);
                if (!LocalUserState.NonUnified.Contains(*NonUnifiedId))
                {
                    LocalUserState.NonUnified.Add(
                        *NonUnifiedId,
                        MakeShared<FOnlineFriendNonUnifiedEOS>(
                            NonUnifiedId,
                            this->PlatformHandle,
                            LocalUserId,
                            NonUnifiedKV));
                }
                LocalUserState.ExternalUsersToIds.Add(NonUnifiedKV, NonUnifiedId);
            }
        }
    }

    // Update invite state for any added non-unified friend objects.
    for (const auto &NonUnifiedKV : NonUnifiedFriends)
    {
        auto *ExistingId = LocalUserState.ExternalUsersToIds.Find(NonUnifiedKV);
        if (ExistingId == nullptr)
        {
            continue;
        }

        auto *NonUnified = LocalUserState.NonUnified.Find(*ExistingId);
        if (NonUnified == nullptr)
        {
            continue;
        }

        if ((*NonUnified)->InviteState != EInviteStatus::Accepted)
        {
            (*NonUnified)->InviteState = EInviteStatus::Accepted;
            bFriendsUpdated = true;
        }

        auto PresenceActivityState = FriendSystem->GetNonUnifiedFriendPresence(LocalUserId, NonUnifiedKV);
        if (PresenceActivityState.IsSet())
        {
            if ((*NonUnified)->PresenceState.Status.State != ConvertPresence(PresenceActivityState.GetValue()))
            {
                (*NonUnified)->PresenceState.Status.State = ConvertPresence(PresenceActivityState.GetValue());
                PresenceUpdated.Add(*ExistingId, MakeShared<FOnlineUserPresence>((*NonUnified)->PresenceState));
            }
        }
    }

    // Remove any non-unified and synthetic friends that should no longer be in those entries.
    TSet<FUniqueNetIdRef> ToRemoveNonUnified;
    for (const auto &KV : LocalUserState.Synthetic)
    {
        if (!NonUnifiedIds.Contains(KV.Key))
        {
            ToRemoveNonUnified.Add(KV.Key);
            RemovedFriends.Add(KV.Key);
        }
    }
    for (const auto &KV : LocalUserState.NonUnified)
    {
        if (!NonUnifiedIds.Contains(KV.Key))
        {
            ToRemoveNonUnified.Add(KV.Key);
            RemovedFriends.Add(KV.Key);
        }
    }
    for (const auto &Id : ToRemoveNonUnified)
    {
        LocalUserState.Synthetic.Remove(*Id);
        LocalUserState.NonUnified.Remove(*Id);
        bFriendsUpdated = true;
    }

    // Fire events.
    if (bFireEvents)
    {
        for (const auto &Id : RemovedFriends)
        {
            this->TriggerOnFriendRemovedDelegates(*GetUniqueNetIdRef(LocalUserId), *Id);
        }
        if (bBlockedPlayersUpdated)
        {
            this->TriggerOnBlockListChangeDelegates(LocalUserNum, TEXT(""));
        }
        if (bFriendsUpdated)
        {
            this->TriggerOnFriendsChangeDelegates(LocalUserNum);
        }
        for (const auto &Id : InvitesAborted)
        {
            this->TriggerOnInviteAbortedDelegates(*GetUniqueNetIdRef(LocalUserId), *Id);
        }
        for (const auto &Id : InvitesAccepted)
        {
            this->TriggerOnInviteAcceptedDelegates(*GetUniqueNetIdRef(LocalUserId), *Id);
        }
        for (const auto &Id : InvitesReceived)
        {
            this->TriggerOnInviteReceivedDelegates(*GetUniqueNetIdRef(LocalUserId), *Id);
        }
        for (const auto &Id : InvitesRejected)
        {
            this->TriggerOnInviteRejectedDelegates(*GetUniqueNetIdRef(LocalUserId), *Id);
        }
        for (const auto &KV : PresenceUpdated)
        {
            this->TriggerOnPresenceReceivedDelegates(*KV.Key, KV.Value);
        }
        if (bOutgoingInviteAdded)
        {
            this->TriggerOnOutgoingInviteSentDelegates(LocalUserNum);
        }
        if (RecentPlayersAdded.Num() > 0)
        {
            this->TriggerOnRecentPlayersAddedDelegates(*GetUniqueNetIdRef(LocalUserId), RecentPlayersAdded);
        }
    }
}

TOptional<TTuple<EOS_EExternalAccountType, FString>> FOnlineSocialInterfaceEOS::GetPrimaryExternalUserInfoId(
    const FExternalUserInfoRef &ExternalUserInfo)
{
    if (ExternalUserInfo->GetLinkedExternalAccounts().Num() == 0)
    {
        return TOptional<TTuple<EOS_EExternalAccountType, FString>>();
    }

    EOS_EExternalAccountType ExternalAccountType;
    FString ExternalAccountId;
    for (const auto &KV : ExternalUserInfo->GetLinkedExternalAccounts())
    {
        ExternalAccountType = KV.Key;
        ExternalAccountId = KV.Value.AccountId;
        break;
    }

    return TTuple<EOS_EExternalAccountType, FString>(ExternalAccountType, ExternalAccountId);
}

TSharedPtr<FOnlineFriend> FOnlineSocialInterfaceEOS::TryGetSyntheticFriend(
    int32 LocalUserNum,
    const FExternalUserInfoRef &ExternalUserInfo)
{
    auto ExternalAccount = this->GetPrimaryExternalUserInfoId(ExternalUserInfo);
    if (!ExternalAccount.IsSet())
    {
        return nullptr;
    }

    FName SyntheticSubsystem = NAME_None;
    if (ExternalAccount->Get<0>() == EOS_EExternalAccountType::EOS_EAT_STEAM)
    {
        SyntheticSubsystem = STEAM_SUBSYSTEM;
    }
    else if (
        ExternalAccount->Get<0>() == EOS_EExternalAccountType::EOS_EAT_XBL ||
        ExternalAccount->Get<0>() == EOS_EExternalAccountType::EOS_EAT_PSN ||
        ExternalAccount->Get<0>() == EOS_EExternalAccountType::EOS_EAT_NINTENDO)
    {
        auto PlatformSubsystem = IOnlineSubsystem::GetByPlatform();
        if (PlatformSubsystem != nullptr)
        {
            SyntheticSubsystem = PlatformSubsystem->GetSubsystemName();
        }
    }

    if (SyntheticSubsystem.IsNone())
    {
        return nullptr;
    }

    auto World = FWorldResolution::GetWorld(this->PlatformHandle->GetEnvironment()->InstanceName, true);
    auto *Subsystem = Online::GetSubsystem(World.Get(), SyntheticSubsystem);
    if (Subsystem != nullptr)
    {
        auto Friends = Subsystem->GetFriendsInterface();
        auto Identity = Subsystem->GetIdentityInterface();
        if (Identity.IsValid() && Friends.IsValid())
        {
            auto SyntheticFriendId = Identity->CreateUniquePlayerId(ExternalAccount->Get<1>());
            if (SyntheticFriendId.IsValid())
            {
                return Friends->GetFriend(LocalUserNum, *SyntheticFriendId, TEXT(""));
            }
        }
    }

    return nullptr;
}

bool FOnlineSocialInterfaceEOS::ReadFriendsList(
    int32 LocalUserNum,
    const FString &ListName,
    const FOnReadFriendsListComplete &Delegate)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserNum, false, ListName, TEXT("Local user is not signed in."));
        return true;
    }

    // IFriendSystem ensures that we always have the friends list after signing in, and it keeps it up-to-date,
    // so there's no querying that needs to happen in this call.
    Delegate.ExecuteIfBound(LocalUserNum, true, ListName, TEXT(""));
    return true;
}

bool FOnlineSocialInterfaceEOS::DeleteFriendsList(
    int32 LocalUserNum,
    const FString &ListName,
    const FOnDeleteFriendsListComplete &Delegate)
{
    Delegate.ExecuteIfBound(LocalUserNum, false, ListName, TEXT("Deleting friends lists is not supported."));
    return true;
}

bool FOnlineSocialInterfaceEOS::SendInvite(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FOnSendInviteComplete &Delegate)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, TEXT("Local user is not signed in."));
        return true;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->SendFriendRequest(IdentityUser->GetUserId(), GetAccountId(FriendId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName, Delegate](
                IFriendSystem::FSendFriendRequestResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    Delegate.ExecuteIfBound(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        TEXT(""));
                }
            },
            [](IFriendSystem::FSendFriendRequestResult Result) {
            }));
    return true;
}

bool FOnlineSocialInterfaceEOS::AcceptInvite(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FOnAcceptInviteComplete &Delegate)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        Delegate.ExecuteIfBound(LocalUserNum, false, FriendId, ListName, TEXT("Local user is not signed in."));
        return true;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->AcceptFriendRequest(IdentityUser->GetUserId(), GetAccountId(FriendId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName, Delegate](
                IFriendSystem::FAcceptFriendRequestResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    Delegate.ExecuteIfBound(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    Delegate.ExecuteIfBound(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        TEXT(""));
                }
            },
            [](IFriendSystem::FAcceptFriendRequestResult Result) {
            }));
    return true;
}

bool FOnlineSocialInterfaceEOS::RejectInvite(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("RejectInvite: Local user is not signed in."));
        return false;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->RejectFriendRequest(IdentityUser->GetUserId(), GetAccountId(FriendId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName](
                IFriendSystem::FRejectFriendRequestResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    this->TriggerOnRejectInviteCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        *Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    this->TriggerOnRejectInviteCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        TEXT(""));
                }
            },
            [](IFriendSystem::FRejectFriendRequestResult Result) {
            }));
    return true;
}

void FOnlineSocialInterfaceEOS::SetFriendAlias(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FString &Alias,
    const FOnSetFriendAliasComplete &Delegate)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        Delegate.ExecuteIfBound(
            LocalUserNum,
            FriendId,
            ListName,
            AsOSSError(Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Local user is not signed in."))));
        return;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->SetFriendAlias(IdentityUser->GetUserId(), GetAccountId(FriendId), Alias)
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName, Delegate](
                IFriendSystem::FSetFriendAliasResult Result) {
                this->ResyncLocalUserStates();
                Delegate.ExecuteIfBound(
                    IdentityUser->GetUserSlot(),
                    *GetUniqueNetIdRef(FriendUserId),
                    ListName,
                    AsOSSError(Result.ErrorCode));
            },
            [](IFriendSystem::FSetFriendAliasResult Result) {
            }));
    return;
}

void FOnlineSocialInterfaceEOS::DeleteFriendAlias(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName,
    const FOnDeleteFriendAliasComplete &Delegate)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        Delegate.ExecuteIfBound(
            LocalUserNum,
            FriendId,
            ListName,
            AsOSSError(Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Local user is not signed in."))));
        return;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->DeleteFriendAlias(IdentityUser->GetUserId(), GetAccountId(FriendId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName, Delegate](
                IFriendSystem::FDeleteFriendAliasResult Result) {
                this->ResyncLocalUserStates();
                Delegate.ExecuteIfBound(
                    IdentityUser->GetUserSlot(),
                    *GetUniqueNetIdRef(FriendUserId),
                    ListName,
                    AsOSSError(Result.ErrorCode));
            },
            [](IFriendSystem::FDeleteFriendAliasResult Result) {
            }));
    return;
}

bool FOnlineSocialInterfaceEOS::DeleteFriend(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("DeleteFriend: Local user is not signed in."));
        return false;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->DeleteFriend(IdentityUser->GetUserId(), GetAccountId(FriendId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, FriendUserId = GetAccountId(FriendId), ListName](
                IFriendSystem::FDeleteFriendResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    this->TriggerOnDeleteFriendCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        *Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    this->TriggerOnDeleteFriendCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(FriendUserId),
                        ListName,
                        TEXT(""));
                }
            },
            [](IFriendSystem::FDeleteFriendResult Result) {
            }));
    return true;
}

bool FOnlineSocialInterfaceEOS::GetFriendsList(
    int32 LocalUserNum,
    const FString &ListName,
    TArray<TSharedRef<FOnlineFriend>> &OutFriends)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        return false;
    }

    auto *LocalUserStatePtr = this->LocalUserStates.Find(GetUniqueNetIdRef(IdentityUser->GetUserId()));
    if (LocalUserStatePtr == nullptr)
    {
        return false;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    OutFriends.Empty();
    for (const auto &KV : LocalUserState.Unified)
    {
        if (KV.Value->InviteState == EInviteStatus::Accepted ||
            KV.Value->InviteState == EInviteStatus::PendingInbound ||
            KV.Value->InviteState == EInviteStatus::PendingOutbound)
        {
            OutFriends.Add(KV.Value);
        }
    }
    for (const auto &KV : LocalUserState.NonUnified)
    {
        if (KV.Value->InviteState == EInviteStatus::Accepted ||
            KV.Value->InviteState == EInviteStatus::PendingInbound ||
            KV.Value->InviteState == EInviteStatus::PendingOutbound)
        {
            OutFriends.Add(KV.Value);
        }
    }
    for (const auto &KV : LocalUserState.Synthetic)
    {
        OutFriends.Add(KV.Value);
    }
    return true;
}

TSharedPtr<FOnlineFriend> FOnlineSocialInterfaceEOS::GetFriend(
    int32 LocalUserNum,
    const FUniqueNetId &FriendId,
    const FString &ListName)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        return nullptr;
    }

    auto *LocalUserStatePtr = this->LocalUserStates.Find(GetUniqueNetIdRef(IdentityUser->GetUserId()));
    if (LocalUserStatePtr == nullptr)
    {
        return nullptr;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    if (LocalUserState.Unified.Contains(FriendId))
    {
        return LocalUserState.Unified[FriendId];
    }
    if (LocalUserState.NonUnified.Contains(FriendId))
    {
        return LocalUserState.NonUnified[FriendId];
    }
    if (LocalUserState.Synthetic.Contains(FriendId))
    {
        return LocalUserState.Synthetic[FriendId];
    }
    return nullptr;
}

bool FOnlineSocialInterfaceEOS::IsFriend(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        return false;
    }

    auto *LocalUserStatePtr = this->LocalUserStates.Find(GetUniqueNetIdRef(IdentityUser->GetUserId()));
    if (LocalUserStatePtr == nullptr)
    {
        return false;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    if (LocalUserState.Unified.Contains(FriendId))
    {
        return LocalUserState.Unified[FriendId]->GetInviteStatus() == EInviteStatus::Accepted;
    }
    if (LocalUserState.NonUnified.Contains(FriendId))
    {
        return LocalUserState.NonUnified[FriendId]->GetInviteStatus() == EInviteStatus::Accepted;
    }
    if (LocalUserState.Synthetic.Contains(FriendId))
    {
        return LocalUserState.Synthetic[FriendId]->GetInviteStatus() == EInviteStatus::Accepted;
    }
    return false;
}

void FOnlineSocialInterfaceEOS::AddRecentPlayers(
    const FUniqueNetId &UserId,
    const TArray<FReportPlayedWithUser> &InRecentPlayers,
    const FString &ListName,
    const FOnAddRecentPlayersComplete &InCompletionDelegate)
{
    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    for (const auto &RecentPlayer : InRecentPlayers)
    {
        FriendSystem->RecordRecentPlayer(GetAccountId(RecentPlayer.UserId));
    }
    this->ResyncLocalUserStates();
    InCompletionDelegate.ExecuteIfBound(UserId, AsOSSError(Errors::Success()));
}

bool FOnlineSocialInterfaceEOS::QueryRecentPlayers(const FUniqueNetId &UserId, const FString &Namespace)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(GetAccountId(UserId));
    if (!IdentityUser.IsValid())
    {
        this->TriggerOnQueryRecentPlayersCompleteDelegates(
            UserId,
            Namespace,
            false,
            TEXT("Local user is not signed in."));
        return true;
    }

    // IFriendSystem ensures that we always have the recent players list after signing in, and it keeps it
    // up-to-date, so there's no querying that needs to happen in this call.
    this->TriggerOnQueryRecentPlayersCompleteDelegates(UserId, Namespace, true, TEXT(""));
    return true;
}

bool FOnlineSocialInterfaceEOS::GetRecentPlayers(
    const FUniqueNetId &UserId,
    const FString &Namespace,
    TArray<TSharedRef<FOnlineRecentPlayer>> &OutRecentPlayers)
{
    if (!UserId.DoesSharedInstanceExist())
    {
        return false;
    }
    auto *LocalUserStatePtr = this->LocalUserStates.Find(UserId.AsShared());
    if (LocalUserStatePtr == nullptr)
    {
        return false;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    OutRecentPlayers.Empty();
    for (const auto &KV : LocalUserState.Unified)
    {
        if (KV.Value->LastSeen.IsSet())
        {
            OutRecentPlayers.Add(KV.Value);
        }
    }
    return true;
}

void FOnlineSocialInterfaceEOS::DumpRecentPlayers() const
{
    for (const auto &StateKV : this->LocalUserStates)
    {
        TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
        for (const auto &KV : StateKV.Value.Unified)
        {
            if (KV.Value->LastSeen.IsSet())
            {
                RecentPlayers.Add(KV.Value);
            }
        }

        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("DumpRecentPlayers: %s: %d recent players"),
            *GetUserIdString(GetAccountId(StateKV.Key)),
            RecentPlayers.Num());
        for (const auto &RecentPlayer : RecentPlayers)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("DumpRecentPlayers: %s: - %s"),
                *GetUserIdString(GetAccountId(StateKV.Key)),
                *GetUserIdString(GetAccountId(RecentPlayer->GetUserId())));
        }
    }
}

bool FOnlineSocialInterfaceEOS::BlockPlayer(int32 LocalUserNum, const FUniqueNetId &PlayerId)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("BlockPlayer: Local user is not signed in."));
        return false;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->BlockPlayer(IdentityUser->GetUserId(), GetAccountId(PlayerId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, TargetUserId = GetAccountId(PlayerId)](IFriendSystem::FBlockPlayerResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    this->TriggerOnBlockedPlayerCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(TargetUserId),
                        TEXT(""),
                        *Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    this->TriggerOnBlockedPlayerCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(TargetUserId),
                        TEXT(""),
                        TEXT(""));
                }
            },
            [](IFriendSystem::FBlockPlayerResult Result) {
            }));
    return true;
}

bool FOnlineSocialInterfaceEOS::UnblockPlayer(int32 LocalUserNum, const FUniqueNetId &PlayerId)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        UE_LOG(LogOnlineSubsystemRedpointEOS, Error, TEXT("UnblockPlayer: Local user is not signed in."));
        return false;
    }

    auto FriendSystem = this->PlatformHandle->GetSystem<IFriendSystem>();
    FriendSystem->UnblockPlayer(IdentityUser->GetUserId(), GetAccountId(PlayerId))
        .Next(IfBoundValue(
            this,
            [this, IdentityUser, TargetUserId = GetAccountId(PlayerId)](IFriendSystem::FUnblockPlayerResult Result) {
                if (!Result.ErrorCode.WasSuccessful())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("%s: %s"),
                        ANSI_TO_TCHAR(__FUNCTION__),
                        *Result.ErrorCode.ToLogString());
                    this->TriggerOnUnblockedPlayerCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        false,
                        *GetUniqueNetIdRef(TargetUserId),
                        TEXT(""),
                        *Result.ErrorCode.ToLogString());
                }
                else
                {
                    this->ResyncLocalUserStates();
                    this->TriggerOnUnblockedPlayerCompleteDelegates(
                        IdentityUser->GetUserSlot(),
                        true,
                        *GetUniqueNetIdRef(TargetUserId),
                        TEXT(""),
                        TEXT(""));
                }
            },
            [](IFriendSystem::FUnblockPlayerResult Result) {
            }));
    return true;
}

bool FOnlineSocialInterfaceEOS::QueryBlockedPlayers(const FUniqueNetId &UserId)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(GetAccountId(UserId));
    if (!IdentityUser.IsValid())
    {
        this->TriggerOnQueryBlockedPlayersCompleteDelegates(UserId, false, TEXT("Local user is not signed in."));
        return true;
    }

    // IFriendSystem ensures that we always have the blocked players list after signing in, and it keeps it
    // up-to-date, so there's no querying that needs to happen in this call.
    this->TriggerOnQueryBlockedPlayersCompleteDelegates(UserId, true, TEXT(""));
    return true;
}

bool FOnlineSocialInterfaceEOS::GetBlockedPlayers(
    const FUniqueNetId &UserId,
    TArray<TSharedRef<FOnlineBlockedPlayer>> &OutBlockedPlayers)
{
    if (!UserId.DoesSharedInstanceExist())
    {
        return false;
    }
    auto *LocalUserStatePtr = this->LocalUserStates.Find(UserId.AsShared());
    if (LocalUserStatePtr == nullptr)
    {
        return false;
    }
    auto &LocalUserState = *LocalUserStatePtr;

    OutBlockedPlayers.Empty();
    for (const auto &KV : LocalUserState.Unified)
    {
        if (KV.Value->InviteState == EInviteStatus::Blocked)
        {
            OutBlockedPlayers.Add(KV.Value);
        }
    }
    return true;
}

void FOnlineSocialInterfaceEOS::DumpBlockedPlayers() const
{
    for (const auto &StateKV : this->LocalUserStates)
    {
        TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
        for (const auto &KV : StateKV.Value.Unified)
        {
            if (KV.Value->InviteState == EInviteStatus::Blocked)
            {
                BlockedPlayers.Add(KV.Value);
            }
        }

        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("DumpBlockedPlayers: %s: %d blocked players"),
            *GetUserIdString(GetAccountId(StateKV.Key)),
            BlockedPlayers.Num());
        for (const auto &BlockedPlayer : BlockedPlayers)
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Verbose,
                TEXT("DumpBlockedPlayers: %s: - %s"),
                *GetUserIdString(GetAccountId(StateKV.Key)),
                *GetUserIdString(GetAccountId(BlockedPlayer->GetUserId())));
        }
    }
}

void FOnlineSocialInterfaceEOS::QueryFriendSettings(
    const FUniqueNetId &LocalUserId,
    FOnSettingsOperationComplete Delegate)
{
    FFriendSettings Settings;
    Delegate.ExecuteIfBound(
        LocalUserId,
        false,
        false,
        Settings,
        *Errors::NotImplemented(
             ANSI_TO_TCHAR(__FUNCTION__),
             TEXT("Epic Online Services does not support QueryFriendSettings."))
             .ToLogString());
}

void FOnlineSocialInterfaceEOS::UpdateFriendInvitePolicySettings(
    const FUniqueNetId &LocalUserId,
    const EFriendInvitePolicy FriendInvitePolicy,
    const bool bAffectsExistingInvites,
    FOnSettingsOperationComplete Delegate)
{
    FFriendSettings Settings;
    Delegate.ExecuteIfBound(
        LocalUserId,
        false,
        false,
        Settings,
        *Errors::NotImplemented(
             ANSI_TO_TCHAR(__FUNCTION__),
             TEXT("Epic Online Services does not support UpdateFriendInvitePolicySettings."))
             .ToLogString());
}

bool FOnlineSocialInterfaceEOS::QueryFriendSettings(
    const FUniqueNetId &UserId,
    const FString &Source,
    const FOnQueryFriendSettingsComplete &Delegate)
{
    Delegate.ExecuteIfBound(
        UserId,
        false,
        *Errors::NotImplemented(
             ANSI_TO_TCHAR(__FUNCTION__),
             TEXT("Epic Online Services does not support QueryFriendSettings."))
             .ToLogString());
    return true;
}

bool FOnlineSocialInterfaceEOS::GetFriendSettings(
    const FUniqueNetId &UserId,
    TMap<FString, TSharedRef<FOnlineFriendSettingsSourceData>> &OutSettings)
{
    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Error,
        TEXT("%s"),
        *Errors::NotImplemented(
             ANSI_TO_TCHAR(__FUNCTION__),
             TEXT("Epic Online Services does not support GetFriendSettings."))
             .ToLogString());
    return false;
}

EFriendInvitePolicy FOnlineSocialInterfaceEOS::GetFriendInvitePolicy(const FUniqueNetId &UserId) const
{
    return EFriendInvitePolicy::Public;
}

bool FOnlineSocialInterfaceEOS::SetFriendSettings(
    const FUniqueNetId &UserId,
    const FString &Source,
    bool bNeverShowAgain,
    const FOnSetFriendSettingsComplete &Delegate)
{
    Delegate.ExecuteIfBound(
        UserId,
        false,
        *Errors::NotImplemented(
             ANSI_TO_TCHAR(__FUNCTION__),
             TEXT("Epic Online Services does not support SetFriendSettings."))
             .ToLogString());
    return true;
}

void FOnlineSocialInterfaceEOS::SendRoomInviteToPotentialFriend(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetIdRef &TargetFriendId,
    const FRoomRef &TargetRoom,
    const FOnSendRoomInviteToPotentialFriendComplete &OnComplete)
{
    // Find the local user state.
    auto *LocalUserState = this->LocalUserStates.Find(LocalUserId.AsShared());
    if (LocalUserState == nullptr)
    {
        OnComplete.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local user has no cross-platform friends state tracked.")));
        return;
    }

    // Find the local user slot (which is necessary for calling runtime platform integrations).
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(GetAccountId(LocalUserId));
    if (!IdentityUser.IsValid())
    {
        OnComplete.ExecuteIfBound(
            Errors::AccessDenied(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user is not signed in.")));
        return;
    }

    // Figure out what kind of target user we're dealing with.
    auto *UnifiedFriend = LocalUserState->Unified.Find(TargetFriendId);
    auto *NonUnifiedFriend = LocalUserState->NonUnified.Find(TargetFriendId);
    auto *SyntheticFriend = LocalUserState->Synthetic.Find(TargetFriendId);
    TSet<TTuple<EOS_EExternalAccountType, FString>> ExternalUserIds;
    if (UnifiedFriend != nullptr)
    {
        for (const auto &KV : (*UnifiedFriend)->GetFriend()->GetLinkedExternalAccounts())
        {
            ExternalUserIds.Add(TTuple<EOS_EExternalAccountType, FString>(KV.Key, KV.Value.AccountId));
        }
    }
    if (NonUnifiedFriend != nullptr)
    {
        for (const auto &KV : (*NonUnifiedFriend)->GetFriend()->GetLinkedExternalAccounts())
        {
            ExternalUserIds.Add(TTuple<EOS_EExternalAccountType, FString>(KV.Key, KV.Value.AccountId));
        }
    }
    if (SyntheticFriend != nullptr)
    {
        FExternalUserInfoPtr ExternalUserInfo;
        for (const auto &SyntheticKV : LocalUserState->ExternalUsersToIds)
        {
            if (*SyntheticKV.Value == *(*SyntheticFriend)->GetUserId())
            {
                ExternalUserInfo = SyntheticKV.Key;
                break;
            }
        }
        if (ExternalUserInfo.IsValid())
        {
            for (const auto &KV : ExternalUserInfo->GetLinkedExternalAccounts())
            {
                ExternalUserIds.Add(TTuple<EOS_EExternalAccountType, FString>(KV.Key, KV.Value.AccountId));
            }
        }
    }

    // If we don't have any external user IDs to send to, we can't send over the local platform.
    if (ExternalUserIds.Num() == 0)
    {
        OnComplete.ExecuteIfBound(Errors::NoConnection(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("There are no local platform implementations available to send the room invite.")));
        return;
    }

    // Ask the runtime platform integrations to send the invite.
    auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
    PresenceSystem->WaitForReconciledPresence(
        IdentityUser->GetUserId(),
        FSimpleDelegate::CreateSPLambda(this, [this, IdentityUser, ExternalUserIds, TargetRoom, OnComplete]() {
            auto CallContext =
                ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, IdentityUser->GetUserSlot());
            using FOpAccount = FMultiOperation<TTuple<EOS_EExternalAccountType, FString>, TArray<FError>>;
            using FOpRuntime = FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, FError>;
            FOpAccount::RunSP(
                this,
                SetToArray(ExternalUserIds),
                [this, TargetRoom, CallContext](
                    TTuple<EOS_EExternalAccountType, FString> ExternalAccountId,
                    TFunction<void(TArray<FError>)> OnAccountDone) -> bool {
                    FOpRuntime::RunSP(
                        this,
                        FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                        [this, TargetRoom, CallContext, ExternalAccountId](
                            TSharedRef<IRuntimePlatformIntegration> Integration,
                            TFunction<void(FError)> OnIntegrationDone) -> bool {
                            Integration->GetPresenceService()->SendRoomInviteToNonUnifiedFriend(
                                CallContext,
                                ExternalAccountId.Get<0>(),
                                ExternalAccountId.Get<1>(),
                                TargetRoom->GetRoomId(),
                                IRuntimePlatformPresenceService::FOnSendRoomInviteToNonUnifiedFriendComplete::
                                    CreateSPLambda(this, [OnIntegrationDone](FError IntegrationErrorCode) {
                                        OnIntegrationDone(IntegrationErrorCode);
                                    }));
                            return true;
                        },
                        [OnAccountDone](const TArray<FError> &Errors) {
                            OnAccountDone(Errors);
                        },
                        Errors::NotImplemented());
                    return true;
                },
                [OnComplete](const TArray<TArray<FError>> &ErrorsUngrouped) {
                    for (const auto &Errors : ErrorsUngrouped)
                    {
                        for (const auto &Error : Errors)
                        {
                            if (Error.WasSuccessful())
                            {
                                // Successfully sent the invite to the target user.
                                OnComplete.ExecuteIfBound(Error);
                                return;
                            }
                        }
                    }

                    // Otherwise return the first error.
                    for (const auto &Errors : ErrorsUngrouped)
                    {
                        for (const auto &Error : Errors)
                        {
                            OnComplete.ExecuteIfBound(Error);
                            return;
                        }
                    }

                    // If no errors at all, return a generic error.
                    OnComplete.ExecuteIfBound(Errors::NoConnection(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("There are no local platform implementations available to send the room invite.")));
                    return;
                });
        }));
}

EOnlinePresenceState::Type FOnlineSocialInterfaceEOS::ConvertPresence(EPresenceUserActivityState ActivityState)
{
    switch (ActivityState)
    {
    case EPresenceUserActivityState::Offline:
        return EOnlinePresenceState::Offline;
    case EPresenceUserActivityState::Idle:
        return EOnlinePresenceState::Away;
    case EPresenceUserActivityState::DoNotDisturb:
        return EOnlinePresenceState::DoNotDisturb;
    case EPresenceUserActivityState::Online:
        return EOnlinePresenceState::Online;
    default:
        return EOnlinePresenceState::Offline;
    }
}

TSharedRef<FOnlineUserPresence> FOnlineSocialInterfaceEOS::CreatePresence(const FPresenceUserState &Result) const
{
    auto OutPresence = MakeShared<FOnlineUserPresence>();
    if (Result.AdvertisedSession.IsSet())
    {
        OutPresence->SessionId = MakeShared<FUniqueNetIdEOSSession>(Result.AdvertisedSession->RoomId->GetRoomId());
        OutPresence->bIsJoinable = Result.AdvertisedSession->bJoinable;
    }
    else
    {
        OutPresence->SessionId = nullptr;
        OutPresence->bIsJoinable = false;
    }
    OutPresence->bIsOnline = Result.ActivityState != EPresenceUserActivityState::Offline;
    OutPresence->bIsPlaying = Result.bIsPlayingThisGame;
    OutPresence->bIsPlayingThisGame = Result.bIsPlayingThisGame;
    OutPresence->bHasVoiceSupport = false;           // @todo
    OutPresence->LastOnline = FDateTime::MinValue(); // @todo: Use IUserCache to find this info.
    OutPresence->Status.StatusStr = Result.GetStatusTextFormatted().ToString();
    OutPresence->Status.State = ConvertPresence(Result.ActivityState);
    for (const auto &Attr : Result.CustomAttributes)
    {
        switch (Attr.Value.Type())
        {
        case FRoomAttribute::EType::String:
            OutPresence->Status.Properties.Add(Attr.Key, Attr.Value.GetString());
            break;
        case FRoomAttribute::EType::Int64:
            OutPresence->Status.Properties.Add(Attr.Key, Attr.Value.GetInt64());
            break;
        case FRoomAttribute::EType::Double:
            OutPresence->Status.Properties.Add(Attr.Key, Attr.Value.GetDouble());
            break;
        case FRoomAttribute::EType::Bool:
            OutPresence->Status.Properties.Add(Attr.Key, Attr.Value.GetBool());
            break;
        default:
            break;
        }
    }
    return OutPresence;
}

void FOnlineSocialInterfaceEOS::SetPresence(
    const FUniqueNetId &User,
    const FOnlineUserPresenceStatus &Status,
    const FOnPresenceTaskCompleteDelegate &Delegate)
{
    FOnlinePresenceSetPresenceParameters Parameters;
    Parameters.State = Status.State;
    Parameters.StatusStr = Status.StatusStr;
    Parameters.Properties = Status.Properties;
    this->SetPresence(User, MoveTemp(Parameters), Delegate);
}

void FOnlineSocialInterfaceEOS::SetPresence(
    const FUniqueNetId &User,
    FOnlinePresenceSetPresenceParameters &&Parameters,
    const FOnPresenceTaskCompleteDelegate &Delegate)
{
#if WITH_EDITOR
    // @note: We need to do this so that the editor loads game translations (from things like blueprints), so that
    // FText::FindText below works.
    FTextLocalizationManager::Get().EnableGameLocalizationPreview();
#endif

    auto AccountId = GetAccountId(User);

    auto CurrentState = this->PlatformHandle->GetSystem<IPresenceSystem>()->GetPresence(AccountId, AccountId);
    auto NewState = CurrentState.IsSet() ? CurrentState.GetValue() : FPresenceUserState(AccountId);

    if (Parameters.State.IsSet())
    {
        switch (Parameters.State.GetValue())
        {
        case EOnlinePresenceState::Online:
            NewState.ActivityState = EPresenceUserActivityState::Online;
            break;
        case EOnlinePresenceState::Offline:
            NewState.ActivityState = EPresenceUserActivityState::Offline;
            break;
        case EOnlinePresenceState::Away:
            NewState.ActivityState = EPresenceUserActivityState::Idle;
            NewState.ActivityState = EPresenceUserActivityState::Idle;
            break;
        case EOnlinePresenceState::DoNotDisturb:
            NewState.ActivityState = EPresenceUserActivityState::DoNotDisturb;
            break;
        case EOnlinePresenceState::Chat:
        default:
            NewState.ActivityState = EPresenceUserActivityState::Online;
            break;
        }
    }

    if (Parameters.StatusStr.IsSet())
    {
        TArray<FString> StatusStrComponents;
        Parameters.StatusStr->ParseIntoArray(StatusStrComponents, TEXT("_"));
        if (StatusStrComponents.Num() == 2)
        {
            FText FoundText;
#if REDPOINT_EOS_UE_5_5_OR_LATER
            if (FText::FindTextInLiveTable_Advanced(
                    FTextKey(StatusStrComponents[0]),
                    FTextKey(StatusStrComponents[1]),
                    FoundText))
#else
            if (FText::FindText(FTextKey(StatusStrComponents[0]), FTextKey(StatusStrComponents[1]), FoundText))
#endif
            {
                NewState.StatusTextFormat = FTextFormat(FoundText);
            }
            else
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Warning,
                    TEXT("The 'StatusStr' value passed to SetPresence should be in the format 'TextNamespace_TextKey', "
                         "where the namespace and key refer to a localizable FText value (i.e. it should be localized "
                         "via the Localization Dashboard in the editor). There was no localizable FText value with the "
                         "namespace '%s' and key '%s' found at runtime. To ensure your game is compatible with "
                         "platforms that require localizable presence text, the Redpoint EOS plugin no longer permits "
                         "you to set arbitrary text strings for presence status."),
                    *StatusStrComponents[0],
                    *StatusStrComponents[1]);
                NewState.StatusTextFormat = FTextFormat(FText::GetEmpty());
            }
        }
        else
        {
            UE_LOG(
                LogOnlineSubsystemRedpointEOS,
                Warning,
                TEXT(
                    "The 'StatusStr' value passed to SetPresence should be in the format 'TextNamespace_TextKey', "
                    "where the namespace and key refer to a localizable FText value (i.e. it should be localized "
                    "via the Localization Dashboard in the editor). The provided string '%s' did not match that "
                    "format. To ensure your game is compatible with platforms that require localizable presence text, "
                    "the Redpoint EOS plugin no longer permits you to set arbitrary text strings for presence status."),
                **Parameters.StatusStr);
        }
    }

    if (Parameters.Properties.IsSet())
    {
        for (const auto &KV : Parameters.Properties.GetValue())
        {
            switch (KV.Value.GetType())
            {
            case EOnlineKeyValuePairDataType::Int64: {
                int64 Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(Value));
                break;
            }
            case EOnlineKeyValuePairDataType::Double: {
                double Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(Value));
                break;
            }
            case EOnlineKeyValuePairDataType::String: {
                FString Value;
                KV.Value.GetValue(Value);
                {
                    TArray<FString> ParameterComponents;
                    Value.ParseIntoArray(ParameterComponents, TEXT("_"));
                    if (ParameterComponents.Num() == 2)
                    {
                        FText FoundText;
#if REDPOINT_EOS_UE_5_5_OR_LATER
                        if (FText::FindTextInLiveTable_Advanced(
                                FTextKey(ParameterComponents[0]),
                                FTextKey(ParameterComponents[1]),
                                FoundText))
#else
                        if (FText::FindText(
                                FTextKey(ParameterComponents[0]),
                                FTextKey(ParameterComponents[1]),
                                FoundText))
#endif
                        {
                            NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(FoundText));
                        }
                    }
                }
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(Value));
                break;
            }
            case EOnlineKeyValuePairDataType::Bool: {
                bool Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(Value));
                break;
            }
            case EOnlineKeyValuePairDataType::Int32: {
                int32 Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(static_cast<int64>(Value)));
                break;
            }
            case EOnlineKeyValuePairDataType::UInt32: {
                uint32 Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(static_cast<int64>(Value)));
                break;
            }
            case EOnlineKeyValuePairDataType::UInt64: {
                uint64 Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                break;
            }
            case EOnlineKeyValuePairDataType::Float: {
                float Value;
                KV.Value.GetValue(Value);
                NewState.StatusTextFormatArguments.Add(KV.Key, FFormatArgumentValue(Value));
                NewState.CustomAttributes.Add(KV.Key, FRoomAttribute(static_cast<double>(Value)));
                break;
            }
            case EOnlineKeyValuePairDataType::Empty:
            case EOnlineKeyValuePairDataType::Blob:
            case EOnlineKeyValuePairDataType::Json:
            default:
                // not supported.
                break;
            }
        }
    }

    this->PlatformHandle->GetSystem<IPresenceSystem>()->SetPresence(
        AccountId,
        NewState,
        IPresenceSystem::FOnSetPresenceComplete::CreateSPLambda(this, [Delegate, AccountId](const FError &ErrorCode) {
            if (!ErrorCode.WasSuccessful())
            {
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Error,
                    TEXT("%s: %s"),
                    ANSI_TO_TCHAR(__FUNCTION__),
                    *ErrorCode.ToLogString());
            }

            auto UserId = GetUniqueNetId(AccountId);
            if (UserId.IsValid())
            {
                Delegate.ExecuteIfBound(*UserId, ErrorCode.WasSuccessful());
            }
        }));
}

void FOnlineSocialInterfaceEOS::QueryPresence(const FUniqueNetId &User, const FOnPresenceTaskCompleteDelegate &Delegate)
{
    // We no longer need to do any query here, because the friends system keeps presence of friends up-to-date.
    Delegate.ExecuteIfBound(User, true);
    return;
}

void FOnlineSocialInterfaceEOS::QueryPresence(
    const FUniqueNetId &LocalUserId,
    const TArray<FUniqueNetIdRef> &UserIds,
    const FOnPresenceTaskCompleteDelegate &Delegate)
{
    // @note: Do we need to support querying presence for arbitrary user IDs, or can we assume that all presence queries
    // are just going to be for friends anyway (which is how things work on most platforms - you can't query arbitrary
    // presence).
    Delegate.ExecuteIfBound(LocalUserId, true);
    return;
}

EOnlineCachedResult::Type FOnlineSocialInterfaceEOS::GetCachedPresence(
    const FUniqueNetId &User,
    TSharedPtr<FOnlineUserPresence> &OutPresence)
{
    // @note: The API signature of GetCachedPresence is terrible because it doesn't have callers specify the local user
    // ID, so we just have to try... all the local users.
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    for (const auto &LocalUser : IdentitySystem->GetUsers())
    {
        auto Friend = this->GetFriend(LocalUser->GetUserSlot(), User, TEXT(""));
        if (Friend.IsValid())
        {
            OutPresence = MakeShared<FOnlineUserPresence>(Friend->GetPresence());
            return EOnlineCachedResult::Success;
        }
    }
    return EOnlineCachedResult::NotFound;
}

EOnlineCachedResult::Type FOnlineSocialInterfaceEOS::GetCachedPresenceForApp(
    const FUniqueNetId &LocalUserId,
    const FUniqueNetId &User,
    const FString &AppId,
    TSharedPtr<FOnlineUserPresence> &OutPresence)
{
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    for (const auto &LocalUser : IdentitySystem->GetUsers())
    {
        if (LocalUser->GetUserId() != GetAccountId(LocalUserId))
        {
            continue;
        }
        auto Friend = this->GetFriend(LocalUser->GetUserSlot(), User, TEXT(""));
        if (Friend.IsValid())
        {
            OutPresence = MakeShared<FOnlineUserPresence>(Friend->GetPresence());
            return EOnlineCachedResult::Success;
        }
    }
    return EOnlineCachedResult::NotFound;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()