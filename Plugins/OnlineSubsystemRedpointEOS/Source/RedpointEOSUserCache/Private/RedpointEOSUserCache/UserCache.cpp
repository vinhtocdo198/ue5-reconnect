// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/UserCache.h"

#include "RedpointEOSAPI/Auth/GetLoggedInAccounts.h"
#include "RedpointEOSAPI/Common/ExternalAccountType.h"
#include "RedpointEOSAPI/Connect/CopyProductUserExternalAccountByAccountType.h"
#include "RedpointEOSAPI/Connect/CopyProductUserExternalAccounts.h"
#include "RedpointEOSAPI/Connect/CopyProductUserInfo.h"
#include "RedpointEOSAPI/Connect/GetExternalAccountMapping.h"
#include "RedpointEOSAPI/Connect/QueryExternalAccountMappings.h"
#include "RedpointEOSAPI/UserInfo/CopyUserInfo.h"
#include "RedpointEOSAPI/UserInfo/QueryUserInfo.h"
#include "RedpointEOSAPI/UserInfo/QueryUserInfoByDisplayName.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSUserCache/Logging.h"
#include "RedpointEOSUserCache/UserInfoImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(375513567, Redpoint::EOS::UserCache)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::API::Common;
using namespace ::Redpoint::EOS::Core::Utils;

EOS_ProductUserId FUserCache::GetLocalUserIdParameter() const
{
    using namespace ::Redpoint::EOS::Core::Id;

    return IsDedicatedServerId(this->LocalUserId) ? nullptr : GetProductUserId(this->LocalUserId);
}

FUserCache::FUserCache(
    const FPlatformHandle &InPlatformHandle,
    const FPlatformInstanceEnvironmentRef &InPlatformInstanceEnvironment,
    const UE::Online::FAccountId &InLocalUserId)
    : PlatformHandle(InPlatformHandle)
    , PlatformInstanceEnvironment(InPlatformInstanceEnvironment)
    , LocalUserId(InLocalUserId)
    , OperationQueue(MakeShared<FExclusiveAsyncOperationQueue>())
    , UsersCached()
    , UsersPendingCallback()
    , EpicAccountIdsByDisplayNameCached()
    , EpicAccountIdsByDisplayNamePendingCallback()
    , UserIdsByExternalAccountIdCached()
    , UserIdsByExternalAccountIdPendingCallback()
{
}

void FUserCache::GetUser(
    const UE::Online::FAccountId &InTargetUserId,
    const IUserCacheSystem::FOnGetUserComplete &InOnComplete)
{
    
    UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("Queuing 1 user for resolution (GetUser)"));

    auto &ExistingUser = this->UsersCached.FindOrAdd(InTargetUserId);
    if (!ExistingUser.IsValid())
    {
        ExistingUser = MakeShared<FUsersCache::FCacheEntry>(InTargetUserId);
    }
    if (ExistingUser->Value->IsReady())
    {
        UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("All 1 user have already been resolved."));
        InOnComplete.ExecuteIfBound(Errors::Success(), ExistingUser->Value);
        return;
    }

    UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("Scheduled pending callback for resolution of 1 user."));
    TArray<UE::Online::FAccountId> TargetUserIds;
    TargetUserIds.Add(InTargetUserId);
    this->UsersPendingCallback.Add(MakeShared<FUsersCache::FPendingCallback>(
        TargetUserIds,
        FUsersCache::FPendingCallback::FOnResolved::CreateSPLambda(
            this,
            [this, InOnComplete](const FUsersCache::FPendingCallback &Pending) {
                auto TargetUser = this->UsersCached.FindRef(Pending.RequiredKeys[0]);
                if (!TargetUser.IsValid())
                {
                    InOnComplete.ExecuteIfBound(
                        Errors::NotFound(
                            ANSI_TO_TCHAR(__FUNCTION__),
                            TEXT("User did not exist in cache when OnUsersResolvedForGetUser fired!")),
                        nullptr);
                }
                else
                {
                    InOnComplete.ExecuteIfBound(
                        TargetUser->Error.IsValid() ? *TargetUser->Error : Errors::Success(),
                        TargetUser->Value);
                }
            })));
    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
}

void FUserCache::GetUsers(
    const TArray<UE::Online::FAccountId> &InTargetUserIds,
    const IUserCacheSystem::FOnGetUsersComplete &InOnComplete)
{
    
    UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("Queuing %d users for resolution (GetUsers)"), InTargetUserIds.Num());

    TArray<UE::Online::FAccountId> RemainingTargetUserIds;
    TMap<UE::Online::FAccountId, FErrorableUserInfo> ResolvedUsers;
    for (const auto &TargetUserId : InTargetUserIds)
    {
        auto &ExistingUser = this->UsersCached.FindOrAdd(TargetUserId);
        if (!ExistingUser.IsValid())
        {
            ExistingUser = MakeShared<FUsersCache::FCacheEntry>(TargetUserId);
        }
        if (ExistingUser->Value->IsReady())
        {
            ResolvedUsers.Add(ExistingUser->Value->GetUserId(), FErrorableUserInfo(ExistingUser->Value));
        }
        else
        {
            ExistingUser->Error = nullptr;
            RemainingTargetUserIds.Add(TargetUserId);
        }
    }
    if (RemainingTargetUserIds.Num() == 0)
    {
        UE_LOG(
            LogRedpointEOSUserCache,
            Verbose,
            TEXT("All %d users have already been resolved."),
            InTargetUserIds.Num());
        InOnComplete.ExecuteIfBound(Errors::Success(), ResolvedUsers);
        return;
    }

    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Scheduled pending callback for resolution of %d users."),
        InTargetUserIds.Num());
    this->UsersPendingCallback.Add(MakeShared<FUsersCache::FPendingCallback>(
        InTargetUserIds,
        FUsersCache::FPendingCallback::FOnResolved::CreateSPLambda(
            this,
            [this, InOnComplete](const FUsersCache::FPendingCallback &Pending) {
                TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos;
                bool bPartialResult = false;
                for (const auto &RequiredUser : Pending.RequiredKeys)
                {
                    auto TargetUser = this->UsersCached.FindRef(RequiredUser);
                    if (!TargetUser.IsValid())
                    {
                        InOnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("User did not exist in cache when OnUsersResolvedForGetUsers fired!")),
                            TMap<UE::Online::FAccountId, FErrorableUserInfo>());
                        return;
                    }
                    if (TargetUser->Error.IsValid())
                    {
                        bPartialResult = true;
                        UserInfos.Add(TargetUser->Value->GetUserId(), FErrorableUserInfo(*TargetUser->Error));
                    }
                    else
                    {
                        UserInfos.Add(TargetUser->Value->GetUserId(), FErrorableUserInfo(TargetUser->Value));
                    }
                }
                InOnComplete.ExecuteIfBound(bPartialResult ? Errors::PartialResult() : Errors::Success(), UserInfos);
            })));
    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
}

FUserInfoRef FUserCache::GetUnresolvedUser(const UE::Online::FAccountId &InTargetUserId)
{
    auto &ExistingUser = this->UsersCached.FindOrAdd(InTargetUserId);
    if (!ExistingUser.IsValid())
    {
        ExistingUser = MakeShared<FUsersCache::FCacheEntry>(InTargetUserId);
    }
    if (ExistingUser->Value->IsReady())
    {
        return ExistingUser->Value;
    }
    ExistingUser->Error = nullptr;

    UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("Returning unresolved user."));
    TArray<UE::Online::FAccountId> TargetUserIds;
    TargetUserIds.Add(InTargetUserId);
    this->UsersPendingCallback.Add(
        MakeShared<FUsersCache::FPendingCallback>(TargetUserIds, FUsersCache::FPendingCallback::FOnResolved()));
    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
    return ExistingUser->Value;
}

void FUserCache::GetUsersByEpicGamesDisplayNames(
    const TArray<FString> &InDisplayNames,
    const IUserCacheSystem::FOnGetUsersByEpicGamesDisplayNamesComplete &InOnComplete)
{
    
    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Queuing %d Epic Games display names for resolution (GetUsersByEpicGamesDisplayNames)"),
        InDisplayNames.Num());

    for (const auto &DisplayName : InDisplayNames)
    {
        auto &ExistingAccountId = this->EpicAccountIdsByDisplayNameCached.FindOrAdd(DisplayName);
        if (!ExistingAccountId.IsValid())
        {
            ExistingAccountId =
                MakeShared<FEpicAccountIdsByDisplayNameCache::FCacheEntry>(DisplayName, TOptional<FString>());
        }
        if (!ExistingAccountId->Value.IsSet())
        {
            ExistingAccountId->Error = nullptr;
        }
    }

    // @note: There is no short circuit for this method, since we need to chain into resolving PUIDs from the Epic Games
    // account IDs anyway.

    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Scheduled pending callback for resolution of %d Epic Games display names."),
        InDisplayNames.Num());
    this->EpicAccountIdsByDisplayNamePendingCallback.Add(MakeShared<
                                                         FEpicAccountIdsByDisplayNameCache::FPendingCallback>(
        InDisplayNames,
        FEpicAccountIdsByDisplayNameCache::FPendingCallback::FOnResolved::CreateSPLambda(
            this,
            [this, InDisplayNames, InOnComplete](const FEpicAccountIdsByDisplayNameCache::FPendingCallback &Pending) {
                UE_LOG(
                    LogRedpointEOSUserCache,
                    Verbose,
                    TEXT("Received callback for resolution of %d Epic Games display names."),
                    InDisplayNames.Num());
                TMap<FString, FErrorableUserInfo> ErroredOnResolve;
                TSet<FString> UniqueEpicAccountIds;
                TMap<FString, FString> DisplayNameToEpicAccountIdMap;
                for (const auto &RequiredDisplayName : Pending.RequiredKeys)
                {
                    auto TargetEpicAccountId = this->EpicAccountIdsByDisplayNameCached.FindRef(RequiredDisplayName);
                    if (!TargetEpicAccountId.IsValid())
                    {
                        InOnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Epic account ID cache entry did not exist in cache when callback of "
                                     "GetUsersByEpicGamesDisplayNames fired!")),
                            TMap<FString, FErrorableUserInfo>());
                        return;
                    }
                    if (TargetEpicAccountId->Error.IsValid())
                    {
                        ErroredOnResolve.Add(RequiredDisplayName, FErrorableUserInfo(*TargetEpicAccountId->Error));
                    }
                    else if (TargetEpicAccountId->Value.IsSet())
                    {
                        UniqueEpicAccountIds.Add(TargetEpicAccountId->Value.GetValue());
                        DisplayNameToEpicAccountIdMap.Add(RequiredDisplayName, TargetEpicAccountId->Value.GetValue());
                    }
                    else
                    {
                        InOnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Epic account ID cache entry did not have either error or value when callback of "
                                     "GetUsersByEpicGamesDisplayNames fired!")),
                            TMap<FString, FErrorableUserInfo>());
                        return;
                    }
                }

                // If we have no Epic account IDs to resolve, then it's all errored.
                if (UniqueEpicAccountIds.Num() == 0)
                {
                    UE_LOG(
                        LogRedpointEOSUserCache,
                        Verbose,
                        TEXT("Immediately returning from Epic Games display name callback as there is no further "
                             "querying to be done."));
                    InOnComplete.ExecuteIfBound(
                        InDisplayNames.Num() == 0 ? Errors::Success() : Errors::PartialResult(),
                        ErroredOnResolve);
                    return;
                }

                // Use our own external account lookup to resolve this.
                TArray<FString> EpicAccountIdsArray;
                for (const auto &EpicAccountId : UniqueEpicAccountIds)
                {
                    EpicAccountIdsArray.Add(EpicAccountId);
                }
                UE_LOG(
                    LogRedpointEOSUserCache,
                    Verbose,
                    TEXT("Requesting resolution of %d Epic Games external account mappings."),
                    EpicAccountIdsArray.Num());
                this->GetUsersByExternalAccountIds(
                    EOS_EExternalAccountType::EOS_EAT_EPIC,
                    EpicAccountIdsArray,
                    IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete::CreateSPLambda(
                        this,
                        [ErroredOnResolve,
                         DisplayNameToEpicAccountIdMap,
                         InOnComplete](FError ErrorCode, TMap<FString, FErrorableUserInfo> UserInfos) {
                            // Build our results map by iterating through our original list of display names and their
                            // resolved account IDs, and adding the ones we found from UserInfos and filling in ones we
                            // don't have as nullptr.
                            TMap<FString, FErrorableUserInfo> Results;
                            bool bPartialResult = false;
                            for (const auto &KV : ErroredOnResolve)
                            {
                                bPartialResult = true;
                                Results.Add(KV.Key, KV.Value);
                            }
                            for (const auto &KV : DisplayNameToEpicAccountIdMap)
                            {
                                auto *ResolvedUserInfo = UserInfos.Find(KV.Value);
                                if (ResolvedUserInfo != nullptr)
                                {
                                    bPartialResult = bPartialResult || ResolvedUserInfo->IsErrored();
                                    Results.Add(KV.Key, *ResolvedUserInfo);
                                }
                                else
                                {
                                    // Almost certainly means the nested call had a call-level failure, so we should
                                    // propagate that with no entries.
                                    checkf(
                                        !ErrorCode.WasSuccessful(),
                                        TEXT("Expected UserInfo on nested GetUsersByExternalAccountIds to only be "
                                             "missing for call-level failures."));
                                    InOnComplete.ExecuteIfBound(ErrorCode, TMap<FString, FErrorableUserInfo>());
                                    return;
                                }
                            }
                            InOnComplete.ExecuteIfBound(
                                bPartialResult ? Errors::PartialResult() : Errors::Success(),
                                Results);
                        }));
            })));
    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
}

void FUserCache::GetUsersByExternalAccountIds(
    EOS_EExternalAccountType InExternalAccountType,
    const TArray<FString> &InExternalAccountIds,
    const IUserCacheSystem::FOnGetUsersByExternalAccountIdsComplete &InOnComplete)
{
    
    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Queuing %d external account IDs of type %s for resolution (GetUsersByExternalAccountIds):"),
        InExternalAccountIds.Num(),
        *ExternalAccountTypeMap.Get(InExternalAccountType));
    for (const auto &ExternalAccountId : InExternalAccountIds)
    {
        UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("- %s"), *ExternalAccountId);
    }

    TArray<FUserIdsByExternalAccountIdCache::FKeyType> PendingCallbackKeys;
    for (const auto &ExternalAccountId : InExternalAccountIds)
    {
        auto Key = TTuple<EOS_EExternalAccountType, FString>(InExternalAccountType, ExternalAccountId);
        auto &ExistingAccountId = this->UserIdsByExternalAccountIdCached.FindOrAdd(Key);
        if (!ExistingAccountId.IsValid())
        {
            ExistingAccountId =
                MakeShared<FUserIdsByExternalAccountIdCache::FCacheEntry>(Key, TOptional<UE::Online::FAccountId>());
        }
        if (!ExistingAccountId->Value.IsSet())
        {
            ExistingAccountId->Error = nullptr;
        }
        PendingCallbackKeys.Add(FUserIdsByExternalAccountIdCache::FKeyType(InExternalAccountType, ExternalAccountId));
    }

    // @note: There is no short circuit for this method, since we need to chain into resolving PUIDs from the external
    // account mappings.

    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Scheduled pending callback for resolution of %d external account mappings."),
        InExternalAccountIds.Num());
    this->UserIdsByExternalAccountIdPendingCallback.Add(MakeShared<FUserIdsByExternalAccountIdCache::FPendingCallback>(
        PendingCallbackKeys,
        FUserIdsByExternalAccountIdCache::FPendingCallback::FOnResolved::CreateSPLambda(
            this,
            [this, InExternalAccountIds, InOnComplete](
                const FUserIdsByExternalAccountIdCache::FPendingCallback &Pending) {
                TMap<FString, FErrorableUserInfo> ErroredOnResolve;
                TSet<UE::Online::FAccountId> UniqueAccountIds;
                TMap<FString, UE::Online::FAccountId> ExternalIdToAccountIdMap;
                for (const auto &RequiredExternalAccountId : Pending.RequiredKeys)
                {
                    auto TargetEpicAccountId =
                        this->UserIdsByExternalAccountIdCached.FindRef(RequiredExternalAccountId);
                    if (!TargetEpicAccountId.IsValid())
                    {
                        InOnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("External mapping cache entry did not exist in cache when callback of "
                                     "GetUsersByExternalAccountIds fired!")),
                            TMap<FString, FErrorableUserInfo>());
                        return;
                    }
                    if (TargetEpicAccountId->Error.IsValid())
                    {
                        ErroredOnResolve.Add(
                            RequiredExternalAccountId.Value,
                            FErrorableUserInfo(*TargetEpicAccountId->Error));
                    }
                    else if (TargetEpicAccountId->Value.IsSet())
                    {
                        UniqueAccountIds.Add(TargetEpicAccountId->Value.GetValue());
                        ExternalIdToAccountIdMap.Add(
                            RequiredExternalAccountId.Value,
                            TargetEpicAccountId->Value.GetValue());
                    }
                    else
                    {
                        InOnComplete.ExecuteIfBound(
                            Errors::NotFound(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("External mapping cache entry did not have either error or value when callback of "
                                     "GetUsersByExternalAccountIds fired!")),
                            TMap<FString, FErrorableUserInfo>());
                        return;
                    }
                }

                // If we have no account IDs to resolve, then it's all errored.
                if (UniqueAccountIds.Num() == 0)
                {
                    InOnComplete.ExecuteIfBound(
                        InExternalAccountIds.Num() == 0 ? Errors::Success() : Errors::PartialResult(),
                        ErroredOnResolve);
                    return;
                }

                // Use our own external account lookup to resolve this.
                TArray<UE::Online::FAccountId> AccountIdsArray;
                for (const auto &AccountId : UniqueAccountIds)
                {
                    AccountIdsArray.Add(AccountId);
                }
                this->GetUsers(
                    AccountIdsArray,
                    IUserCacheSystem::FOnGetUsersComplete::CreateSPLambda(
                        this,
                        [ErroredOnResolve,
                         ExternalIdToAccountIdMap,
                         InOnComplete](FError ErrorCode, TMap<UE::Online::FAccountId, FErrorableUserInfo> UserInfos) {
                            // Build our results map by iterating through our original list of external account IDs and
                            // their resolved account IDs, and adding the ones we found from UserInfos and filling in
                            // ones we don't have as nullptr.
                            TMap<FString, FErrorableUserInfo> Results;
                            bool bPartialResult = false;
                            for (const auto &KV : ErroredOnResolve)
                            {
                                bPartialResult = true;
                                Results.Add(KV.Key, KV.Value);
                            }
                            for (const auto &KV : ExternalIdToAccountIdMap)
                            {
                                auto *ResolvedUserInfo = UserInfos.Find(KV.Value);
                                if (ResolvedUserInfo != nullptr)
                                {
                                    bPartialResult = bPartialResult || ResolvedUserInfo->IsErrored();
                                    Results.Add(KV.Key, *ResolvedUserInfo);
                                }
                                else
                                {
                                    // Almost certainly means the nested call had a call-level failure, so we should
                                    // propagate that with no entries.
                                    checkf(
                                        !ErrorCode.WasSuccessful(),
                                        TEXT("Expected UserInfo on nested GetUsers to only be "
                                             "missing for call-level failures."));
                                    InOnComplete.ExecuteIfBound(ErrorCode, TMap<FString, FErrorableUserInfo>());
                                    return;
                                }
                            }
                            InOnComplete.ExecuteIfBound(
                                bPartialResult ? Errors::PartialResult() : Errors::Success(),
                                Results);
                        }));
            })));
    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
}

void FUserCache::ClearCache()
{
    this->UsersCached.Empty();
}

template <typename TCacheType>
void Queue_Templated_FlushPendingCallbacks(
    const FString &InType,
    typename TCacheType::FCacheMap &InCached,
    typename TCacheType::FPendingCallbackMap &InPendingCallback,
    TFunction<TSharedRef<typename TCacheType::FCacheEntry>(const typename TCacheType::FKeyType &)> InEntryFactory,
    TFunction<FString(const typename TCacheType::FValueType &)> InValueToString,
    TFunction<bool(const typename TCacheType::FValueType &)> InValueIsReady)
{
    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("Queue_Templated_FlushPendingCallbacks(%s): Got %d pending callbacks remaining."),
        *InType,
        InPendingCallback.Num());
    auto PendingCallbacksCopy = InPendingCallback;
    int32 i = 0;
    for (const auto &Pending : PendingCallbacksCopy)
    {
        UE_LOG(
            LogRedpointEOSUserCache,
            Verbose,
            TEXT("Queue_Templated_FlushPendingCallbacks(%s): Pending callback #%d has %d entries to check."),
            *InType,
            i,
            Pending->RequiredKeys.Num());
        bool bAllUserIdsResolvedOrErrored = true;
        for (const auto &RequiredKey : Pending->RequiredKeys)
        {
            auto &TargetEntry = InCached.FindOrAdd(RequiredKey);
            if (!TargetEntry.IsValid())
            {
                TargetEntry = InEntryFactory(RequiredKey);
            }
            FString TargetEntryString = InValueToString(TargetEntry->Value);
            bool bTargetEntryIsReady = InValueIsReady(TargetEntry->Value);
            FString TargetEntryReady = bTargetEntryIsReady ? TEXT("Yes") : TEXT("No");
            FString TargetEntryError = TargetEntry->Error.IsValid() ? TargetEntry->Error->ToLogString() : TEXT("");
            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("Queue_Templated_FlushPendingCallbacks(%s): Pending callback #%d, entry %s: Ready? %s '%s'"),
                *InType,
                i,
                *TargetEntryString,
                *TargetEntryReady,
                *TargetEntryError);
            if (!bTargetEntryIsReady && !TargetEntry->Error.IsValid())
            {
                bAllUserIdsResolvedOrErrored = false;
                break;
            }
        }
        if (bAllUserIdsResolvedOrErrored)
        {
            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("Queue_Templated_FlushPendingCallbacks(%s): All entries resolved or errored, firing callback."),
                *InType,
                i,
                Pending->RequiredKeys.Num());
            InPendingCallback.Remove(Pending);
            Pending->OnResolved.ExecuteIfBound(*Pending);
        }
        i++;
    }
}

void FUserCache::Queue_FlushPendingCallbacksAndPotentiallySchedule()
{
    using namespace ::Redpoint::EOS::Core::Id;

    Queue_Templated_FlushPendingCallbacks<FEpicAccountIdsByDisplayNameCache>(
        TEXT("epicAccountIdsByDisplayName"),
        this->EpicAccountIdsByDisplayNameCached,
        this->EpicAccountIdsByDisplayNamePendingCallback,
        [](const FString &InKey) {
            return MakeShared<FEpicAccountIdsByDisplayNameCache::FCacheEntry>(InKey, TOptional<FString>());
        },
        [](const TOptional<FString> &InValue) {
            return InValue.IsSet() ? InValue.GetValue() : TEXT("(not resolved)");
        },
        [](const TOptional<FString> &InValue) {
            return InValue.IsSet();
        });
    Queue_Templated_FlushPendingCallbacks<FUserIdsByExternalAccountIdCache>(
        TEXT("userIdsByExternalAccountId"),
        this->UserIdsByExternalAccountIdCached,
        this->UserIdsByExternalAccountIdPendingCallback,
        [](const TTuple<EOS_EExternalAccountType, FString> &InKey) {
            return MakeShared<FUserIdsByExternalAccountIdCache::FCacheEntry>(
                InKey,
                TOptional<UE::Online::FAccountId>());
        },
        [](const TOptional<UE::Online::FAccountId> &InValue) {
            return InValue.IsSet() ? GetUserIdString(InValue.GetValue()) : TEXT("(not resolved)");
        },
        [](const TOptional<UE::Online::FAccountId> &InValue) {
            return InValue.IsSet();
        });
    Queue_Templated_FlushPendingCallbacks<FUsersCache>(
        TEXT("users"),
        this->UsersCached,
        this->UsersPendingCallback,
        [](const UE::Online::FAccountId &InKey) {
            return MakeShared<FUsersCache::FCacheEntry>(InKey);
        },
        [](const TSharedRef<FUserInfoImpl> &InValue) {
            return GetUserIdString(InValue->GetUserId());
        },
        [](const TSharedRef<FUserInfoImpl> &InValue) {
            return InValue->IsReady();
        });

    // @note: We intentionally schedule display name and external ID queries first, since they will schedule their own
    // user queries once they have internally resolved to an account ID.
    if (this->EpicAccountIdsByDisplayNamePendingCallback.Num() > 0)
    {
        this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FUserCache::EpicAccountIdsByDisplayNameQueue_Process));
    }
    if (this->UserIdsByExternalAccountIdPendingCallback.Num() > 0)
    {
        this->OperationQueue->AddToQueue(FExclusiveAsyncOperationQueue::FOperation::CreateSP(
            this,
            &FUserCache::UserIdsByExternalAccountIdQueue_Process));
    }
    if (this->UsersPendingCallback.Num() > 0)
    {
        this->OperationQueue->AddToQueue(
            FExclusiveAsyncOperationQueue::FOperation::CreateSP(this, &FUserCache::UsersQueue_Process));
    }
}

void FUserCache::UsersQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Core::Id;

    // Determine the users we're going to try to resolve in this operation.
    TSet<UE::Online::FAccountId> UsersToResolve;
    for (const auto &Pending : this->UsersPendingCallback)
    {
        for (const auto &RequiredUserId : Pending->RequiredKeys)
        {
            auto &TargetUser = this->UsersCached.FindOrAdd(RequiredUserId);
            if (!TargetUser.IsValid())
            {
                TargetUser = MakeShared<FUsersCache::FCacheEntry>(RequiredUserId);
            }
            if (TargetUser->Value->IsReady())
            {
                continue;
            }

            UsersToResolve.Add(RequiredUserId);
            if (UsersToResolve.Num() >= EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_MAX_ACCOUNT_IDS)
            {
                break;
            }
        }
        if (UsersToResolve.Num() >= EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_MAX_ACCOUNT_IDS)
        {
            break;
        }
    }

    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("ProcessQueue: Querying %d external account mappings."),
        UsersToResolve.Num());

    // If we have no users to resolve, we're done.
    if (UsersToResolve.Num() == 0)
    {
        this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
        InOnDone.ExecuteIfBound();
        return;
    }

    // Query the external account mappings for those accounts.
    TArray<EOS_ProductUserId> TargetProductUserIds;
    for (const auto &UserId : UsersToResolve)
    {
        TargetProductUserIds.Add(GetProductUserId(UserId));
    }
    FQueryProductUserIdMappings::Execute(
        this->PlatformHandle,
        FQueryProductUserIdMappings::Options{this->GetLocalUserIdParameter(),
            TargetProductUserIds},
        FQueryProductUserIdMappings::CompletionDelegate::CreateSP(
            this,
            &FUserCache::UsersQueue_OnProductUserIdMappingsQueried,
            UsersToResolve,
            InOnDone));
}

void FUserCache::UsersQueue_OnProductUserIdMappingsQueried(
    const API::Connect::FQueryProductUserIdMappings::Result &InResult,
    TSet<UE::Online::FAccountId> InUsersToResolve,
    FExclusiveAsyncOperationQueue::FOnDone InOnDone)
{
    UE_LOG(
        LogRedpointEOSUserCache,
        Verbose,
        TEXT("ProcessQueue: Queried %d external account mappings."),
        InUsersToResolve.Num());

    // If we have an error, propagate it to all of the users we were attempting to resolve.
    if (InResult.ResultCode != EOS_EResult::EOS_Success)
    {
        UE_LOG(
            LogRedpointEOSUserCache,
            Verbose,
            TEXT("ProcessQueue: Got error while querying external account mappings: %s"),
            *ConvertError(InResult.ResultCode).ToLogString());
        for (const auto &UserId : InUsersToResolve)
        {
            auto &TargetUser = this->UsersCached.FindOrAdd(UserId);
            if (!TargetUser.IsValid())
            {
                TargetUser = MakeShared<FUsersCache::FCacheEntry>(UserId);
            }
            if (!TargetUser->Error.IsValid() && !TargetUser->Value->IsReady())
            {
                TargetUser->Error = MakeShared<FError>(ConvertError(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("Failed to query product user ID mappings."),
                    InResult.ResultCode));
            }
        }
        this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
        InOnDone.ExecuteIfBound();
        return;
    }
    else
    {
        using namespace ::Redpoint::EOS::API::Connect;
        using namespace ::Redpoint::EOS::Core::Id;
        
        UE_LOG(
            LogRedpointEOSUserCache,
            Verbose,
            TEXT("ProcessQueue: Processing successfully retrieved external account mappings."));

        // Attempt to figure out what the local Epic Games account is for the local user.
        TOptional<EOS_EpicAccountId> LocalEpicAccountId;
        if (!IsDedicatedServerId(this->LocalUserId))
        {
            EOS_EResult CopyEpicIdResult;
            FCopyProductUserExternalAccountByAccountType::Result CopyEpicId;
            FCopyProductUserExternalAccountByAccountType::Execute(
                this->PlatformHandle,
                FCopyProductUserExternalAccountByAccountType::Options{
                    GetProductUserId(LocalUserId),
                    EOS_EExternalAccountType::EOS_EAT_EPIC},
                CopyEpicIdResult,
                CopyEpicId);
            if (CopyEpicIdResult == EOS_EResult::EOS_Success)
            {
                using namespace ::Redpoint::EOS::API::Auth;

                TArray<EOS_EpicAccountId> ExistingUsersResult;
                EOS_EResult ExistingUsersResultCode;
                FGetLoggedInAccounts::Execute(PlatformHandle, ExistingUsersResultCode, ExistingUsersResult);
                for (const auto &EpicAccountId : ExistingUsersResult)
                {
                    if (GetEpicGamesUserIdString(EpicAccountId) == CopyEpicId.AccountId)
                    {
                        LocalEpicAccountId = EpicAccountId;
                        break;
                    }
                }
            }
        }

        // Now iterate through all of the users and attempt to load in their resolved data.
        TArray<TTuple<UE::Online::FAccountId, TSharedRef<FUserInfoData>>> EpicAccountsToResolve;
        for (const auto &UserId : InUsersToResolve)
        {
            TSharedRef<FUserInfoData> UserInfoData = MakeShared<FUserInfoData>();

            // Retrieve and attach external account data.
            {
                EOS_EResult ExternalAccountsResult;
                TArray<FCopyProductUserExternalAccounts::ResultEntry> ExternalAccounts;
                FCopyProductUserExternalAccounts::Execute(
                    this->PlatformHandle,
                    FCopyProductUserExternalAccounts::Options{GetProductUserId(UserId)},
                    ExternalAccountsResult,
                    ExternalAccounts);
                for (const auto &ExternalAccount : ExternalAccounts)
                {
                    UserInfoData->LinkedExternalAccounts.Add(
                        ExternalAccount.AccountIdType,
                        FUserExternalAccountInfo{
                            ExternalAccount.DisplayName,
                            ExternalAccount.AccountId,
                            ExternalAccount.AccountIdType,
                            ExternalAccount.LastLoginTime,
                        });
                }
            }

            // Retrieve and set the "default"/"last logged in" platform.
            {
                EOS_EResult ProductUserResult;
                FCopyProductUserInfo::Result ProductUser;
                FCopyProductUserInfo::Execute(
                    this->PlatformHandle,
                    FCopyProductUserInfo::Options{GetProductUserId(UserId)},
                    ProductUserResult,
                    ProductUser);
                if (ProductUserResult == EOS_EResult::EOS_Success)
                {
                    UserInfoData->MostRecentlySignedInPlatform = ProductUser.AccountIdType;
                }
                else if (UserInfoData->LinkedExternalAccounts.Num() > 0)
                {
                    // Fallback to just picking a platform if we couldn't get the last sign in info.
                    for (const auto &KV : UserInfoData->LinkedExternalAccounts)
                    {
                        UserInfoData->MostRecentlySignedInPlatform = KV.Key;
                        break;
                    }
                }
            }

            if (UserInfoData->LinkedExternalAccounts.Contains(EOS_EExternalAccountType::EOS_EAT_EPIC) &&
                LocalEpicAccountId.IsSet())
            {
                // If the user has Epic Games linked and we are also an Epic Games user locally, we need to go and fetch
                // that information.
                EpicAccountsToResolve.Add(
                    TTuple<UE::Online::FAccountId, TSharedRef<FUserInfoData>>(UserId, UserInfoData));
            }
            else
            {
                // Otherwise, this account is done.

                // Ask the user info to materialize all of the attributes.
                UserInfoData->MaterializeAttributes(UserId);

                // Update the target user with the fetched information.
                auto &TargetUser = this->UsersCached.FindOrAdd(UserId);
                if (!TargetUser.IsValid())
                {
                    TargetUser = MakeShared<FUsersCache::FCacheEntry>(UserId);
                }
                TargetUser->Error = nullptr;
                TargetUser->Value->SetUserInfoData(UserInfoData);
            }
        }

        // If we have Epic Games accounts to fetch, go and do that now.
        if (EpicAccountsToResolve.Num() > 0)
        {
            using namespace ::Redpoint::EOS::API::UserInfo;

            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("ProcessQueue: Querying %d Epic Games accounts."),
                EpicAccountsToResolve.Num());

            // Run all of the queries.
            FMultiOperation<TTuple<UE::Online::FAccountId, TSharedRef<FUserInfoData>>, bool>::RunSP(
                this->AsShared(),
                EpicAccountsToResolve,
                [this, LocalEpicAccountId](
                    TTuple<UE::Online::FAccountId, TSharedRef<FUserInfoData>> Value,
                    TFunction<void(bool OutValue)> OnDone) -> bool {
                    // Query the target Epic Games account information.
                    UE_LOG(
                        LogRedpointEOSUserCache,
                        Verbose,
                        TEXT("ProcessQueue: Querying Epic Games account '%s' from local Epic Games account '%s'..."),
                        *Value.Value->LinkedExternalAccounts[EOS_EExternalAccountType::EOS_EAT_EPIC].AccountId,
                        *GetEpicGamesUserIdString(LocalEpicAccountId.GetValue()));
                    EOS_EpicAccountId LocalEpicAccountIdValue = LocalEpicAccountId.GetValue();
                    EOS_EpicAccountId TargetEpicAccountIdValue = GetEpicGamesAccountId(
                        Value.Value->LinkedExternalAccounts[EOS_EExternalAccountType::EOS_EAT_EPIC].AccountId);
                    FQueryUserInfo::Execute(
                        this->PlatformHandle,
                        FQueryUserInfo::Options{LocalEpicAccountIdValue, TargetEpicAccountIdValue},
                        FQueryUserInfo::CompletionDelegate::CreateSPLambda(
                            this,
                            [this, Value, LocalEpicAccountIdValue, TargetEpicAccountIdValue, OnDone](
                                const FQueryUserInfo::Result &QueryResult) {
                                // Attempt to get information about this Epic Games user.
                                UE_LOG(
                                    LogRedpointEOSUserCache,
                                    Verbose,
                                    TEXT("ProcessQueue: FQueryUserInfo result returned for user ID '%s', with error "
                                         "log: %s"),
                                    *GetEpicGamesUserIdString(QueryResult.TargetUserId),
                                    *ConvertError(QueryResult.ResultCode).ToLogString());
                                if (QueryResult.ResultCode == EOS_EResult::EOS_Success)
                                {
                                    UE_LOG(
                                        LogRedpointEOSUserCache,
                                        Verbose,
                                        TEXT("ProcessQueue: FCopyUserInfo being called for user ID '%s'"),
                                        *GetEpicGamesUserIdString(TargetEpicAccountIdValue));
                                    EOS_EResult CopyTargetEpicResult;
                                    FCopyUserInfo::Result CopyTargetEpic;
                                    FCopyUserInfo::Execute(
                                        this->PlatformHandle,
                                        FCopyUserInfo::Options{LocalEpicAccountIdValue, TargetEpicAccountIdValue},
                                        CopyTargetEpicResult,
                                        CopyTargetEpic);
                                    if (CopyTargetEpicResult == EOS_EResult::EOS_Success)
                                    {
                                        UE_LOG(
                                            LogRedpointEOSUserCache,
                                            Verbose,
                                            TEXT("ProcessQueue: Associated Epic Games account ID '%s' with product "
                                                 "user ID '%s'."),
                                            *GetEpicGamesUserIdString(TargetEpicAccountIdValue),
                                            *GetUserIdString(Value.Key));
                                        Value.Value->LinkedEpicGamesAccount = FUserEpicGamesAccountInfo{
                                            TargetEpicAccountIdValue,
                                            CopyTargetEpic.Country,
                                            CopyTargetEpic.DisplayName,
                                            CopyTargetEpic.PreferredLanguage,
                                            CopyTargetEpic.Nickname,
                                            CopyTargetEpic.DisplayNameSanitized,
                                        };
                                    }
                                    else
                                    {
                                        UE_LOG(
                                            LogRedpointEOSUserCache,
                                            Verbose,
                                            TEXT("ProcessQueue: Unable to look up Epic Games account ID '%s' that was "
                                                 "associated with user ID '%s': %s"),
                                            *GetEpicGamesUserIdString(TargetEpicAccountIdValue),
                                            *GetUserIdString(Value.Key),
                                            *ConvertError(
                                                 ANSI_TO_TCHAR(__FUNCTION__),
                                                 TEXT("CopyUserInfo call failed."),
                                                 CopyTargetEpicResult)
                                                 .ToLogString());
                                    }
                                }
                                else
                                {
                                    UE_LOG(
                                        LogRedpointEOSUserCache,
                                        Verbose,
                                        TEXT("ProcessQueue: Unable to look up Epic Games account ID '%s' that was "
                                             "associated with user ID '%s': %s"),
                                        *GetEpicGamesUserIdString(TargetEpicAccountIdValue),
                                        *GetUserIdString(Value.Key),
                                        *ConvertError(
                                             ANSI_TO_TCHAR(__FUNCTION__),
                                             TEXT("QueryUserInfo call failed."),
                                             QueryResult.ResultCode)
                                             .ToLogString());
                                }

                                // Regardless of success, ask the user info to materialize all of the attributes.
                                Value.Value->MaterializeAttributes(Value.Key);

                                // Regardless of success, we finalise this user even if we couldn't attach Epic
                                // Games account information to it.
                                auto &TargetUser = this->UsersCached.FindOrAdd(Value.Key);
                                if (!TargetUser.IsValid())
                                {
                                    TargetUser = MakeShared<FUsersCache::FCacheEntry>(Value.Key);
                                }
                                TargetUser->Error = nullptr;
                                TargetUser->Value->SetUserInfoData(Value.Value);

                                // Indicate that our query is done.
                                OnDone(true);
                            }));
                    return true;
                },
                [this, InOnDone](TArray<bool> OutValues) {
                    // We're done. Queue more processing if needed and return.
                    UE_LOG(
                        LogRedpointEOSUserCache,
                        Verbose,
                        TEXT("ProcessQueue: Processing is complete after Epic Games query, flushing pending callbacks "
                             "and scheduling more processes if needed."));
                    this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
                    InOnDone.ExecuteIfBound();
                },
                false);
        }
        else
        {
            UE_LOG(LogRedpointEOSUserCache, Verbose, TEXT("ProcessQueue: There are no Epic Games accounts to query."));

            // We're done. Queue more processing if needed and return.
            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("ProcessQueue: Processing is complete after external mappings query, flushing pending callbacks "
                     "and scheduling more processes if needed."));
            this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
            InOnDone.ExecuteIfBound();
            return;
        }
    }
}

void FUserCache::EpicAccountIdsByDisplayNameQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::API::UserInfo;
    using namespace ::Redpoint::EOS::Core::Id;
    
    // Attempt to figure out what the local Epic Games account is for the local user.
    TOptional<EOS_EpicAccountId> LocalEpicAccountId;
    if (!IsDedicatedServerId(this->LocalUserId))
    {
        EOS_EResult CopyEpicIdResult;
        FCopyProductUserExternalAccountByAccountType::Result CopyEpicId;
        FCopyProductUserExternalAccountByAccountType::Execute(
            this->PlatformHandle,
            FCopyProductUserExternalAccountByAccountType::Options{
                GetProductUserId(this->LocalUserId),
                EOS_EExternalAccountType::EOS_EAT_EPIC},
            CopyEpicIdResult,
            CopyEpicId);
        if (CopyEpicIdResult == EOS_EResult::EOS_Success)
        {
            using namespace ::Redpoint::EOS::API::Auth;

            TArray<EOS_EpicAccountId> ExistingUsersResult;
            EOS_EResult ExistingUsersResultCode;
            FGetLoggedInAccounts::Execute(PlatformHandle, ExistingUsersResultCode, ExistingUsersResult);
            for (const auto &EpicAccountId : ExistingUsersResult)
            {
                if (GetEpicGamesUserIdString(EpicAccountId) == CopyEpicId.AccountId)
                {
                    LocalEpicAccountId = EpicAccountId;
                    break;
                }
            }
        }
    }
    if (!LocalEpicAccountId.IsSet())
    {
        for (const auto &KV : this->EpicAccountIdsByDisplayNameCached)
        {
            if (!KV.Value->Error.IsValid())
            {
                KV.Value->Error = MakeShared<FError>(Errors::AccessDenied(
                    ANSI_TO_TCHAR(__FUNCTION__),
                    TEXT("You can not call this API because you are not signed into an Epic Games account locally.")));
            }
        }
        this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
        InOnDone.ExecuteIfBound();
        return;
    }

    // Get the display names to query.
    TArray<FEpicAccountIdsByDisplayNameCache::FCachePair> Entries;
    for (const auto &KV : this->EpicAccountIdsByDisplayNameCached)
    {
        if (!KV.Value->Error.IsValid() && !KV.Value->Value.IsSet())
        {
            Entries.Add(KV);
        }
    }

    // Query all of the display names in parallel.
    FMultiOperation<FEpicAccountIdsByDisplayNameCache::FCachePair, bool>::RunSP(
        this->AsShared(),
        Entries,
        [this, LocalEpicAccountId](
            FEpicAccountIdsByDisplayNameCache::FCachePair Entry,
            TFunction<void(bool OutValue)> OnDone) -> bool {
            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("ProcessQueue: Querying Epic Games account display name '%s' from local Epic Games account "
                     "'%s'..."),
                *Entry.Key,
                *GetEpicGamesUserIdString(LocalEpicAccountId.GetValue()));
            EOS_EpicAccountId LocalEpicAccountIdValue = LocalEpicAccountId.GetValue();
            FQueryUserInfoByDisplayName::Execute(
                this->PlatformHandle,
                FQueryUserInfoByDisplayName::Options{LocalEpicAccountIdValue, Entry.Key},
                FQueryUserInfoByDisplayName::CompletionDelegate::CreateSPLambda(
                    this,
                    [LocalEpicAccountId, Entry, OnDone](const FQueryUserInfoByDisplayName::Result &QueryResult) {
                        UE_LOG(
                            LogRedpointEOSUserCache,
                            Verbose,
                            TEXT("ProcessQueue: Received Epic Games account display name query result for display name "
                                 "'%s' from local Epic Games account '%s'..."),
                            *Entry.Key,
                            *GetEpicGamesUserIdString(LocalEpicAccountId.GetValue()));
                        if (QueryResult.ResultCode != EOS_EResult::EOS_Success)
                        {
                            Entry.Value->Error = MakeShared<FError>(ConvertError(
                                ANSI_TO_TCHAR(__FUNCTION__),
                                TEXT("Unable to query Epic Games account by display name."),
                                QueryResult.ResultCode));
                        }
                        else
                        {
                            Entry.Value->Value = GetEpicGamesUserIdString(QueryResult.TargetUserId);
                        }
                        OnDone(true);
                        return;
                    }));
            return true;
        },
        [this, Entries, InOnDone](TArray<bool> OutValues) {
            for (const auto &KV : Entries)
            {
                if (!KV.Value->Error.IsValid() && !KV.Value->Value.IsSet())
                {
                    KV.Value->Error = MakeShared<FError>(
                        Errors::NotFound(ANSI_TO_TCHAR(__FUNCTION__), TEXT("This user account was not queried.")));
                }
            }
            UE_LOG(
                LogRedpointEOSUserCache,
                Verbose,
                TEXT("ProcessQueue: Query batch of Epic Games account display names is complete."));
            this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
            InOnDone.ExecuteIfBound();
        },
        false);
}

void FUserCache::UserIdsByExternalAccountIdQueue_Process(const FExclusiveAsyncOperationQueue::FOnDone &InOnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Core::Id;
    
    // Of the external mappings to query, we can only query one type at a time, so pick the first type we find and then
    // only add mappings that match.
    TArray<FUserIdsByExternalAccountIdCache::FCachePair> Entries;
    for (const auto &KV : this->UserIdsByExternalAccountIdCached)
    {
        if (!KV.Value->Error.IsValid() && !KV.Value->Value.IsSet())
        {
            if (Entries.Num() == 0 || Entries[0].Key.Key == KV.Key.Key)
            {
                Entries.Add(KV);
            }
        }
    }

    // Query the mappings in batches.
    FMultiOperation<FUserIdsByExternalAccountIdCache::FCachePair, bool>::RunBatchedSP(
        this->AsShared(),
        Entries,
        EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_MAX_ACCOUNT_IDS,
        [this](
            TArray<FUserIdsByExternalAccountIdCache::FCachePair> Batch,
            TFunction<void(TArray<bool> OutValue)> OnDone) -> bool {
            TArray<FString> AccountIds;
            for (const auto &Entry : Batch)
            {
                AccountIds.Add(Entry.Key.Value);
            }
            FQueryExternalAccountMappings::Execute(
                this->PlatformHandle,
                FQueryExternalAccountMappings::Options{this->GetLocalUserIdParameter(),
                    Batch[0].Key.Key,
                    AccountIds},
                FQueryExternalAccountMappings::CompletionDelegate::CreateSPLambda(
                    this,
                    [this, Batch, OnDone](const FQueryExternalAccountMappings::Result &QueryResult) {
                        if (QueryResult.ResultCode != EOS_EResult::EOS_Success &&
                            QueryResult.ResultCode != EOS_EResult::EOS_PartialResult)
                        {
                            for (const auto &Entry : Batch)
                            {
                                Entry.Value->Error = MakeShared<FError>(ConvertError(
                                    ANSI_TO_TCHAR(__FUNCTION__),
                                    TEXT("Unable to query external account mappings."),
                                    QueryResult.ResultCode));
                            }
                        }
                        else
                        {
                            for (const auto &Entry : Batch)
                            {
                                auto ProductUserId = FGetExternalAccountMapping::Execute(
                                    this->PlatformHandle,
                                    FGetExternalAccountMapping::Options{
                                        this->GetLocalUserIdParameter(),
                                        Batch[0].Key.Key,
                                        Entry.Key.Value});
                                auto AccountId = GetAccountId(ProductUserId);
                                if (IsValidId(AccountId))
                                {
                                    Entry.Value->Value = AccountId;
                                }
                                else
                                {
                                    Entry.Value->Error = MakeShared<FError>(Errors::NotFound(
                                        ANSI_TO_TCHAR(__FUNCTION__),
                                        TEXT("There is no user mapped to this external account ID.")));
                                }
                            }
                        }
                        // @note: Just a defaulted array to satisfy the contract of RunBatchedSP.
                        TArray<bool> BoolResults;
                        for (int i = 0; i < Batch.Num(); i++)
                        {
                            BoolResults.Add(true);
                        }
                        OnDone(BoolResults);
                    }));
            return true;
        },
        [this, InOnDone, Entries](TArray<bool> OutValues) {
            for (const auto &KV : Entries)
            {
                if (!KV.Value->Error.IsValid() && !KV.Value->Value.IsSet())
                {
                    KV.Value->Error = MakeShared<FError>(
                        Errors::NotFound(ANSI_TO_TCHAR(__FUNCTION__), TEXT("This user account was not queried.")));
                }
            }
            this->Queue_FlushPendingCallbacksAndPotentiallySchedule();
            InOnDone.ExecuteIfBound();
        },
        false);
}

}

REDPOINT_EOS_CODE_GUARD_END()