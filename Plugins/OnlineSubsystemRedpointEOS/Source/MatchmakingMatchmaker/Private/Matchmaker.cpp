// Copyright June Rhodes. All Rights Reserved.

#include "Matchmaker.h"

#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "MatchmakerSubsystem.h"
#include "OnlineSubsystemUtils.h"
#if WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif
#include "RedpointMatchmaking/Utility/ParseTeamCapacities.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

AMatchmaker::AMatchmaker()
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    this->Identity.Reset();
    this->Lobby.Reset();
    this->PartySystem.Reset();
    this->bIsUsable = false;
    this->bIsMatchmaking.Reset();
    this->LastMatchmakingProgressData.Reset();
#endif
}

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<const FUniqueNetId> AMatchmaker::ResolveOptionalUserId(
    const FUniqueNetIdRepl &UserIdRepl,
    const FString &FunctionName)
{
    TSharedPtr<const FUniqueNetId> UserId;
    if (!UserIdRepl.IsValid())
    {
        UWorld *World = this->GetWorld();
        if (!IsValid(World))
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,
                TEXT("Can not call AMatchmaker::%s; unable to find the current world."),
                *FunctionName);
            return nullptr;
        }
        APlayerController *PC = World->GetFirstPlayerController();
        if (!IsValid(PC))
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,

                TEXT("Can not call AMatchmaker::%s; unable to find the first player controller."),
                *FunctionName);
            return nullptr;
        }
        ULocalPlayer *LP = PC->GetLocalPlayer();
        if (!IsValid(LP))
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,

                TEXT("Can not call AMatchmaker::%s; the first player controller isn't a local player."),
                *FunctionName);
            return nullptr;
        }
        FUniqueNetIdRepl NetId = LP->GetPreferredUniqueNetId();
        if (!NetId.IsValid())
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,
                TEXT("Can not call AMatchmaker::%s; the first player controller isn't signed into the "
                     "online subsystem."),
                *FunctionName);
            return nullptr;
        }

        UserId = NetId.GetUniqueNetId();
    }
    else
    {
        UserId = UserIdRepl.GetUniqueNetId();
    }

    return UserId;
}

bool AMatchmaker::IsPartyPrimaryParty(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    auto PS = this->PartySystem.Pin();
    if (PS.IsValid())
    {
        auto Party = PS->GetParty(LocalUserId, PartyId);
        if (Party.IsValid())
        {
            return Party->PartyTypeId == IOnlinePartySystem::GetPrimaryPartyTypeId();
        }
    }
    return false;
}

void AMatchmaker::SetMemberReadinessStatus(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    bool bIsReady)
{
    if (!bIsReady)
    {
        this->bLocallyForcedNotReady.Add(LocalUserId, true);
    }

    auto PS = this->PartySystem.Pin();

    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("SetMemberReadinessStatus: Member %s is setting their ready status to '%s'."),
        *LocalUserId.ToString(),
        bIsReady ? TEXT("Ready") : TEXT("NotReady"));

    auto ExistingMemberData = PS->GetPartyMemberData(LocalUserId, PartyId, LocalUserId, DefaultPartyDataNamespace);
    if (!ExistingMemberData.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Warning,
            TEXT("SetMemberReadinessStatus: Unable to update member readiness because we couldn't get the member's "
                 "party data (likely because they left the party)."));
        return;
    }

    auto MemberData = MakeShared<FOnlinePartyData>(*ExistingMemberData);
    MemberData->SetAttribute(TEXT("ready"), bIsReady);

    if (!PS->UpdatePartyMemberData(LocalUserId, PartyId, DefaultPartyDataNamespace, *MemberData))
    {
        this->bLocallyForcedNotReady.Remove(LocalUserId);
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Unable to update ready status!"));
    }
}

void AMatchmaker::OnNativePartyJoined(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    if (!this->IsPartyPrimaryParty(LocalUserId, PartyId))
    {
        return;
    }

    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativePartyJoined: Member %s joined party %s. Evaluating party readiness."),
        *LocalUserId.ToString(),
        *PartyId.ToString());

    this->EvaluatePartyReadiness(LocalUserId, PartyId);
}

void AMatchmaker::OnNativePartyExited(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
}

void AMatchmaker::OnNativePartyMemberExited(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const EMemberExitedReason Reason)
{
    if (!this->IsPartyPrimaryParty(LocalUserId, PartyId))
    {
        return;
    }

    if (this->PartySystem.Pin()->IsMemberLeader(LocalUserId, PartyId, LocalUserId))
    {
        if (Reason == EMemberExitedReason::Left)
        {
            // The party member left (they weren't kicked or removed by us). Un-ready the
            // party leader so we don't automatically start queuing if someone from the party
            // accidentally leaves.
            UE_LOG(
                LogMatchmakingMatchmaker,
                Verbose,
                TEXT("OnNativePartyMemberExited: Member %s exited party %s. The host is automatically unreadying."),
                *LocalUserId.ToString(),
                *PartyId.ToString());
            this->SetMemberReadinessStatus(LocalUserId, PartyId, false);
            return;
        }
    }

    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativePartyMemberExited: Member %s exited party %s. Evaluating party readiness."),
        *LocalUserId.ToString(),
        *PartyId.ToString());
    this->EvaluatePartyReadiness(LocalUserId, PartyId);
}

void AMatchmaker::OnNativePartyMemberJoined(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId)
{
    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativePartyMemberJoined: Member %s joined party %s. Evaluating party readiness."),
        *LocalUserId.ToString(),
        *PartyId.ToString());
    this->EvaluatePartyReadiness(LocalUserId, PartyId);
}

void AMatchmaker::OnNativePartyMemberPromoted(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &NewLeaderId)
{
    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativePartyMemberJoined: Member %s in party %s was notified that %s is promoted to party leader. "
             "Evaluating party readiness."),
        *LocalUserId.ToString(),
        *PartyId.ToString(),
        *NewLeaderId.ToString());
    this->EvaluatePartyReadiness(LocalUserId, PartyId);
}

void AMatchmaker::OnNativePartyMemberDataReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &MemberId,
    const FName &Namespace,
    const FOnlinePartyData &PartyMemberData)
{
    if (this->bLocallyForcedNotReady.Contains(MemberId))
    {
        this->bLocallyForcedNotReady.Remove(MemberId);
    }

    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativePartyMemberDataReceived: Member %s in party %s updated their party data. Evaluating party "
             "readiness."),
        *LocalUserId.ToString(),
        *PartyId.ToString());
    this->EvaluatePartyReadiness(LocalUserId, PartyId);
}

void AMatchmaker::StartClientMatchmaking(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    checkf(
        !this->RequestsByLocalUser.Contains(LocalUserId),
        TEXT("Can't start client matchmaking if there is already a request in progress for this user!"));

    UMatchmakerSubsystem *MatchmakerSubsystem =
        this->GetWorld()->GetGameInstance<UGameInstance>()->GetSubsystem<UMatchmakerSubsystem>();

    FMatchmakingEngineRequest Request = {};
    Request.Identity = this->Identity;
    Request.Lobby = this->Lobby;
    Request.PartySystem = this->PartySystem;
    Request.Session = this->Session;
    Request.Stats = this->Stats;
    Request.WorldContextHandle = GEngine->GetWorldContextFromWorldChecked(this->GetWorld()).ContextHandle;
    Request.UserId = LocalUserId.DoesSharedInstanceExist() && LocalUserId.IsValid() ? LocalUserId.AsShared()
                                                                                    : this->GetWorld()
                                                                                          ->GetFirstPlayerController()
                                                                                          ->GetLocalPlayer()
                                                                                          ->GetPreferredUniqueNetId()
                                                                                          .GetUniqueNetId();
    Request.PartyId = PartyId.AsShared();
    Request.HostConfiguration = nullptr;
    Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap;
    Request.CompletionBehaviour.OnResultsReady = FMatchmakingEngineRequestComplete::CreateUObject(
        MatchmakerSubsystem,
        &UMatchmakerSubsystem::OnNativeMatchmakingResultsReady);
    Request.CompletionBehaviour.OnComplete = FMatchmakingEngineRequestComplete::CreateUObject(
        MatchmakerSubsystem,
        &UMatchmakerSubsystem::OnNativeMatchmakingComplete);
    Request.CompletionBehaviour.bConnectViaSessionID = this->bConnectViaSessionID;
    Request.CompletionBehaviour.MapName = this->GetMapName();
    Request.OnProgress =
        FMatchmakingEngineRequestProgress::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingProgress);
    Request.OnCancelled =
        FMatchmakingEngineRequestCancelled::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingCancelled);
    Request.OnError = FMatchmakingEngineRequestError::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingError);
    this->RequestsByLocalUser.Add(LocalUserId, this->MatchmakingEngine->Enqueue(Request));
}

void AMatchmaker::StopClientMatchmaking(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    checkf(
        this->RequestsByLocalUser.Contains(LocalUserId),
        TEXT("Expected client matchmaking to already be in progress when calling StopClientMatchmaking!"));
    this->MatchmakingEngine->Cancel(this->RequestsByLocalUser[LocalUserId]);
    this->RequestsByLocalUser.Remove(LocalUserId);
}

void AMatchmaker::StartHostMatchmaking(const FUniqueNetId &LocalUserId, const TSharedPtr<const FOnlinePartyId> &PartyId)
{
    using namespace ::Redpoint::Matchmaking::Utility;

#if WITH_EDITOR
    // Try to make the request ID extra unique when running in the editor.
    int64 RequestId = (FDateTime::UtcNow().GetTicks() / 100000) * 100000;
    if (GEngine != nullptr && IsValid(this->GetWorld()))
    {
        FWorldContext &WorldContext = GEngine->GetWorldContextFromWorldChecked(this->GetWorld());
        RequestId += ((int64)WorldContext.PIEInstance % 100) * 1000;
    }
    RequestId += FMath::RandRange(0, 999);
#else
    int64 RequestId = FDateTime::UtcNow().GetTicks();
#endif

    if (!this->HostConfiguration.bUseDedicatedServers && this->HostConfiguration.Map.IsNull())
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Can't start matchmaking because the configured map is invalid!"));
        return;
    }

    auto ParsedTeamCapacities = ParseTeamCapacities(this->HostConfiguration.TeamCapacities);
    if (!ParsedTeamCapacities.IsSet())
    {
        return;
    }
    TArray<int32> GeneratedTeamCapacities = ParsedTeamCapacities.GetValue();

    UMatchmakerSubsystem *MatchmakerSubsystem =
        this->GetWorld()->GetGameInstance<UGameInstance>()->GetSubsystem<UMatchmakerSubsystem>();

    FMatchmakingEngineRequest Request = {};
    Request.Identity = this->Identity;
    Request.Lobby = this->Lobby;
    Request.PartySystem = this->PartySystem;
    Request.Session = this->Session;
    Request.Stats = this->Stats;
    Request.WorldContextHandle = GEngine->GetWorldContextFromWorldChecked(this->GetWorld()).ContextHandle;
    Request.UserId = LocalUserId.AsShared();
    Request.PartyId = PartyId;
    auto RequestHostConfiguration = MakeShared<FMatchmakingEngineRequestHostConfiguration>();
    RequestHostConfiguration->RequestId = RequestId;
    RequestHostConfiguration->QueueName =
        this->HostConfiguration.QueueName.IsEmpty() ? TEXT("Default") : this->HostConfiguration.QueueName;
    RequestHostConfiguration->TeamCapacities = GeneratedTeamCapacities;
    if (this->HostConfiguration.bAllowPartiallyFilledMatches)
    {
        RequestHostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::CompletePartiallyFilled;
        RequestHostConfiguration->BalanceMode = this->HostConfiguration.bPrioritizeBalance
                                                    ? EMatchmakingBalanceMode::MaximizeBalance
                                                    : EMatchmakingBalanceMode::MaximizeTeamFill;
        RequestHostConfiguration->MinimumTeamMemberCountForPartialFill =
            this->HostConfiguration.MinimumTeamMemberCountForPartialFill;
        RequestHostConfiguration->MinimumWaitSecondsBaseline = this->HostConfiguration.NoCandidatesTimeout;
        RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot =
            this->HostConfiguration.NoCandidatesTimeoutPerEmptySlot;
        if (RequestHostConfiguration->MinimumWaitSecondsBaseline < 15)
        {
            RequestHostConfiguration->MinimumWaitSecondsBaseline = 15;
        }
        if (RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot < 0)
        {
            RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = 0;
        }
    }
    else
    {
        RequestHostConfiguration->OnNoCandidates = EMatchmakingBehaviourOnNoCandidates::WaitUntilFull;
        RequestHostConfiguration->BalanceMode = EMatchmakingBalanceMode::MaximizeTeamFill;
        // These settings don't actually do anything in WaitUntilFull mode.
        RequestHostConfiguration->MinimumWaitSecondsBaseline = 60;
        RequestHostConfiguration->MinimumWaitSecondsPerEmptySlot = 5;
    }
    RequestHostConfiguration->SkillStatPrefix = this->HostConfiguration.SkillStatPrefix;
    RequestHostConfiguration->SkillThresholdFunction.Unbind(); // Leave default.
    Request.HostConfiguration = RequestHostConfiguration;
    if (this->HostConfiguration.bUseDedicatedServers)
    {
        Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::FindDedicatedServer;
    }
    else
    {
        Request.CompletionBehaviour.Type = EMatchmakingEngineCompletionBehaviourType::StartListenServerWithMap;
    }
    Request.CompletionBehaviour.OnResultsReady = FMatchmakingEngineRequestComplete::CreateUObject(
        MatchmakerSubsystem,
        &UMatchmakerSubsystem::OnNativeMatchmakingResultsReady);
    Request.CompletionBehaviour.OnComplete = FMatchmakingEngineRequestComplete::CreateUObject(
        MatchmakerSubsystem,
        &UMatchmakerSubsystem::OnNativeMatchmakingComplete);
    Request.CompletionBehaviour.bConnectViaSessionID = this->bConnectViaSessionID;
    if (this->HostConfiguration.bUseDedicatedServers)
    {
        Request.CompletionBehaviour.DedicatedServerBeaconPort = this->HostConfiguration.BeaconPort;
        Request.CompletionBehaviour.OnGetDedicatedServerSearchParams =
            FMatchmakingEngineGetDedicatedServerSearchParams::CreateUObject(
                this,
                &AMatchmaker::OnNativeMatchmakingGetDedicatedServerSearchParams);
    }
    else
    {
        Request.CompletionBehaviour.MapName = this->GetMapName();
    }
    Request.OnProgress =
        FMatchmakingEngineRequestProgress::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingProgress);
    Request.OnCancelled =
        FMatchmakingEngineRequestCancelled::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingCancelled);
    Request.OnError = FMatchmakingEngineRequestError::CreateUObject(this, &AMatchmaker::OnNativeMatchmakingError);
    this->RequestsByLocalUser.Add(LocalUserId, this->MatchmakingEngine->Enqueue(Request));
    this->bIsMatchmaking.Add(LocalUserId, true);
}

bool AMatchmaker::StopHostMatchmaking(const FUniqueNetId &LocalUserId)
{
    if (this->RequestsByLocalUser.Contains(LocalUserId))
    {
        this->MatchmakingEngine->Cancel(this->RequestsByLocalUser[LocalUserId]);
        this->RequestsByLocalUser.Remove(LocalUserId);
    }
    return true;
}

void AMatchmaker::EvaluatePartyReadiness(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
{
    auto PS = this->PartySystem.Pin();

    if (!this->IsPartyPrimaryParty(LocalUserId, PartyId))
    {
        // This isn't the primary party.
        return;
    }

    // Check if we need to change between client and host matchmaking modes.
    TSharedRef<const FUniqueNetId> NewLeader = PS->GetParty(LocalUserId, PartyId)->LeaderId.ToSharedRef();
    if (this->LastLeader.IsValid())
    {
        if (*this->LastLeader == LocalUserId && *NewLeader != LocalUserId)
        {
            if (!this->RequestsByLocalUser.Contains(LocalUserId))
            {
                // We were the party leader, but we're no longer the party leader. We now act as a client, which means
                // we start our matchmaking request in the background waiting for the leader to queue us up.
                UE_LOG(
                    LogMatchmakingMatchmaker,
                    Verbose,
                    TEXT("EvaluatePartyReadiness: We are no longer the party leader, starting client matchmaking."));
                this->StartClientMatchmaking(LocalUserId, PartyId);
            }
        }
        else if (*this->LastLeader != LocalUserId && *NewLeader == LocalUserId)
        {
            if (this->RequestsByLocalUser.Contains(LocalUserId))
            {
                // We are now the leader, which means we have to cancel our client matchmaking if it's going.
                UE_LOG(
                    LogMatchmakingMatchmaker,
                    Verbose,
                    TEXT("EvaluatePartyReadiness: We are now the party leader, stopping client matchmaking."));
                this->StopClientMatchmaking(LocalUserId, PartyId);
            }
        }

        this->LastLeader = NewLeader;
    }
    else
    {
        // We are evaluating for the first time. If we are a client, we need to start matchmaking now. If we're a host,
        // we don't need to do anything.
        if (*NewLeader != LocalUserId)
        {
            if (!this->RequestsByLocalUser.Contains(LocalUserId))
            {
                UE_LOG(
                    LogMatchmakingMatchmaker,
                    Verbose,
                    TEXT("EvaluatePartyReadiness: We are evaluating party readiness for the first time, and we are not "
                         "the leader. Starting client matchmaking."));
                this->StartClientMatchmaking(LocalUserId, PartyId);
            }
        }
    }

    if (*NewLeader != LocalUserId)
    {
        // We aren't the leader of the party (and only the party leader decides readiness).
        return;
    }

    bool bIsReady = true;

    TArray<FOnlinePartyMemberConstRef> Members;
    if (!PS->GetPartyMembers(LocalUserId, PartyId, Members))
    {
        UE_LOG(LogMatchmakingMatchmaker, Error, TEXT("Unable to get members from party!"));
        return;
    }

    for (const auto &Member : Members)
    {
        if (this->bLocallyForcedNotReady.Contains(*Member->GetUserId()))
        {
            bIsReady = false;
            break;
        }
        FOnlinePartyDataConstPtr MemberData =
            PS->GetPartyMemberData(LocalUserId, PartyId, *Member->GetUserId(), DefaultPartyDataNamespace);
        if (!MemberData.IsValid())
        {
            bIsReady = false;
            break;
        }
        FVariantData ReadyStatus;
        FString ReadyAttr = TEXT("ready");
        if (!MemberData->GetAttribute(ReadyAttr, ReadyStatus) ||
            ReadyStatus.GetType() != EOnlineKeyValuePairDataType::Bool)
        {
            bIsReady = false;
            break;
        }
        bool bMemberReady;
        ReadyStatus.GetValue(bMemberReady);
        if (!bMemberReady)
        {
            bIsReady = false;
            break;
        }
    }

    if (!bIsReady && this->RequestsByLocalUser.Contains(LocalUserId))
    {
        if (!this->StopHostMatchmaking(LocalUserId))
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Warning,
                TEXT("Matchmaking could not be cancelled in response to party no longer ready."));
        }
    }

    if (bIsReady && !this->RequestsByLocalUser.Contains(LocalUserId))
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Verbose,
            TEXT("EvaluatePartyReadiness: We are the party leader, and all members are ready. Starting host "
                 "matchmaking."));
        this->StartHostMatchmaking(LocalUserId, PartyId.AsShared());
    }
}

void AMatchmaker::OnNativeMatchmakingProgress(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FMatchmakingEngineProgressInfo ProgressInfo)
{
    if (!ProgressInfo.StepName.IsEqual(FName("ValidateRequest")) &&
        !ProgressInfo.StepName.IsEqual(FName("WaitForPartyLeader")))
    {
        this->bIsMatchmaking.Add(*Request.UserId, true);
    }

    FMatchmakerProgressInfo ProgressInfoBp = {};
    ProgressInfoBp.StepName = ProgressInfo.StepName;
    ProgressInfoBp.CurrentStatus = ProgressInfo.CurrentStatus;
    ProgressInfoBp.CurrentDetail = ProgressInfo.CurrentDetail;
    ProgressInfoBp.CurrentProgress = ProgressInfo.CurrentProgress;
    ProgressInfoBp.EstimatedTimeOfCompletion = ProgressInfo.EstimatedTimeOfCompletion;

    this->LastMatchmakingProgressData.Add(*Request.UserId, ProgressInfoBp);
    this->OnProgress(ProgressInfoBp);
}

void AMatchmaker::OnNativeMatchmakingCancelled(const FString &TaskId, const struct FMatchmakingEngineRequest &Request)
{
    UE_LOG(
        LogMatchmakingMatchmaker,
        Verbose,
        TEXT("OnNativeMatchmakingCancelled: Received cancellation event from Matchmaking."));
    this->bIsMatchmaking.Add(*Request.UserId, false);
    this->OnCancelled();

    // If we were still tracking our request, that means cancellation happened for a remote reason, and not
    // because the AMatchmaker actor wanted to cancel. Remove the currently tracked request.
    if (this->RequestsByLocalUser.Contains(*Request.UserId))
    {
        this->RequestsByLocalUser.Remove(*Request.UserId);

        // If this party is still the user's primary party, check for party readiness again so that clients
        // will start their background polling again.
        if (this->IsPartyPrimaryParty(*Request.UserId, *Request.PartyId))
        {
            this->EvaluatePartyReadiness(*Request.UserId, *Request.PartyId);
        }
    }
}

// NOLINTNEXTLINE(matchmaking-online-callback-must-handle-timeouts)
void AMatchmaker::OnNativeMatchmakingError(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    const FOnlineError &Error)
{
    this->bIsMatchmaking.Add(*Request.UserId, false);
    this->SetReadyState(FUniqueNetIdRepl(Request.UserId), false);
    this->OnError(Error.ToLogString());
}

FSearchParams AMatchmaker::OnNativeMatchmakingGetDedicatedServerSearchParams(
    const FString &TaskId,
    const struct FMatchmakingEngineRequest &Request,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FMatchmakingEngineResponse Response,
    int32 SearchIteration)
{
    FSearchParams Params;
    for (const auto &Filter : this->HostConfiguration.SessionFilters)
    {
        EOnlineComparisonOp::Type Comparison = EOnlineComparisonOp::Equals;
        switch (Filter.Comparison)
        {
        case EMatchmakerHostConfigurationAttributeFilterComparison::Equal:
            Comparison = EOnlineComparisonOp::Equals;
            break;
        case EMatchmakerHostConfigurationAttributeFilterComparison::NotEqual:
            Comparison = EOnlineComparisonOp::NotEquals;
            break;
        case EMatchmakerHostConfigurationAttributeFilterComparison::LessThan:
            Comparison = EOnlineComparisonOp::LessThan;
            break;
        case EMatchmakerHostConfigurationAttributeFilterComparison::LessThanOrEqual:
            Comparison = EOnlineComparisonOp::LessThanEquals;
            break;
        case EMatchmakerHostConfigurationAttributeFilterComparison::GreaterThan:
            Comparison = EOnlineComparisonOp::GreaterThan;
            break;
        case EMatchmakerHostConfigurationAttributeFilterComparison::GreaterThanOrEqual:
            Comparison = EOnlineComparisonOp::GreaterThanEquals;
            break;
        default:
            break;
        }
        switch (Filter.AttributeType)
        {
        case EMatchmakerHostConfigurationAttributeFilterType::String:
            Params.Add(
                FName(*Filter.AttributeName),
                FOnlineSessionSearchParam(Filter.AttributeValueString, Comparison));
            break;
        case EMatchmakerHostConfigurationAttributeFilterType::Int64:
            Params.Add(FName(*Filter.AttributeName), FOnlineSessionSearchParam(Filter.AttributeValueInt64, Comparison));
            break;
        case EMatchmakerHostConfigurationAttributeFilterType::Float:
            Params.Add(FName(*Filter.AttributeName), FOnlineSessionSearchParam(Filter.AttributeValueFloat, Comparison));
            break;
        case EMatchmakerHostConfigurationAttributeFilterType::Boolean:
            Params.Add(
                FName(*Filter.AttributeName),
                FOnlineSessionSearchParam(Filter.AttributeValueBoolean, Comparison));
            break;
        default:
            break;
        }
    }
    return Params;
}

FName AMatchmaker::GetMapName()
{
    FString LongPackageName = this->HostConfiguration.Map.GetLongPackageName();
    return FName(*LongPackageName);
}

#endif

void AMatchmaker::BeginPlay()
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (this->GetWorld()->GetNetMode() == NM_DedicatedServer)
    {
        // Destroy this actor if we're running on a dedicated server.
        UE_LOG(
            LogMatchmakingMatchmaker,
            Verbose,
            TEXT("AMatchmaker is not running because this game is running as "
                 "a dedicated server (this->GetWorld()->GetNetMode() == NM_DedicatedServer)."));
        this->bIsUsable = false;
        Super::BeginPlay();
        return;
    }

    this->MatchmakingEngine = IMatchmakingEngine::Get();

    IOnlineSubsystem *OSS = Online::GetSubsystem(this->GetWorld());
    if (OSS == nullptr)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("There is no current online subsystem, which is required for "
                 "matchmaking."));
        this->bIsUsable = false;
        Super::BeginPlay();
        return;
    }
    this->Identity = OSS->GetIdentityInterface();
    this->Lobby = Online::GetLobbyInterface(OSS);
    this->PartySystem = OSS->GetPartyInterface();
    this->Session = OSS->GetSessionInterface();
    this->Stats = OSS->GetStatsInterface();
    this->bConnectViaSessionID = !OSS->GetSubsystemName().IsEqual(STEAM_SUBSYSTEM);

    if (!this->Identity.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The current online subsystem does not provide an identity implementation, which "
                 "is required for "
                 "matchmaking."));
        this->bIsUsable = false;
        Super::BeginPlay();
        return;
    }

    if (!this->Lobby.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("The current online subsystem does not provide a lobby implementation, which is "
                 "required for "
                 "matchmaking."));
        this->bIsUsable = false;
        Super::BeginPlay();
        return;
    }

    this->bIsUsable = true;

    if (!this->PartySystem.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Warning,
            TEXT("The current online subsystem does not provide a party system implementation. You "
                 "will only be able "
                 "to queue solo players."));
    }

    // Register the party events, if required.
    if (auto PS = this->PartySystem.Pin())
    {
        this->PartyJoinedDelegateHandle = PS->AddOnPartyJoinedDelegate_Handle(
            FOnPartyJoined::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyJoined));
        this->PartyExitedDelegateHandle = PS->AddOnPartyExitedDelegate_Handle(
            FOnPartyExited::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyExited));
        this->PartyMemberExitedDelegateHandle = PS->AddOnPartyMemberExitedDelegate_Handle(
            FOnPartyMemberExited::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyMemberExited));
        this->PartyMemberJoinedDelegateHandle = PS->AddOnPartyMemberJoinedDelegate_Handle(
            FOnPartyMemberJoined::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyMemberJoined));
        this->PartyMemberPromotedDelegateHandle = PS->AddOnPartyMemberPromotedDelegate_Handle(
            FOnPartyMemberPromoted::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyMemberPromoted));
        this->PartyMemberDataReceivedDelegateHandle = PS->AddOnPartyMemberDataReceivedDelegate_Handle(
            FOnPartyMemberDataReceived::FDelegate::CreateUObject(this, &AMatchmaker::OnNativePartyMemberDataReceived));
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif

    Super::BeginPlay();
}

void AMatchmaker::SetReadyState(const FUniqueNetIdRepl &UserIdRepl, bool bIsReady)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!this->bIsUsable)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::SetReadyState; the matchmaker is not usable."));
        return;
    }

    TSharedPtr<const FUniqueNetId> UserId = this->ResolveOptionalUserId(UserIdRepl, TEXT("SetReadyState"));
    if (!UserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Ignoring AMatchmaker::SetReadyState because UserId is not valid"));
        return;
    }

    TSharedPtr<const FOnlinePartyId> PartyId = nullptr;
    if (auto PS = this->PartySystem.Pin())
    {
        // Try to get the primary party.
        auto PrimaryParty = PS->GetParty(*UserId, IOnlinePartySystem::GetPrimaryPartyTypeId());
        if (PrimaryParty.IsValid())
        {
            PartyId = PrimaryParty->PartyId;
        }
    }

    if (PartyId.IsValid())
    {
        this->SetMemberReadinessStatus(*UserId, *PartyId, bIsReady);
    }
    else
    {
        // Check if we're cancelling.
        if (!bIsReady)
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Verbose,
                TEXT("SetReadyState: We are a solo player who is cancelling. Stopping host matchmaking."));
            if (!this->StopHostMatchmaking(*UserId))
            {
                UE_LOG(
                    LogMatchmakingMatchmaker,
                    Warning,
                    TEXT("Matchmaking could not be cancelled in response to bIsReady set to "
                         "false."));
            }
            return;
        }

        // Otherwise, we're ready to queue.
        UE_LOG(
            LogMatchmakingMatchmaker,
            Verbose,
            TEXT("SetReadyState: We are a solo player who is ready to queue. Starting host matchmaking."));
        this->StartHostMatchmaking(*UserId, nullptr);
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif
}

void AMatchmaker::GetReadyState(
    const FUniqueNetIdRepl &UserIdRepl,
    const FUniqueNetIdRepl &TargetUserIdRepl,
    bool &bIsReady)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!this->bIsUsable)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::GetReadyState; the matchmaker is not usable."));
        return;
    }

    TSharedPtr<const FUniqueNetId> UserId = this->ResolveOptionalUserId(UserIdRepl, TEXT("GetReadyState"));
    TSharedPtr<const FUniqueNetId> TargetUserId = this->ResolveOptionalUserId(TargetUserIdRepl, TEXT("GetReadyState"));
    if (!UserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::GetReadyState because LocalUserId is not valid."));
        return;
    }
    if (!TargetUserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::GetReadyState because TargetUserId is not valid."));
        return;
    }

    TSharedPtr<const FOnlinePartyId> PartyId = nullptr;
    if (auto PS = this->PartySystem.Pin())
    {
        // Try to get the primary party.
        auto PrimaryParty = PS->GetParty(*UserId, IOnlinePartySystem::GetPrimaryPartyTypeId());
        if (PrimaryParty.IsValid())
        {
            PartyId = PrimaryParty->PartyId;
        }
    }

    if (!PartyId.IsValid())
    {
        if (*TargetUserId != *UserId)
        {
            UE_LOG(
                LogMatchmakingMatchmaker,
                Error,
                TEXT("Called AMatchmaker::GetReadyState for a target user ID that is not in your "
                     "party (because the "
                     "local user ID doesn't have a primary party)."));
            return;
        }

        // We'll be matchmaking as the host, so we're ready if we're matchmaking.
        bIsReady = this->RequestsByLocalUser.Contains(*UserId);
        return;
    }

    if (auto PS = this->PartySystem.Pin())
    {
        const auto &MemberData = PS->GetPartyMemberData(*UserId, *PartyId, *TargetUserId, DefaultPartyDataNamespace);
        if (MemberData.IsValid())
        {
            FVariantData ReadyStatus;
            MemberData->GetAttribute(TEXT("ready"), ReadyStatus);
            if (ReadyStatus.GetType() == EOnlineKeyValuePairDataType::Bool)
            {
                ReadyStatus.GetValue(bIsReady);
                return;
            }
        }
        bIsReady = false;
        return;
    }

    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Can not call AMatchmaker::GetReadyState, party system not available."));
    bIsReady = false;
    return;
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif
}

void AMatchmaker::IsMatchmaking(bool &bOutIsMatchmaking)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    bOutIsMatchmaking = false;

    if (!this->bIsUsable)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::IsMatchmaking; the matchmaker is not usable."));
        return;
    }

    for (const auto &KV : this->bIsMatchmaking)
    {
        if (KV.Value)
        {
            bOutIsMatchmaking = true;
            return;
        }
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
    bOutIsMatchmaking = false;
#endif
}

void AMatchmaker::GetMatchmakingStatus(const FUniqueNetIdRepl &UserIdRepl, FMatchmakerProgressInfo &ProgressInfo)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!this->bIsUsable)
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::GetMatchmakingStatus; the matchmaker is not usable."));
        return;
    }

    TSharedPtr<const FUniqueNetId> UserId = this->ResolveOptionalUserId(UserIdRepl, TEXT("GetMatchmakingStatus"));
    if (!UserId.IsValid())
    {
        UE_LOG(
            LogMatchmakingMatchmaker,
            Error,
            TEXT("Can not call AMatchmaker::GetMatchmakingStatus because LocalUserId is not "
                 "valid."));
        return;
    }

    if (this->bIsMatchmaking.Contains(*UserId) && this->bIsMatchmaking[*UserId] &&
        this->LastMatchmakingProgressData.Contains(*UserId))
    {
        ProgressInfo = this->LastMatchmakingProgressData[*UserId];
    }
    else
    {
        ProgressInfo = FMatchmakerProgressInfo();
        ProgressInfo.StepName = NAME_None;
        ProgressInfo.CurrentStatus = FText::GetEmpty();
        ProgressInfo.CurrentDetail = FText::GetEmpty();
        ProgressInfo.CurrentProgress = 0;
        ProgressInfo.EstimatedTimeOfCompletion = FDateTime::MinValue();
    }
#else
    UE_LOG(
        LogMatchmakingMatchmaker,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
#if WITH_EDITOR
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
#endif
}

REDPOINT_EOS_CODE_GUARD_END()