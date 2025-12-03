// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineFriendsInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineFriendNonUnifiedEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineFriendUnifiedEOS.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4053231989, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::UserCache;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineSocialInterfaceEOS : public IOnlineFriends,
                                                                 public IOnlinePresence,
                                                                 public IHasEventRegistration,
                                                                 public TSharedFromThis<FOnlineSocialInterfaceEOS>
{
private:
    FPlatformHandle PlatformHandle;
    struct FLocalUserState
    {
        TUserIdMap<TSharedRef<FOnlineFriendUnifiedEOS>> Unified;
        TUserIdMap<TSharedRef<FOnlineFriendNonUnifiedEOS>> NonUnified;
        TUserIdMap<TSharedRef<FOnlineFriend>> Synthetic;
        TMap<FExternalUserInfoRef, FUniqueNetIdRef> ExternalUsersToIds;

        TMap<FName, FDelegateHandle> OnSyntheticPresenceReceived;
        TMap<FName, FDelegateHandle> OnSyntheticPresenceArrayUpdated;
    };
    TUserIdMap<FLocalUserState> LocalUserStates;

public:
    FOnlineSocialInterfaceEOS(const FPlatformHandle &InPlatformHandle);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FOnlineSocialInterfaceEOS);
    virtual ~FOnlineSocialInterfaceEOS() override = default;

    // Friends
private:
    void ResyncLocalUserStates();
    void ResyncLocalUserState(const FUniqueNetIdRef &LocalUniqueNetId, bool bFireEvents);
    static TOptional<TTuple<EOS_EExternalAccountType, FString>> GetPrimaryExternalUserInfoId(
        const FExternalUserInfoRef &ExternalUserInfo);
    TSharedPtr<FOnlineFriend> TryGetSyntheticFriend(int32 LocalUserNum, const FExternalUserInfoRef &ExternalUserInfo);

public:
    virtual bool ReadFriendsList(
        int32 LocalUserNum,
        const FString &ListName,
        const FOnReadFriendsListComplete &Delegate) override;
    virtual bool DeleteFriendsList(
        int32 LocalUserNum,
        const FString &ListName,
        const FOnDeleteFriendsListComplete &Delegate) override;

    virtual bool SendInvite(
        int32 LocalUserNum,
        const FUniqueNetId &FriendId,
        const FString &ListName,
        const FOnSendInviteComplete &Delegate) override;
    virtual bool AcceptInvite(
        int32 LocalUserNum,
        const FUniqueNetId &FriendId,
        const FString &ListName,
        const FOnAcceptInviteComplete &Delegate) override;
    virtual bool RejectInvite(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName) override;

    virtual void SetFriendAlias(
        int32 LocalUserNum,
        const FUniqueNetId &FriendId,
        const FString &ListName,
        const FString &Alias,
        const FOnSetFriendAliasComplete &Delegate) override;
    virtual void DeleteFriendAlias(
        int32 LocalUserNum,
        const FUniqueNetId &FriendId,
        const FString &ListName,
        const FOnDeleteFriendAliasComplete &Delegate) override;

    virtual bool DeleteFriend(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName) override;

    virtual bool GetFriendsList(
        int32 LocalUserNum,
        const FString &ListName,
        TArray<TSharedRef<FOnlineFriend>> &OutFriends) override;
    virtual TSharedPtr<FOnlineFriend> GetFriend(
        int32 LocalUserNum,
        const FUniqueNetId &FriendId,
        const FString &ListName) override;
    virtual bool IsFriend(int32 LocalUserNum, const FUniqueNetId &FriendId, const FString &ListName) override;

    virtual void AddRecentPlayers(
        const FUniqueNetId &UserId,
        const TArray<FReportPlayedWithUser> &InRecentPlayers,
        const FString &ListName,
        const FOnAddRecentPlayersComplete &InCompletionDelegate) override;
    virtual bool QueryRecentPlayers(const FUniqueNetId &UserId, const FString &Namespace) override;
    virtual bool GetRecentPlayers(
        const FUniqueNetId &UserId,
        const FString &Namespace,
        TArray<TSharedRef<FOnlineRecentPlayer>> &OutRecentPlayers) override;
    virtual void DumpRecentPlayers() const override;

    virtual bool BlockPlayer(int32 LocalUserNum, const FUniqueNetId &PlayerId) override;
    virtual bool UnblockPlayer(int32 LocalUserNum, const FUniqueNetId &PlayerId) override;
    virtual bool QueryBlockedPlayers(const FUniqueNetId &UserId) override;
    virtual bool GetBlockedPlayers(
        const FUniqueNetId &UserId,
        TArray<TSharedRef<FOnlineBlockedPlayer>> &OutBlockedPlayers) override;
    virtual void DumpBlockedPlayers() const override;

    virtual void QueryFriendSettings(const FUniqueNetId &LocalUserId, FOnSettingsOperationComplete Delegate) override;
    virtual void UpdateFriendInvitePolicySettings(
        const FUniqueNetId &LocalUserId,
        const EFriendInvitePolicy FriendInvitePolicy,
        const bool bAffectsExistingInvites,
        FOnSettingsOperationComplete Delegate) override;
    virtual bool QueryFriendSettings(
        const FUniqueNetId &UserId,
        const FString &Source,
        const FOnQueryFriendSettingsComplete &Delegate) override;
    virtual bool GetFriendSettings(
        const FUniqueNetId &UserId,
        TMap<FString, TSharedRef<FOnlineFriendSettingsSourceData>> &OutSettings) override;
    virtual EFriendInvitePolicy GetFriendInvitePolicy(const FUniqueNetId &UserId) const override;
    virtual bool SetFriendSettings(
        const FUniqueNetId &UserId,
        const FString &Source,
        bool bNeverShowAgain,
        const FOnSetFriendSettingsComplete &Delegate) override;

    typedef TDelegate<void(const FError &ErrorCode)> FOnSendRoomInviteToPotentialFriendComplete;
    void SendRoomInviteToPotentialFriend(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetIdRef &TargetFriendId,
        const FRoomRef &TargetRoom,
        const FOnSendRoomInviteToPotentialFriendComplete &OnComplete);

    // Presence
private:
    static EOnlinePresenceState::Type ConvertPresence(EPresenceUserActivityState ActivityState);
    TSharedRef<FOnlineUserPresence> CreatePresence(const FPresenceUserState &Result) const;

public:
    virtual void SetPresence(
        const FUniqueNetId &User,
        const FOnlineUserPresenceStatus &Status,
        const FOnPresenceTaskCompleteDelegate &Delegate) override;

    virtual void SetPresence(
        const FUniqueNetId &User,
        FOnlinePresenceSetPresenceParameters &&Parameters,
        const FOnPresenceTaskCompleteDelegate &Delegate) override;

    virtual void QueryPresence(const FUniqueNetId &User, const FOnPresenceTaskCompleteDelegate &Delegate) override;

    virtual void QueryPresence(
        const FUniqueNetId &LocalUserId,
        const TArray<FUniqueNetIdRef> &UserIds,
        const FOnPresenceTaskCompleteDelegate &Delegate) override;

    virtual EOnlineCachedResult::Type GetCachedPresence(
        const FUniqueNetId &User,
        TSharedPtr<FOnlineUserPresence> &OutPresence) override;

    virtual EOnlineCachedResult::Type GetCachedPresenceForApp(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &User,
        const FString &AppId,
        TSharedPtr<FOnlineUserPresence> &OutPresence) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social
{
REDPOINT_EOS_FILE_NS_EXPORT(
    4053231989,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social,
    FOnlineSocialInterfaceEOS);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()