// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationOSSv1/Services/OSSv1RuntimePlatformRelationshipService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Caller/SafeOSSv1Caller.h"
#include "RedpointEOSPlatformIntegrationOSSv1/Logging.h"
#include "RedpointEOSPlatformIntegrationOSSv1/WrappedExternalUserInfo.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2555362757, Redpoint::EOS::Platform::Integration::OSSv1::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1;
using namespace ::Redpoint::EOS::Platform::Integration::OSSv1::Caller;

FOSSv1RuntimePlatformRelationshipService::FOSSv1RuntimePlatformRelationshipService(
    const EOS_EExternalAccountType &InExternalAccountTypeToHandle,
    const FName &InSubsystemName,
    const FResolveUserIdToExternalAccountIdInfo &InExternalInfoResolver,
    const FString &InFriendsListName,
    const FString &InRecentPlayersNamespace)
    : ExternalAccountTypeToHandle(InExternalAccountTypeToHandle)
    , SubsystemName(InSubsystemName)
    , ExternalInfoResolver(InExternalInfoResolver)
    , FriendsListName(InFriendsListName)
    , RecentPlayersNamespace(InRecentPlayersNamespace)
{
}

FString FOSSv1RuntimePlatformRelationshipService::GetCacheKey() const
{
    return this->SubsystemName.ToString().ToLower();
}

bool FOSSv1RuntimePlatformRelationshipService::AllowRelationshipCaching() const
{
    return !this->SubsystemName.IsEqual(FName("PS4")) && !this->SubsystemName.IsEqual(FName("PS5")) &&
           !this->SubsystemName.IsEqual(FName("Sony")) && !this->SubsystemName.IsEqual(FName("Switch")) &&
           !this->SubsystemName.IsEqual(FName("GDK"));
}

FExternalUserInfoRef FOSSv1RuntimePlatformRelationshipService::CreateExternalUserInfo(
    const FOnlineUser &OnlineUser) const
{
    auto LegacyExternalAccountInfo = this->ExternalInfoResolver(*OnlineUser.GetUserId());
    checkf(
        LegacyExternalAccountInfo.IsSet(),
        TEXT("Expected external info resolver to be able to resolve a unique net ID from the intended subsystem."));
    return MakeShared<FWrappedExternalUserInfo>(
        LegacyExternalAccountInfo->AccountIdType,
        LegacyExternalAccountInfo->AccountId,
        OnlineUser.GetDisplayName());
}

void FOSSv1RuntimePlatformRelationshipService::OnChangesFromOSS(
    const FName &InstanceName,
    const FUniqueNetId &LocalPlatformUserId)
{
    auto World = FWorldResolution::GetWorld(InstanceName, true);
    if (!World.IsValid())
    {
        return;
    }

    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    if (!IdentityInterface.IsValid() || !FriendsInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Received friends event from subsystem '%s', but could not resolve subsystem APIs in event callback."),
            *this->SubsystemName.ToString());
        return;
    }

    auto PlatformHandle = IInstancePool::Pool().TryGetExisting(InstanceName);
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Received friends event from subsystem '%s', but there is no EOS platform instance to route the event "
                 "to."),
            *this->SubsystemName.ToString());
        return;
    }

    for (int LocalUserNum = 0; LocalUserNum < MAX_LOCAL_PLAYERS; LocalUserNum++)
    {
        auto CandidatePlayerId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
        if (CandidatePlayerId.IsValid() && *CandidatePlayerId == LocalPlatformUserId)
        {
            this->OnChangesFromOSS(
                PlatformHandle.ToSharedRef(),
                IdentityInterface.ToSharedRef(),
                FriendsInterface.ToSharedRef(),
                LocalPlatformUserId,
                LocalUserNum);
            return;
        }
    }

    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        Warning,
        TEXT("Received friends event from subsystem '%s', but the local player ID could not be resolved back to a "
             "local user number."),
        *this->SubsystemName.ToString());
}

void FOSSv1RuntimePlatformRelationshipService::OnChangesFromOSS(const FName &InstanceName, int32 LocalUserNum)
{
    auto World = FWorldResolution::GetWorld(InstanceName, true);
    if (!World.IsValid())
    {
        return;
    }

    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    if (!IdentityInterface.IsValid() || !FriendsInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Received friends event from subsystem '%s', but could not resolve subsystem APIs in event callback."),
            *this->SubsystemName.ToString());
        return;
    }

    auto PlatformHandle = IInstancePool::Pool().TryGetExisting(InstanceName);
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Received friends event from subsystem '%s', but there is no EOS platform instance to route the event "
                 "to."),
            *this->SubsystemName.ToString());
        return;
    }

    auto LocalPlatformUserId = IdentityInterface->GetUniquePlayerId(LocalUserNum);
    if (!LocalPlatformUserId.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            Warning,
            TEXT("Received friends event from subsystem '%s', but the local player ID could not be resolved back to a "
                 "local user number."),
            *this->SubsystemName.ToString());
        return;
    }

    this->OnChangesFromOSS(
        PlatformHandle.ToSharedRef(),
        IdentityInterface.ToSharedRef(),
        FriendsInterface.ToSharedRef(),
        *LocalPlatformUserId,
        LocalUserNum);
}

void FOSSv1RuntimePlatformRelationshipService::OnChangesFromOSS(
    const FPlatformHandle &PlatformHandle,
    const TSharedRef<IOnlineIdentity> &IdentityInterface,
    const TSharedRef<IOnlineFriends> &FriendsInterface,
    const FUniqueNetId &LocalPlatformUserId,
    int32 LocalUserNum)
{
    TArray<TSharedRef<FOnlineFriend>> Friends;
    TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
    TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
    FSafeOSSv1Caller::GetFriendsList(FriendsInterface, LocalUserNum, this->FriendsListName, Friends);
    if (this->CanQueryRecentPlayers())
    {
        FSafeOSSv1Caller::GetRecentPlayers(
            FriendsInterface,
            LocalPlatformUserId,
            this->RecentPlayersNamespace,
            RecentPlayers);
    }
    FSafeOSSv1Caller::GetBlockedPlayers(FriendsInterface, LocalPlatformUserId, BlockedPlayers);

    auto &Storage = this->PlatformStorage[PlatformHandle];
    auto &CurrentRelationships = Storage.CachedRelationships.FindOrAdd(LocalUserNum);

    auto CurrentMutualFriends = CurrentRelationships.FindOrAdd(ERelationshipType::MutualFriends);
    auto CurrentIncomingFriendRequest = CurrentRelationships.FindOrAdd(ERelationshipType::IncomingFriendRequest);
    auto CurrentOutgoingFriendRequest = CurrentRelationships.FindOrAdd(ERelationshipType::OutgoingFriendRequest);
    auto CurrentBlockedList = CurrentRelationships.FindOrAdd(ERelationshipType::Blocked);
    auto CurrentRecentPlayerList = CurrentRelationships.FindOrAdd(ERelationshipType::RecentPlayer);

    auto &UpdatedMutualFriends = CurrentRelationships.FindOrAdd(ERelationshipType::MutualFriends);
    auto &UpdatedIncomingFriendRequest = CurrentRelationships.FindOrAdd(ERelationshipType::IncomingFriendRequest);
    auto &UpdatedOutgoingFriendRequest = CurrentRelationships.FindOrAdd(ERelationshipType::OutgoingFriendRequest);
    auto &UpdatedBlockedList = CurrentRelationships.FindOrAdd(ERelationshipType::Blocked);
    auto &UpdatedRecentPlayerList = CurrentRelationships.FindOrAdd(ERelationshipType::RecentPlayer);

    TMap<ERelationshipType, TArray<FExternalUserInfoRef>> AddedRelationships;
    TMap<ERelationshipType, TArray<FExternalUserInfoRef>> RemovedRelationships;

    ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> NewMutualFriends;
    ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> NewIncomingFriendRequest;
    ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> NewOutgoingFriendRequest;
    ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> NewBlockedList;
    ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> NewRecentPlayerList;
    for (const auto &Friend : Friends)
    {
        switch (Friend->GetInviteStatus())
        {
        case EInviteStatus::Accepted:
            NewMutualFriends.Add(*Friend->GetUserId(), Friend);
            break;
        case EInviteStatus::PendingInbound:
            NewIncomingFriendRequest.Add(*Friend->GetUserId(), Friend);
            break;
        case EInviteStatus::PendingOutbound:
            NewOutgoingFriendRequest.Add(*Friend->GetUserId(), Friend);
            break;
        case EInviteStatus::Blocked:
            NewBlockedList.Add(*Friend->GetUserId(), Friend);
            break;
        case EInviteStatus::Unknown:
        case EInviteStatus::Suggested:
        default:
            break;
        }
    }
    for (const auto &RecentPlayer : RecentPlayers)
    {
        NewRecentPlayerList.Add(*RecentPlayer->GetUserId(), RecentPlayer);
    }
    for (const auto &Blocked : BlockedPlayers)
    {
        NewBlockedList.Add(*Blocked->GetUserId(), Blocked);
    }

    auto Differencing = [this, &RemovedRelationships, &AddedRelationships](
                            ERelationshipType RelationshipType,
                            const ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<FExternalUserInfoPtr> &Current,
                            const ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<TSharedPtr<FOnlineUser>> &New,
                            ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<FExternalUserInfoPtr> &Update) {
        for (const auto &KV : Current)
        {
            if (!New.Contains(*KV.Key))
            {
                FExternalUserInfoPtr Ptr;
                if (Update.RemoveAndCopyValue(KV.Key, Ptr))
                {
                    RemovedRelationships.FindOrAdd(ERelationshipType::MutualFriends).Add(Ptr.ToSharedRef());
                }
            }
        }
        for (const auto &KV : New)
        {
            if (!Current.Contains(*KV.Key))
            {
                auto Ref = this->CreateExternalUserInfo(*KV.Value);
                AddedRelationships.FindOrAdd(ERelationshipType::MutualFriends).Add(Ref);
                Update.Add(*KV.Key, Ref);
            }
        }
    };
    Differencing(ERelationshipType::MutualFriends, CurrentMutualFriends, NewMutualFriends, UpdatedMutualFriends);
    Differencing(
        ERelationshipType::IncomingFriendRequest,
        CurrentIncomingFriendRequest,
        NewIncomingFriendRequest,
        UpdatedIncomingFriendRequest);
    Differencing(
        ERelationshipType::OutgoingFriendRequest,
        CurrentOutgoingFriendRequest,
        NewOutgoingFriendRequest,
        UpdatedOutgoingFriendRequest);
    Differencing(ERelationshipType::Blocked, CurrentBlockedList, NewBlockedList, UpdatedBlockedList);
    Differencing(
        ERelationshipType::RecentPlayer,
        CurrentRecentPlayerList,
        NewRecentPlayerList,
        UpdatedRecentPlayerList);

    Storage.OnRelationshipsChangedDelegate.FindOrAdd(LocalUserNum).Broadcast(AddedRelationships, RemovedRelationships);
}

void FOSSv1RuntimePlatformRelationshipService::GetRelationships(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    ERelationshipType RelationshipTypes,
    const FOnGetRelationshipComplete &OnComplete)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    auto *PlayerRelationships = Storage.CachedRelationships.Find(CallContext->LocalUserNum);
    if (PlayerRelationships == nullptr)
    {
        OnComplete.ExecuteIfBound(Errors::Success(), TMap<ERelationshipType, TArray<FExternalUserInfoRef>>());
        return;
    }

    TMap<ERelationshipType, TArray<FExternalUserInfoRef>> RelationshipsByType;
    auto RelationshipTypeValues = TArray<ERelationshipType>{
        ERelationshipType::MutualFriends,
        ERelationshipType::IncomingFriendRequest,
        ERelationshipType::OutgoingFriendRequest,
        ERelationshipType::Blocked,
        ERelationshipType::RecentPlayer};
    for (const auto &RelationshipTypeValue : RelationshipTypeValues)
    {
        if ((RelationshipTypes & RelationshipTypeValue) != ERelationshipType::None)
        {
            auto &Results = RelationshipsByType.FindOrAdd(RelationshipTypeValue);
            for (const auto &KV : PlayerRelationships->FindOrAdd(RelationshipTypeValue))
            {
                if (KV.Value.IsValid())
                {
                    Results.Add(KV.Value.ToSharedRef());
                }
            }
        }
    }

    OnComplete.ExecuteIfBound(Errors::Success(), RelationshipsByType);
    return;
}

IRuntimePlatformRelationshipService::FOnRelationshipsChanged &FOSSv1RuntimePlatformRelationshipService::
    OnRelationshipsChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];
    return Storage.OnRelationshipsChangedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

void FOSSv1RuntimePlatformRelationshipService::RegisterEvents(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        VeryVerbose,
        TEXT("Registering friends events and caching relationships for subsystem '%s'."),
        *this->SubsystemName.ToString());

    // Get storage for this platform handle.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    // Get the relevant interfaces.
    TSharedPtr<IOnlineIdentity> IdentityInterface = nullptr;
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!IdentityInterface.IsValid())
            {
                IdentityInterface = OSSInstance->GetIdentityInterface();
            }
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    // If we don't have a friends implementation, this subsystem doesn't support friends.
    if (!FriendsInterface.IsValid())
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationOSSv1,
            VeryVerbose,
            TEXT("The subsystem '%s' does not support friends."),
            *this->SubsystemName.ToString());
        Storage.bGlobalEventsRegistered = true;
        OnComplete.ExecuteIfBound();
        return;
    }

    // Perform the initial read; we assume that we never need to call ReadFriendsList again after we
    // do it the first time, as OSS implementations are expected to keep their lists up-to-date when
    // they fire events.
    auto InstanceName = CallContext->PlatformHandle->GetEnvironment()->InstanceName;
    auto Graph = MakeShared<FAsyncTaskGraph>();

    // If we have not registered global events on this friends implementation, register them.
    FAsyncTaskPtr PreviousTask;
    if (!Storage.bGlobalEventsRegistered)
    {
        PreviousTask = Graph->AddDelegate(
            FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                this,
                [this, CallContext, InstanceName, FriendsInterface, &Storage](
                    const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                    Storage.OnFriendRemovedDelegateHandle = FriendsInterface->AddOnFriendRemovedDelegate_Handle(
                        FOnFriendRemovedDelegate::CreateSPLambda(
                            this,
                            [this, InstanceName](const FUniqueNetId &UserId, const FUniqueNetId &FriendId) {
                                this->OnChangesFromOSS(InstanceName, UserId);
                            }));
                    Storage.OnInviteAcceptedDelegateHandle = FriendsInterface->AddOnInviteAcceptedDelegate_Handle(
                        FOnInviteAcceptedDelegate::CreateSPLambda(
                            this,
                            [this, InstanceName](const FUniqueNetId &UserId, const FUniqueNetId &FriendId) {
                                this->OnChangesFromOSS(InstanceName, UserId);
                            }));
                    Storage.OnInviteRejectedDelegateHandle = FriendsInterface->AddOnInviteRejectedDelegate_Handle(
                        FOnInviteRejectedDelegate::CreateSPLambda(
                            this,
                            [this, InstanceName](const FUniqueNetId &UserId, const FUniqueNetId &FriendId) {
                                this->OnChangesFromOSS(InstanceName, UserId);
                            }));
                    Storage.OnInviteReceivedDelegateHandle = FriendsInterface->AddOnInviteReceivedDelegate_Handle(
                        FOnInviteReceivedDelegate::CreateSPLambda(
                            this,
                            [this, InstanceName](const FUniqueNetId &UserId, const FUniqueNetId &FriendId) {
                                this->OnChangesFromOSS(InstanceName, UserId);
                            }));
                    Storage.OnRecentPlayersAddedDelegateHandle =
                        FriendsInterface->AddOnRecentPlayersAddedDelegate_Handle(
                            FOnRecentPlayersAddedDelegate::CreateSPLambda(
                                this,
                                [this, InstanceName](
                                    const FUniqueNetId &UserId,
                                    const TArray<TSharedRef<FOnlineRecentPlayer>> &AddedPlayers) {
                                    this->OnChangesFromOSS(InstanceName, UserId);
                                }));
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                }),
            FAsyncTaskGraph::ENoDependencies::StartImmediately);
        Storage.bGlobalEventsRegistered = true;
    }

    // Query the user's friends.
    PreviousTask = Graph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, CallContext, FriendsInterface](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                FSafeOSSv1Caller::ReadFriendsList(
                    FriendsInterface.ToSharedRef(),
                    CallContext->LocalUserNum,
                    this->FriendsListName,
                    FOnReadFriendsListComplete::CreateSPLambda(
                        this,
                        [this, OnTaskComplete](int32, bool bWasSuccessful, const FString &, const FString &ErrorStr) {
                            if (!bWasSuccessful)
                            {
                                UE_LOG(
                                    LogRedpointEOSPlatformIntegrationOSSv1,
                                    VeryVerbose,
                                    TEXT("The subsystem '%s' failed to perform the initial cache of friends: %s"),
                                    *this->SubsystemName.ToString(),
                                    *ErrorStr);
                            }
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PreviousTask);

    // Query the user's blocked players.
    PreviousTask = Graph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, CallContext, IdentityInterface, FriendsInterface](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                auto LocalPlayerId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
                if (!LocalPlayerId.IsValid())
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                    return;
                }

                FSafeOSSv1Caller::QueryBlockedPlayers(
                    FriendsInterface.ToSharedRef(),
                    *LocalPlayerId,
                    FOnQueryBlockedPlayersCompleteDelegate::CreateSPLambda(
                        this,
                        [this, OnTaskComplete](const FUniqueNetId &, bool bWasSuccessful, const FString &Error) {
                            if (!bWasSuccessful)
                            {
                                UE_LOG(
                                    LogRedpointEOSPlatformIntegrationOSSv1,
                                    VeryVerbose,
                                    TEXT("The subsystem '%s' failed to perform the initial cache of blocked players: "
                                         "%s"),
                                    *this->SubsystemName.ToString(),
                                    *Error);
                            }
                            OnTaskComplete.ExecuteIfBound(Errors::Success());
                        }));
            }),
        PreviousTask);

    // Query the user's recent players.
    if (this->CanQueryRecentPlayers())
    {
        PreviousTask = Graph->AddDelegate(
            FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                this,
                [this, CallContext, IdentityInterface, FriendsInterface](
                    const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                    auto LocalPlayerId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
                    if (!LocalPlayerId.IsValid())
                    {
                        OnTaskComplete.ExecuteIfBound(Errors::Success());
                        return;
                    }

                    FSafeOSSv1Caller::QueryRecentPlayers(
                        FriendsInterface.ToSharedRef(),
                        *LocalPlayerId,
                        this->RecentPlayersNamespace,
                        FOnQueryRecentPlayersCompleteDelegate::CreateSPLambda(
                            this,
                            [this, OnTaskComplete](
                                const FUniqueNetId &,
                                const FString &,
                                bool bWasSuccessful,
                                const FString &Error) {
                                if (!bWasSuccessful)
                                {
                                    UE_LOG(
                                        LogRedpointEOSPlatformIntegrationOSSv1,
                                        VeryVerbose,
                                        TEXT("The subsystem '%s' failed to perform the initial cache of recent "
                                             "players: %s"),
                                        *this->SubsystemName.ToString(),
                                        *Error);
                                }
                                OnTaskComplete.ExecuteIfBound(Errors::Success());
                            }));
                }),
            PreviousTask);
    }

    // Build the initial relationship cache.
    PreviousTask = Graph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, CallContext, IdentityInterface, FriendsInterface, &Storage](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                auto LocalPlayerId = IdentityInterface->GetUniquePlayerId(CallContext->LocalUserNum);
                if (!LocalPlayerId.IsValid())
                {
                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                    return;
                }

                TArray<TSharedRef<FOnlineFriend>> Friends;
                TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
                TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                FSafeOSSv1Caller::GetFriendsList(
                    FriendsInterface.ToSharedRef(),
                    CallContext->LocalUserNum,
                    this->FriendsListName,
                    Friends);
                if (this->CanQueryRecentPlayers())
                {
                    FSafeOSSv1Caller::GetRecentPlayers(
                        FriendsInterface.ToSharedRef(),
                        *LocalPlayerId,
                        this->RecentPlayersNamespace,
                        RecentPlayers);
                }
                FSafeOSSv1Caller::GetBlockedPlayers(FriendsInterface.ToSharedRef(), *LocalPlayerId, BlockedPlayers);

                auto &CachedRelationships = Storage.CachedRelationships.FindOrAdd(CallContext->LocalUserNum);

                ::Redpoint::EOS::Core::Id::TUniqueNetIdMap<FExternalUserInfoPtr> MutualFriends, IncomingFriendRequest,
                    OutgoingFriendRequest, BlockedList, RecentPlayerList;

                for (const auto &Friend : Friends)
                {
                    switch (Friend->GetInviteStatus())
                    {
                    case EInviteStatus::Accepted:
                        MutualFriends.Add(*Friend->GetUserId(), this->CreateExternalUserInfo(*Friend));
                        break;
                    case EInviteStatus::PendingInbound:
                        IncomingFriendRequest.Add(*Friend->GetUserId(), this->CreateExternalUserInfo(*Friend));
                        break;
                    case EInviteStatus::PendingOutbound:
                        OutgoingFriendRequest.Add(*Friend->GetUserId(), this->CreateExternalUserInfo(*Friend));
                        break;
                    case EInviteStatus::Blocked:
                        BlockedList.Add(*Friend->GetUserId(), this->CreateExternalUserInfo(*Friend));
                        break;
                    case EInviteStatus::Unknown:
                    case EInviteStatus::Suggested:
                    default:
                        break;
                    }
                }
                for (const auto &RecentPlayer : RecentPlayers)
                {
                    RecentPlayerList.Add(*RecentPlayer->GetUserId(), this->CreateExternalUserInfo(*RecentPlayer));
                }
                for (const auto &Blocked : BlockedPlayers)
                {
                    BlockedList.Add(*Blocked->GetUserId(), this->CreateExternalUserInfo(*Blocked));
                }

                CachedRelationships.Add(ERelationshipType::MutualFriends, MutualFriends);
                CachedRelationships.Add(ERelationshipType::IncomingFriendRequest, IncomingFriendRequest);
                CachedRelationships.Add(ERelationshipType::OutgoingFriendRequest, OutgoingFriendRequest);
                CachedRelationships.Add(ERelationshipType::Blocked, BlockedList);
                CachedRelationships.Add(ERelationshipType::RecentPlayer, RecentPlayerList);

                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PreviousTask);

    // Register event handlers for the local user.
    PreviousTask = Graph->AddDelegate(
        FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
            this,
            [this, CallContext, InstanceName, FriendsInterface, &Storage](
                const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                Storage.OnFriendsChangeDelegateHandle.Add(
                    CallContext->LocalUserNum,
                    FriendsInterface->AddOnFriendsChangeDelegate_Handle(
                        CallContext->LocalUserNum,
                        FOnFriendsChangeDelegate::CreateSP(
                            this,
                            &FOSSv1RuntimePlatformRelationshipService::OnOSSv1FriendsChange,
                            InstanceName,
                            CallContext->LocalUserNum)));
                Storage.OnOutgoingInviteSentDelegateHandle.Add(
                    CallContext->LocalUserNum,
                    FriendsInterface->AddOnOutgoingInviteSentDelegate_Handle(
                        CallContext->LocalUserNum,
                        FOnOutgoingInviteSentDelegate::CreateSP(
                            this,
                            &FOSSv1RuntimePlatformRelationshipService::OnOSSv1OutgoingInviteSent,
                            InstanceName,
                            CallContext->LocalUserNum)));
                Storage.OnBlockListChangeDelegateHandle.Add(
                    CallContext->LocalUserNum,
                    FriendsInterface->AddOnBlockListChangeDelegate_Handle(
                        CallContext->LocalUserNum,
                        FOnBlockListChangeDelegate::CreateSP(
                            this,
                            &FOSSv1RuntimePlatformRelationshipService::OnOSSv1BlockListChange,
                            InstanceName,
                            CallContext->LocalUserNum)));
                OnTaskComplete.ExecuteIfBound(Errors::Success());
            }),
        PreviousTask);

    // Add final callback and execute.
    Graph->FinallyDelegate(
        FAsyncTaskGraph::FOnExecuteFinally::CreateSPLambda(this, [OnComplete](const FError &Error) {
            OnComplete.ExecuteIfBound();
        }));
    Graph->Start();
}

void FOSSv1RuntimePlatformRelationshipService::UnregisterEvents(
    const FRuntimePlatformServiceCallContextRef &CallContext)
{
    UE_LOG(
        LogRedpointEOSPlatformIntegrationOSSv1,
        VeryVerbose,
        TEXT("Unregistering friends events for subsystem '%s'."),
        *this->SubsystemName.ToString());

    // Get storage for this platform handle.
    auto &Storage = this->PlatformStorage[CallContext->PlatformHandle];

    // Get the relevant interfaces.
    TSharedPtr<IOnlineFriends> FriendsInterface = nullptr;
    {
        auto *OSSInstance = Online::GetSubsystem(CallContext->World.Get(), this->SubsystemName);
        if (OSSInstance != nullptr)
        {
            if (!FriendsInterface.IsValid())
            {
                FriendsInterface = OSSInstance->GetFriendsInterface();
            }
        }
    }

    // Remove user-specific event handlers.
    {
        auto *Handle = Storage.OnFriendsChangeDelegateHandle.Find(CallContext->LocalUserNum);
        if (Handle != nullptr)
        {
            FriendsInterface->ClearOnFriendsChangeDelegate_Handle(CallContext->LocalUserNum, *Handle);
            Storage.OnFriendsChangeDelegateHandle.Remove(CallContext->LocalUserNum);
        }
    }
    {
        auto *Handle = Storage.OnOutgoingInviteSentDelegateHandle.Find(CallContext->LocalUserNum);
        if (Handle != nullptr)
        {
            FriendsInterface->ClearOnOutgoingInviteSentDelegate_Handle(CallContext->LocalUserNum, *Handle);
            Storage.OnOutgoingInviteSentDelegateHandle.Remove(CallContext->LocalUserNum);
        }
    }
    {
        auto *Handle = Storage.OnBlockListChangeDelegateHandle.Find(CallContext->LocalUserNum);
        if (Handle != nullptr)
        {
            FriendsInterface->ClearOnBlockListChangeDelegate_Handle(CallContext->LocalUserNum, *Handle);
            Storage.OnBlockListChangeDelegateHandle.Remove(CallContext->LocalUserNum);
        }
    }
}

bool FOSSv1RuntimePlatformRelationshipService::CanQueryRecentPlayers() const
{
    return true;
}

void FOSSv1RuntimePlatformRelationshipService::OnOSSv1FriendsChange(FName InstanceName, int32 LocalUserNum)
{
    this->OnChangesFromOSS(InstanceName, LocalUserNum);
}

void FOSSv1RuntimePlatformRelationshipService::OnOSSv1OutgoingInviteSent(FName InstanceName, int32 LocalUserNum)
{
    this->OnChangesFromOSS(InstanceName, LocalUserNum);
}

void FOSSv1RuntimePlatformRelationshipService::OnOSSv1BlockListChange(
    int32,
    const FString &,
    FName InstanceName,
    int32 LocalUserNum)
{
    this->OnChangesFromOSS(InstanceName, LocalUserNum);
}

}; // namespace REDPOINT_EOS_FILE_NS_ID(2555362757,Redpoint::EOS::Platform::Integration::OSSv1::Services)

#endif

REDPOINT_EOS_CODE_GUARD_END()