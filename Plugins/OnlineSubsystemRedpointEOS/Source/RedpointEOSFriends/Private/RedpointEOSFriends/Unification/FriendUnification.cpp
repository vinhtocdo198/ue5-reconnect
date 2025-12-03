// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSFriends/Unification/FriendUnification.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/AsyncTaskGraph.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSCore/Utils/UnpackMap.h"
#include "RedpointEOSFriends/Database/SerializedCachedFriend.h"
#include "RedpointEOSFriends/Logging.h"
#include "RedpointEOSFriends/Unification/CachedExternalUserInfo.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformRelationshipService.h"
#include "RedpointEOSPlatform/Types/RelationshipType.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3009419599, Redpoint::EOS::Friends::Unification)
{
using namespace ::Redpoint::EOS::API::Common;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Friends;
using namespace ::Redpoint::EOS::Friends::Database;
using namespace ::Redpoint::EOS::Friends::Unification;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Platform::Types;
using namespace ::Redpoint::EOS::UserCache;

void FFriendUnification::ReadUnifiedFriends(
    const FPlatformHandle &PlatformHandle,
    FIdentityUserId LocalUserId,
    const TSharedRef<FLocalUserFriendState> &LocalUserFriendState,
    const TArray<TSharedRef<IRuntimePlatformIntegration>> &RuntimePlatformIntegrations,
    const FOnReadUnifiedFriendsComplete &OnComplete)
{
    auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
    auto LocalUser = IdentitySystem->GetUser(LocalUserId);
    if (!LocalUser.IsValid())
    {
        OnComplete.ExecuteIfBound(
            Errors::InvalidUser(ANSI_TO_TCHAR(__FUNCTION__), TEXT("Local user is not signed in.")),
            TAccountIdMap<FUnifiedUserInfoPtr>(),
            TArray<FExternalUserInfoRef>());
        return;
    }

    auto Executor = MakeShared<FAsyncTaskGraph>();
    auto CallContext = ConstructRuntimePlatformServiceCallContext(PlatformHandle, LocalUser->GetUserSlot());
    auto UnificationState = MakeShared<FFriendUnificationState>();

    UE_LOG(LogRedpointEOSFriends, Verbose, TEXT("%s: Reading unified friends..."), *GetUserIdString(LocalUserId));

    // Read friends from all runtime integrations, and all avatars of friends via runtime integrations.
    auto PreviousTask = Executor->Add(
        [CallContext,
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
         LocalUserId,
#endif
         LocalUserFriendState,
         RuntimePlatformIntegrations,
         UnificationState](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            using FIntegrationOp = FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, bool>;
            FIntegrationOp::Run(
                RuntimePlatformIntegrations,
                FIntegrationOp::FInitiate::CreateLambda([CallContext,
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                                                         LocalUserId,
#endif
                                                         LocalUserFriendState,
                                                         UnificationState](
                                                            TSharedRef<IRuntimePlatformIntegration> Integration,
                                                            FIntegrationOp::FOnEntryDone OnIntegrationDone) {
                    auto CacheKey = Integration->GetRelationshipService()->GetCacheKey();
                    auto AllowRelationshipCaching = Integration->GetRelationshipService()->AllowRelationshipCaching();
                    UE_LOG(
                        LogRedpointEOSFriends,
                        Verbose,
                        TEXT("%s: Reading friends from runtime integration with cache key '%s'..."),
                        *GetUserIdString(LocalUserId),
                        *CacheKey);
                    Integration->GetRelationshipService()->GetRelationships(
                        CallContext,
                        ERelationshipType::MutualFriends,
                        IRuntimePlatformRelationshipService::FOnGetRelationshipComplete::CreateLambda(
                            [UnificationState,
                             CallContext,
                             CacheKey,
                             AllowRelationshipCaching,
#if REDPOINT_EOS_INCLUDE_UE_LOG_ONLY_CAPTURES
                             LocalUserId,
#endif
                             LocalUserFriendState,
                             Integration,
                             OnIntegrationDone](
                                const FError &IntegrationError,
                                const TMap<ERelationshipType, TArray<FExternalUserInfoRef>> &RelationshipsByType) {
                                if (!IntegrationError.WasSuccessful())
                                {
                                    if (!GIsAutomationTesting)
                                    {
                                        UE_LOG(
                                            LogRedpointEOSFriends,
                                            Warning,
                                            TEXT("Unable to retrieve friends from runtime integration: %s"),
                                            *IntegrationError.ToLogString());
                                    }
                                    OnIntegrationDone.ExecuteIfBound(true);
                                    return;
                                }

                                // If the runtime integration was successful, we must ensure that
                                // ExternalFriendsWithAvatarsByCacheKey is set, even if it had no friends to return.
                                // This ensures that the a stale friend list will not be loaded from the cache when the
                                // runtime integration reports that the user genuinely no longer has any friends on
                                // their friends list.
                                auto &ExternalFriendsForCacheClear =
                                    UnificationState->ExternalFriendsWithAvatarsByCacheKey.FindOrAdd(CacheKey);

                                // @note: We must empty the external friends by cache key here, otherwise
                                // friends that are deleted on the local platform are never removed from the
                                // array when we have an updated friends list.
                                ExternalFriendsForCacheClear.Empty();

                                auto *MutualFriends = RelationshipsByType.Find(ERelationshipType::MutualFriends);
                                if (MutualFriends == nullptr)
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Verbose,
                                        TEXT("%s: Runtime integration with cache key '%s' had no mutual friends."),
                                        *GetUserIdString(LocalUserId),
                                        *CacheKey);
                                    OnIntegrationDone.ExecuteIfBound(true);
                                    return;
                                }

                                UE_LOG(
                                    LogRedpointEOSFriends,
                                    Verbose,
                                    TEXT("%s: Runtime integration with cache key '%s' provided %d mutual friends."),
                                    *GetUserIdString(LocalUserId),
                                    *CacheKey,
                                    MutualFriends->Num());

                                // Load all of the avatars.
                                using FAvatarOp = FMultiOperation<FExternalUserInfoRef, TOptional<FString>>;
                                FAvatarOp::Run(
                                    *MutualFriends,
                                    FAvatarOp::FInitiate::CreateLambda([Integration, CallContext](
                                                                           FExternalUserInfoRef ExternalUserInfo,
                                                                           FAvatarOp::FOnEntryDone OnAvatarDone) {
                                        Integration->GetAvatarService()->GetAvatarUrl(
                                            CallContext,
                                            ExternalUserInfo,
                                            IRuntimePlatformAvatarService::FOnGetAvatarUrlComplete::CreateLambda(
                                                [OnAvatarDone](
                                                    const FExternalUserInfoRef &UserInfo,
                                                    const FError &AvatarError,
                                                    const FString &AvatarUrl) {
                                                    if (AvatarError.WasSuccessful() && !AvatarUrl.IsEmpty())
                                                    {
                                                        OnAvatarDone.ExecuteIfBound(AvatarUrl);
                                                    }
                                                    else
                                                    {
                                                        OnAvatarDone.ExecuteIfBound(TOptional<FString>());
                                                    }
                                                }));
                                        return true;
                                    }),
                                    FAvatarOp::FOnDone::CreateLambda([OnIntegrationDone,
                                                                      LocalUserFriendState,
                                                                      MutualFriendsValue = *MutualFriends,
                                                                      UnificationState,
                                                                      CacheKey,
                                                                      AllowRelationshipCaching](
                                                                         const TArray<TOptional<FString>> &AvatarUrls) {
                                        // Iterate through the results, adding entries to both the
                                        // ExternalFriendsWithAvatarsByCacheKey that we'll return, and updating the
                                        // serialized cached friends in the user's cross-platform friends database.
                                        auto &ExternalFriends =
                                            UnificationState->ExternalFriendsWithAvatarsByCacheKey.FindOrAdd(CacheKey);

                                        TArray<FSerializedCachedFriend> SerializedCachedFriends;
                                        for (int i = 0; i < MutualFriendsValue.Num(); i++)
                                        {
                                            const auto &MutualFriend = MutualFriendsValue[i];
                                            const auto &AvatarUrl = AvatarUrls[i];

                                            // Add the result to the list of external users by cache key.
                                            ExternalFriends.Add(FExternalUserInfoAndAvatarUrl{MutualFriend, AvatarUrl});

                                            // Add it to the user's serialized cached friends.
                                            if (AllowRelationshipCaching)
                                            {
                                                FSerializedCachedFriend SerializedCachedFriend;
                                                for (const auto &LinkedAccount :
                                                     MutualFriend->GetLinkedExternalAccounts())
                                                {
                                                    FSerializedCachedFriend::FSerializedCachedFriendLinkedAccount
                                                        SerializedLinkedAccount;
                                                    SerializedLinkedAccount.ExternalAccountType =
                                                        LinkedAccount.Value.AccountIdType;
                                                    SerializedLinkedAccount.DisplayName =
                                                        LinkedAccount.Value.DisplayName;
                                                    SerializedLinkedAccount.AccountId = LinkedAccount.Value.AccountId;
                                                    SerializedLinkedAccount.LastLoginTime =
                                                        LinkedAccount.Value.LastLoginTime;
                                                    SerializedCachedFriend.LinkedAccounts.Add(
                                                        LinkedAccount.Key,
                                                        SerializedLinkedAccount);
                                                }
                                                SerializedCachedFriend.Attributes = MutualFriend->GetAttributes();
                                                if (AvatarUrl.IsSet())
                                                {
                                                    SerializedCachedFriend.AccountAvatarUrl = AvatarUrl.GetValue();
                                                    SerializedCachedFriend.AccountAvatarUrlLastFetched =
                                                        FDateTime::UtcNow();
                                                }
                                                SerializedCachedFriends.Add(SerializedCachedFriend);
                                            }
                                        }
                                        if (AllowRelationshipCaching)
                                        {
                                            LocalUserFriendState->FriendDatabase->RuntimeIntegrationCachedFriends
                                                .FindOrAdd(CacheKey) = SerializedCachedFriends;
                                        }
                                        else
                                        {
                                            // Actively clear the cache if we aren't permitted to perform relationship
                                            // caching, in case permission changes over time.
                                            LocalUserFriendState->FriendDatabase->RuntimeIntegrationCachedFriends
                                                .FindOrAdd(CacheKey)
                                                .Empty();
                                        }

                                        if (!LocalUserFriendState->bHasCacheUpdateEverDirtiedDatabase)
                                        {
                                            // We only permit friend unification to ever dirty the database once, while
                                            // friend unification is happening initially during login. We don't want to
                                            // dirty the database on subsequent calls, as friend unification happens as
                                            // part of ImmediateRefresh which can happen as part of ImmediateSave (which
                                            // results in an unending save loop if friend unification always dirties).
                                            LocalUserFriendState->bDirty = true;
                                            LocalUserFriendState->bHasCacheUpdateEverDirtiedDatabase = true;
                                        }

                                        OnIntegrationDone.ExecuteIfBound(true);
                                    }));
                            }));
                    return true;
                }),
                FIntegrationOp::FOnDone::CreateLambda([OnTaskDone](const TArray<bool> &) {
                    OnTaskDone.ExecuteIfBound(Errors::Success());
                }));
        },
        FAsyncTaskGraph::ENoDependencies::StartImmediately);

    // Add all of the external friends previously cached in the cross-platform friends database.
    PreviousTask = Executor->Add(
        [LocalUserFriendState, UnificationState](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            for (const auto &KV : LocalUserFriendState->FriendDatabase->RuntimeIntegrationCachedFriends)
            {
                // If we were able to populate this cache key from the runtime integration, do not use the cache from
                // the cross-platform friends database.
                if (UnificationState->ExternalFriendsWithAvatarsByCacheKey.Contains(KV.Key))
                {
                    continue;
                }

                UnificationState->CacheKeysLoadedFromCache.Add(KV.Key);
                auto &ExternalFriends = UnificationState->ExternalFriendsWithAvatarsByCacheKey.FindOrAdd(KV.Key);
                for (const auto &CachedExternalFriendInfo : KV.Value)
                {
                    auto CachedExternalUserInfo = MakeShared<FCachedExternalUserInfo>(CachedExternalFriendInfo);
                    UnificationState->ExternalFriendsLoadedFromCache.Add(CachedExternalUserInfo);
                    ExternalFriends.Add(
                        FExternalUserInfoAndAvatarUrl{CachedExternalUserInfo, CachedExternalUserInfo->GetAvatarUrl()});
                }
            }
            OnTaskDone.ExecuteIfBound(Errors::Success());
        },
        PreviousTask);

    // For all the external friends we fetched, see if there is an associated product user ID.
    PreviousTask = Executor->Add(
        [PlatformHandle, LocalUserId, UnificationState](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            // Bucket the external friends by external account types.
            TMap<EOS_EExternalAccountType, TArray<FString>> ExternalAccountIdsByType;
            TMap<FExternalAccountTypeAndId, FString> AssociatedAvatars;
            for (const auto &CacheKeyToExternalUserInfosWithAvatarUrls :
                 UnificationState->ExternalFriendsWithAvatarsByCacheKey)
            {
                for (const auto &ExternalUserInfosWithAvatarUrl : CacheKeyToExternalUserInfosWithAvatarUrls.Value)
                {
                    for (const auto &LinkedExternalAccount :
                         ExternalUserInfosWithAvatarUrl.ExternalUserInfo->GetLinkedExternalAccounts())
                    {
                        ExternalAccountIdsByType.FindOrAdd(LinkedExternalAccount.Key)
                            .AddUnique(LinkedExternalAccount.Value.AccountId);
                        auto LinkedAccountKey =
                            FExternalAccountTypeAndId(LinkedExternalAccount.Key, LinkedExternalAccount.Value.AccountId);
                        if (ExternalUserInfosWithAvatarUrl.AvatarUrl.IsSet())
                        {
                            AssociatedAvatars.Add(
                                LinkedAccountKey,
                                ExternalUserInfosWithAvatarUrl.AvatarUrl.GetValue());
                        }
                        if (!UnificationState->ExternalAccounts.Contains(LinkedAccountKey))
                        {
                            UnificationState->ExternalAccounts.Add(LinkedAccountKey, ExternalUserInfosWithAvatarUrl);
                        }
                    }
                }
            }

            // Query external accounts in parallel.
            using FOp = FMultiOperation<TTuple<EOS_EExternalAccountType, TArray<FString>>, bool>;
            FOp::Run(
                UnpackMap(ExternalAccountIdsByType),
                FOp::FInitiate::CreateLambda([PlatformHandle, LocalUserId, UnificationState, AssociatedAvatars](
                                                 TTuple<EOS_EExternalAccountType, TArray<FString>> KV,
                                                 FOp::FOnEntryDone OnExternalAccountDone) {
                    auto UserCache = PlatformHandle->GetSystem<IUserCacheSystem>();
                    UserCache->GetUsersByExternalAccountIds(
                        LocalUserId,
                        KV.Key,
                        KV.Value,
                        IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::CreateLambda(
                            [ExternalAccountType = KV.Key, UnificationState, AssociatedAvatars, OnExternalAccountDone](
                                const FError &UserCacheErrorCode,
                                const TMap<FString, FErrorableUserInfo> &UserInfos) {
                                if (!UserCacheErrorCode.WasSuccessful())
                                {
                                    UE_LOG(
                                        LogRedpointEOSFriends,
                                        Warning,
                                        TEXT("Unable to retrieve product user IDs for external account type '%s': %s"),
                                        *ExternalAccountTypeMap.Get(ExternalAccountType),
                                        *UserCacheErrorCode.ToLogString());
                                }
                                else
                                {
                                    for (const auto &UserInfoKV : UserInfos)
                                    {
                                        if (!UserInfoKV.Value.IsErrored())
                                        {
                                            auto Key = FExternalAccountTypeAndId(ExternalAccountType, UserInfoKV.Key);
                                            auto *AvatarUrl = AssociatedAvatars.Find(Key);
                                            UnificationState->ExternalAccountsToFullAccounts.Add(
                                                Key,
                                                FUserInfoAndAvatarUrl{
                                                    UserInfoKV.Value.GetUserInfo(),
                                                    AvatarUrl != nullptr ? *AvatarUrl : TOptional<FString>()});
                                        }
                                    }
                                }
                                OnExternalAccountDone.ExecuteIfBound(true);
                            }));
                    return true;
                }),
                FOp::FOnDone::CreateLambda([OnTaskDone](const TArray<bool> &) {
                    OnTaskDone.ExecuteIfBound(Errors::Success());
                }));
        },
        PreviousTask);

    // Assemble unified friends from runtime integration results.
    PreviousTask = Executor->Add(
        [UnificationState](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            TAccountIdMap<FUnifiedFriendPartialState> PartialStates;

            // Iterate through the external accounts that we found a product user ID for and start building up the
            // unified friend states.
            for (const auto &ExternalAccountTypeAndIdToUserInfoAndAvatarUrl :
                 UnificationState->ExternalAccountsToFullAccounts)
            {
                checkf(
                    ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Value.UserInfo.IsValid(),
                    TEXT("UserInfo should not be null in ExternalAccountTypeAndIdToUserInfoAndAvatarUrl map!"));

                auto UserId = ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Value.UserInfo->GetUserId();
                auto &PartialState = PartialStates.FindOrAdd(
                    UserId,
                    FUnifiedFriendPartialState{
                        ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Value.UserInfo,
                        TMap<FExternalAccountTypeAndId, FExternalUserInfoAndAvatarUrl>(),
                        ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Value.AvatarUrl});
                auto *ExternalUserInfoAndAvatarUrl =
                    UnificationState->ExternalAccounts.Find(ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Key);
                if (ExternalUserInfoAndAvatarUrl != nullptr)
                {
                    PartialState.ExternalUserInfos.Add(
                        ExternalAccountTypeAndIdToUserInfoAndAvatarUrl.Key,
                        *ExternalUserInfoAndAvatarUrl);
                }
            }

            // Now populate unified friends from the states.
            for (const auto &AccountIdToPartialState : PartialStates)
            {
                TMap<FString, TSharedPtr<FExternalUserInfo>> RuntimePlatformIntegrationExternalFriends;
                TOptional<FString> AvatarUrl;
                if (AccountIdToPartialState.Value.AvatarUrl.IsSet())
                {
                    AvatarUrl = AccountIdToPartialState.Value.AvatarUrl;
                }
                for (const auto &ExternalAccountTypeAndIdToExternalUserInfoAndAvatarUrl :
                     AccountIdToPartialState.Value.ExternalUserInfos)
                {
                    auto ExternalKey =
                        ExternalAccountTypeMap.Get(ExternalAccountTypeAndIdToExternalUserInfoAndAvatarUrl.Key.Get<0>())
                            .ToLower();
                    RuntimePlatformIntegrationExternalFriends.Add(
                        ExternalKey,
                        ExternalAccountTypeAndIdToExternalUserInfoAndAvatarUrl.Value.ExternalUserInfo);
                    if (!AvatarUrl.IsSet() &&
                        ExternalAccountTypeAndIdToExternalUserInfoAndAvatarUrl.Value.AvatarUrl.IsSet())
                    {
                        AvatarUrl = ExternalAccountTypeAndIdToExternalUserInfoAndAvatarUrl.Value.AvatarUrl;
                    }
                }
                UnificationState->UnifiedFriends.Add(
                    AccountIdToPartialState.Key,
                    MakeShared<FUnifiedUserInfo>(
                        AccountIdToPartialState.Value.PuidUserInfo.ToSharedRef(),
                        RuntimePlatformIntegrationExternalFriends,
                        AvatarUrl));
            }

            OnTaskDone.ExecuteIfBound(Errors::Success());
        },
        PreviousTask);

    // Load information for any cross-platform friends that we don't already have information for.
    PreviousTask = Executor->Add(
        [PlatformHandle, LocalUserId, CallContext, LocalUserFriendState, RuntimePlatformIntegrations, UnificationState](
            const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            auto UserCache = PlatformHandle->GetSystem<IUserCacheSystem>();

            // Get a unique list of users that we need to lookup.
            TArray<FIdentityUserId> FriendUserIds;
            auto FoundAvatarUrls = MakeShared<TAccountIdMap<TOptional<FString>>>();
            for (const auto &SerializedFriend : LocalUserFriendState->FriendDatabase->Friends)
            {
                if (!UnificationState->UnifiedFriends.Contains(SerializedFriend.Key))
                {
                    FriendUserIds.AddUnique(SerializedFriend.Key);
                    if (SerializedFriend.Value.AvatarUrl.IsSet())
                    {
                        FoundAvatarUrls->Add(SerializedFriend.Key, SerializedFriend.Value.AvatarUrl);
                    }
                }
            }

            // Lookup the users by their product user IDs.
            UserCache->GetUsers(
                LocalUserId,
                FriendUserIds,
                IUserCacheSystem::FOnGetUsersComplete::CreateLambda(
                    [OnTaskDone,
                     CallContext,
                     LocalUserFriendState,
                     FoundAvatarUrls,
                     RuntimePlatformIntegrations,
                     UnificationState](
                        const FError &UserCacheErrorCode,
                        const TMap<UE::Online::FAccountId, FErrorableUserInfo> &UserInfos) {
                        if (!UserCacheErrorCode.WasSuccessful())
                        {
                            UE_LOG(
                                LogRedpointEOSFriends,
                                Warning,
                                TEXT("Unable to retrieve user information for cross-platform friends: %s"),
                                *UserCacheErrorCode.ToLogString());
                            OnTaskDone.ExecuteIfBound(Errors::Success());
                            return;
                        }

                        // Grab the list of friends we could actually get information about.
                        TArray<FUserInfoRef> FriendUserInfos;
                        for (const auto &UserInfoKV : UserInfos)
                        {
                            if (!UserInfoKV.Value.IsErrored())
                            {
                                FriendUserInfos.Add(UserInfoKV.Value.GetUserInfo());
                            }
                        }

                        // Query for their avatars across all runtime platform integrations.
                        using FIntegrationOp = FMultiOperation<TSharedRef<IRuntimePlatformIntegration>, bool>;
                        FIntegrationOp::Run(
                            RuntimePlatformIntegrations,
                            FIntegrationOp::FInitiate::CreateLambda(
                                [CallContext, FriendUserInfos, LocalUserFriendState, FoundAvatarUrls, UnificationState](
                                    TSharedRef<IRuntimePlatformIntegration> Integration,
                                    FIntegrationOp::FOnEntryDone OnIntegrationDone) -> bool {
                                    // Load all of the avatars.
                                    using FAvatarOp = FMultiOperation<FUserInfoRef, TOptional<FString>>;
                                    FAvatarOp::Run(
                                        FriendUserInfos,
                                        FAvatarOp::FInitiate::CreateLambda(
                                            [Integration, CallContext](
                                                FUserInfoRef UserInfo,
                                                FAvatarOp::FOnEntryDone OnAvatarDone) -> bool {
                                                Integration->GetAvatarService()->GetAvatarUrl(
                                                    CallContext,
                                                    UserInfo,
                                                    IRuntimePlatformAvatarService::FOnGetAvatarUrlComplete::
                                                        CreateLambda([OnAvatarDone](
                                                                         const FExternalUserInfoRef &,
                                                                         const FError &AvatarError,
                                                                         const FString &AvatarUrl) {
                                                            if (AvatarError.WasSuccessful() && !AvatarUrl.IsEmpty())
                                                            {
                                                                OnAvatarDone.ExecuteIfBound(AvatarUrl);
                                                            }
                                                            else
                                                            {
                                                                OnAvatarDone.ExecuteIfBound(TOptional<FString>());
                                                            }
                                                        }));
                                                return true;
                                            }),
                                        FAvatarOp::FOnDone::CreateLambda(
                                            [OnIntegrationDone, FoundAvatarUrls, FriendUserInfos](
                                                const TArray<TOptional<FString>> &AvatarUrls) {
                                                for (int i = 0; i < FriendUserInfos.Num(); i++)
                                                {
                                                    const auto &MutualFriend = FriendUserInfos[i];
                                                    const auto &AvatarUrl = AvatarUrls[i];

                                                    if (AvatarUrl.IsSet())
                                                    {
                                                        FoundAvatarUrls->Add(MutualFriend->GetUserId(), AvatarUrl);
                                                    }
                                                }
                                                OnIntegrationDone.ExecuteIfBound(true);
                                            }));
                                    return true;
                                }),
                            FIntegrationOp::FOnDone::CreateLambda(
                                [UnificationState, FriendUserInfos, LocalUserFriendState, FoundAvatarUrls, OnTaskDone](
                                    const TArray<bool> &) {
                                    for (const auto &FriendUserInfo : FriendUserInfos)
                                    {
                                        auto AvatarUrl = FoundAvatarUrls->FindRef(FriendUserInfo->GetUserId());

                                        // Update the serialized friend in the cross-platform friends database.
                                        if (AvatarUrl.IsSet())
                                        {
                                            auto *SerializedFriend = LocalUserFriendState->FriendDatabase->Friends.Find(
                                                FriendUserInfo->GetUserId());
                                            if (SerializedFriend != nullptr)
                                            {
                                                SerializedFriend->AvatarUrl = AvatarUrl;

                                                if (!LocalUserFriendState->bHasCacheUpdateEverDirtiedDatabase)
                                                {
                                                    // We only permit friend unification to ever dirty the database
                                                    // once, while friend unification is happening initially during
                                                    // login. We don't want to dirty the database on subsequent calls,
                                                    // as friend unification happens as part of ImmediateRefresh which
                                                    // can happen as part of ImmediateSave (which results in an unending
                                                    // save loop if friend unification always dirties).
                                                    LocalUserFriendState->bDirty = true;
                                                    LocalUserFriendState->bHasCacheUpdateEverDirtiedDatabase = true;
                                                }
                                            }
                                        }

                                        // Add the unified friend.
                                        UnificationState->UnifiedFriends.Add(
                                            FriendUserInfo->GetUserId(),
                                            MakeShared<FUnifiedUserInfo>(
                                                FriendUserInfo,
                                                TMap<FString, TSharedPtr<FExternalUserInfo>>(),
                                                AvatarUrl));
                                    }
                                    OnTaskDone.ExecuteIfBound(Errors::Success());
                                }));
                    }));
        },
        PreviousTask);

    // Calculate non-unified friends.
    PreviousTask = Executor->Add(
        [PlatformHandle, CallContext, UnificationState](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            TSet<FExternalAccountTypeAndId> SeenExternalAccountIds;
            for (const auto &UnifiedFriendKV : UnificationState->UnifiedFriends)
            {
                for (const auto &LinkedAccount : UnifiedFriendKV.Value->GetLinkedExternalAccounts())
                {
                    SeenExternalAccountIds.Add(
                        FExternalAccountTypeAndId(LinkedAccount.Key, LinkedAccount.Value.AccountId));
                }
            }
            for (const auto &ExternalAccountTypeAndIdToExternalUserInfoAndAvatar : UnificationState->ExternalAccounts)
            {
                if (SeenExternalAccountIds.Contains(ExternalAccountTypeAndIdToExternalUserInfoAndAvatar.Key))
                {
                    continue;
                }
                SeenExternalAccountIds.Add(ExternalAccountTypeAndIdToExternalUserInfoAndAvatar.Key);
                UnificationState->NonUnifiedFriends.Add(
                    ExternalAccountTypeAndIdToExternalUserInfoAndAvatar.Value.ExternalUserInfo.ToSharedRef());
            }
            OnTaskDone.ExecuteIfBound(Errors::Success());
        },
        PreviousTask);

    // Return results.
    PreviousTask = Executor->Add(
        [UnificationState, OnComplete](const FAsyncTaskGraph::FOnComplete &OnTaskDone) {
            OnComplete.ExecuteIfBound(
                Errors::Success(),
                UnificationState->UnifiedFriends,
                UnificationState->NonUnifiedFriends);
            OnTaskDone.ExecuteIfBound(Errors::Success());
        },
        PreviousTask);

    // Run async task graph.
    Executor->Start();
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()