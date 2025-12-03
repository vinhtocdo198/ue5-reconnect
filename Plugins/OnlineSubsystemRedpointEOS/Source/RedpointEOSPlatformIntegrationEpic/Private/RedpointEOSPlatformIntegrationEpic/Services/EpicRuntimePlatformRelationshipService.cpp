// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationEpic/Services/EpicRuntimePlatformRelationshipService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Friends/GetBlockedUsers.h"
#include "RedpointEOSAPI/Friends/GetFriends.h"
#include "RedpointEOSAPI/Friends/GetStatus.h"
#include "RedpointEOSAPI/Friends/NotifyBlockedUsersUpdate.h"
#include "RedpointEOSAPI/Friends/NotifyFriendsUpdate.h"
#include "RedpointEOSAPI/Friends/QueryFriends.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountId.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatformIntegrationEpic/EpicExternalUserInfo.h"
#include "RedpointEOSUserCache/ExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1578170046, Redpoint::EOS::Platform::Integration::Epic::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Friends;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Platform::Integration::Epic;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::UserCache;

FEpicRuntimePlatformRelationshipService::FEpicRuntimePlatformRelationshipService()
    : PlatformStorage()
    , UserStorage()
{
}

FString FEpicRuntimePlatformRelationshipService::GetCacheKey() const
{
    // For compatibility with old "RedpointEAS" OSSv1 subsystem.
    return TEXT("RedpointEAS");
}

bool FEpicRuntimePlatformRelationshipService::AllowRelationshipCaching() const
{
    return true;
}

void FEpicRuntimePlatformRelationshipService::GetRelationships(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    ERelationshipType RelationshipTypes,
    const FOnGetRelationshipComplete &OnComplete)
{
    TMap<ERelationshipType, TArray<FExternalUserInfoRef>> Results;

    auto *UserStore = this->UserStorage.Find(CallContext->GetUserKey());
    if (UserStore == nullptr || !UserStore->EpicGamesAccountId.IsSet())
    {
        // This isn't an Epic Games user.
        OnComplete.ExecuteIfBound(Errors::Success(), Results);
        return;
    }

    auto EpicAccountId = GetEpicGamesAccountId(*UserStore->EpicGamesAccountId);

    TArray<EOS_EpicAccountId> Friends;
    TArray<EOS_EpicAccountId> BlockedUsers;
    EOS_EResult FriendsResultCode;
    EOS_EResult BlockedUsersResultCode;

    FGetFriends::Execute(CallContext->PlatformHandle, FGetFriends::Options{EpicAccountId}, FriendsResultCode, Friends);
    FGetBlockedUsers::Execute(
        CallContext->PlatformHandle,
        FGetBlockedUsers::Options{EpicAccountId},
        BlockedUsersResultCode,
        BlockedUsers);

    if (FriendsResultCode == EOS_EResult::EOS_Success)
    {
        for (const auto &Friend : Friends)
        {
            if (FGetStatus::Execute(CallContext->PlatformHandle, FGetStatus::Options{EpicAccountId, Friend}) ==
                EOS_EFriendsStatus::EOS_FS_Friends)
            {
                auto FriendString = GetEpicGamesUserIdString(Friend);
                auto *ExternalUser = UserStore->EpicExternalUserInfoCache.Find(FriendString);
                if (ExternalUser == nullptr)
                {
                    ExternalUser = &UserStore->EpicExternalUserInfoCache.Add(
                        FriendString,
                        MakeShared<FEpicExternalUserInfo>(Friend));
                }
                Results.FindOrAdd(ERelationshipType::MutualFriends).Add(*ExternalUser);
            }
        }
    }
    if (BlockedUsersResultCode == EOS_EResult::EOS_Success)
    {
        for (const auto &BlockedUser : BlockedUsers)
        {
            auto BlockedUserString = GetEpicGamesUserIdString(BlockedUser);
            auto *ExternalUser = UserStore->EpicExternalUserInfoCache.Find(BlockedUserString);
            if (ExternalUser == nullptr)
            {
                ExternalUser = &UserStore->EpicExternalUserInfoCache.Add(
                    BlockedUserString,
                    MakeShared<FEpicExternalUserInfo>(BlockedUser));
            }
            Results.FindOrAdd(ERelationshipType::Blocked).Add(*ExternalUser);
        }
    }

    OnComplete.ExecuteIfBound(Errors::Success(), Results);
}

IRuntimePlatformRelationshipService::FOnRelationshipsChanged &FEpicRuntimePlatformRelationshipService::
    OnRelationshipsChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    // @note: We still return a delegate even for non-Epic Games accounts; it'll just never be fired. The delegate will
    // also be reset every logout as UserStorage entries are removed in UnregisterEvents.
    auto &UserStore = this->UserStorage.FindOrAdd(CallContext->GetUserKey(), FUserStorage());
    return UserStore.OnRelationshipsChangedDelegate;
}

void FEpicRuntimePlatformRelationshipService::RegisterEvents(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(CallContext->LocalUserNum);
    if (!IdentityUser.IsValid() || !IdentityUser->GetCrossPlatformAccountId().IsValid() ||
        !IdentityUser->GetCrossPlatformAccountId()->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID))
    {
        // This isn't an Epic Games user.
        OnComplete.ExecuteIfBound();
        return;
    }

    auto EpicAccountId =
        StaticCastSharedPtr<const FEpicGamesCrossPlatformAccountId>(IdentityUser->GetCrossPlatformAccountId())
            ->GetEpicAccountId();
    FQueryFriends::Execute(
        CallContext->PlatformHandle,
        FQueryFriends::Options{EpicAccountId},
        FQueryFriends::CompletionDelegate::CreateSPLambda(
            this,
            [this, CallContext, EpicAccountId, OnComplete](const FQueryFriends::Result &Result) {
                auto &UserStore = this->UserStorage.FindOrAdd(CallContext->GetUserKey(), FUserStorage());
                UserStore.EpicGamesAccountId = GetEpicGamesUserIdString(EpicAccountId);

                auto &PlatformStore =
                    this->PlatformStorage.FindOrAdd(CallContext->GetPlatformKey(), FPlatformStorage());
                PlatformStore.LocalUsersSignedIn.Add(CallContext->LocalUserNum);
                if (!PlatformStore.OnBlockedUsersUpdate.IsValid())
                {
                    PlatformStore.OnBlockedUsersUpdate = FNotifyBlockedUsersUpdate::Register(
                        CallContext->PlatformHandle,
                        FNotifyBlockedUsersUpdate::Options{},
                        FNotifyBlockedUsersUpdate::EventDelegate::CreateSPLambda(
                            this,
                            [this,
                             PlatformHandle = CallContext->PlatformHandle,
                             LocalUserNum =
                                 CallContext->LocalUserNum](const FNotifyBlockedUsersUpdate::Result &EventResult) {
                                auto LocalUserIdString = GetEpicGamesUserIdString(EventResult.LocalUserId);
                                auto TargetUserIdString = GetEpicGamesUserIdString(EventResult.TargetUserId);
                                for (auto &KV : this->UserStorage)
                                {
                                    if (KV.Key.Get<0>() == PlatformHandle && KV.Key.Get<1>() == LocalUserNum &&
                                        KV.Value.EpicGamesAccountId == LocalUserIdString)
                                    {
                                        auto *ExternalUser =
                                            KV.Value.EpicExternalUserInfoCache.Find(TargetUserIdString);
                                        if (ExternalUser == nullptr)
                                        {
                                            ExternalUser = &KV.Value.EpicExternalUserInfoCache.Add(
                                                TargetUserIdString,
                                                MakeShared<FEpicExternalUserInfo>(EventResult.TargetUserId));
                                        }

                                        TMap<ERelationshipType, TArray<FExternalUserInfoRef>> Added;
                                        TMap<ERelationshipType, TArray<FExternalUserInfoRef>> Removed;
                                        if (EventResult.bBlocked)
                                        {
                                            Added.FindOrAdd(ERelationshipType::Blocked).Add(*ExternalUser);
                                        }
                                        else
                                        {
                                            Removed.FindOrAdd(ERelationshipType::Blocked).Add(*ExternalUser);
                                        }

                                        KV.Value.OnRelationshipsChangedDelegate.Broadcast(Added, Removed);
                                    }
                                }
                            }));
                }
                if (!PlatformStore.OnFriendsUpdate.IsValid())
                {
                    PlatformStore.OnFriendsUpdate = FNotifyFriendsUpdate::Register(
                        CallContext->PlatformHandle,
                        FNotifyFriendsUpdate::Options{},
                        FNotifyFriendsUpdate::EventDelegate::CreateSPLambda(
                            this,
                            [this,
                             PlatformHandle = CallContext->PlatformHandle,
                             LocalUserNum =
                                 CallContext->LocalUserNum](const FNotifyFriendsUpdate::Result &EventResult) {
                                auto LocalUserIdString = GetEpicGamesUserIdString(EventResult.LocalUserId);
                                auto TargetUserIdString = GetEpicGamesUserIdString(EventResult.TargetUserId);
                                for (auto &KV : this->UserStorage)
                                {
                                    if (KV.Key.Get<0>() == PlatformHandle && KV.Key.Get<1>() == LocalUserNum &&
                                        KV.Value.EpicGamesAccountId == LocalUserIdString)
                                    {
                                        auto *ExternalUser =
                                            KV.Value.EpicExternalUserInfoCache.Find(TargetUserIdString);
                                        if (ExternalUser == nullptr)
                                        {
                                            ExternalUser = &KV.Value.EpicExternalUserInfoCache.Add(
                                                TargetUserIdString,
                                                MakeShared<FEpicExternalUserInfo>(EventResult.TargetUserId));
                                        }

                                        TMap<ERelationshipType, TArray<FExternalUserInfoRef>> Added;
                                        TMap<ERelationshipType, TArray<FExternalUserInfoRef>> Removed;
                                        bool bChanged = false;
                                        if (EventResult.PreviousStatus == EOS_EFriendsStatus::EOS_FS_NotFriends &&
                                            EventResult.CurrentStatus == EOS_EFriendsStatus::EOS_FS_Friends)
                                        {
                                            Added.FindOrAdd(ERelationshipType::MutualFriends).Add(*ExternalUser);
                                            bChanged = true;
                                        }
                                        else if (
                                            EventResult.PreviousStatus == EOS_EFriendsStatus::EOS_FS_Friends &&
                                            EventResult.CurrentStatus == EOS_EFriendsStatus::EOS_FS_NotFriends)
                                        {
                                            Removed.FindOrAdd(ERelationshipType::MutualFriends).Add(*ExternalUser);
                                            bChanged = true;
                                        }

                                        if (bChanged)
                                        {
                                            KV.Value.OnRelationshipsChangedDelegate.Broadcast(Added, Removed);
                                        }
                                    }
                                }
                            }));
                }

                OnComplete.ExecuteIfBound();
            }));
}

void FEpicRuntimePlatformRelationshipService::UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    auto *UserStore = this->UserStorage.Find(CallContext->GetUserKey());
    if (UserStore != nullptr)
    {
        this->UserStorage.Remove(CallContext->GetUserKey());
    }

    auto *PlatformStore = this->PlatformStorage.Find(CallContext->GetPlatformKey());
    if (PlatformStore != nullptr)
    {
        PlatformStore->LocalUsersSignedIn.Remove(CallContext->LocalUserNum);
        if (PlatformStore->LocalUsersSignedIn.Num() == 0)
        {
            this->PlatformStorage.Remove(CallContext->GetPlatformKey());
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()