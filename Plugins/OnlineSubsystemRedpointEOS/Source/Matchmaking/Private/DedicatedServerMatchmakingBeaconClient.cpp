// Copyright June Rhodes. All Rights Reserved.

#include "DedicatedServerMatchmakingBeaconClient.h"

#include "RedpointMatchmaking/MatchmakingLog.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Engine/NetConnection.h"
#elif WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void ADedicatedServerMatchmakingBeaconClient::OnConnected()
{
    this->OnConnectedDelegate.ExecuteIfBound();
    Super::OnConnected();
}

void ADedicatedServerMatchmakingBeaconClient::OnFailure()
{
    this->OnFailureDelegate.ExecuteIfBound();
    Super::OnFailure();
}

#endif

void ADedicatedServerMatchmakingBeaconClient::ReserveForMatch_Implementation(
    const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
    const TArray<int32> &TeamSizes)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    if (!IsValid(this->HostStateOnServer))
    {
        UE_LOG(LogMatchmaking, Error, TEXT("Reservation request failed: Host state missing on server"));
        this->OnReserveForMatchResult(false);
        return;
    }

    this->OnReserveForMatchResult(
        this->HostStateOnServer->ReserveForMatch(this->GetNetConnection()->PlayerId, MatchMembers, TeamSizes));
#elif WITH_EDITOR
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
}

void ADedicatedServerMatchmakingBeaconClient::OnReserveForMatchResult_Implementation(bool bWasSuccessful)
{
#if !REDPOINT_EOS_IS_FREE_EDITION
    this->OnReserveForMatchResultDelegate.ExecuteIfBound(bWasSuccessful);
#elif WITH_EDITOR
    UE_LOG(
        LogMatchmaking,
        Error,
        TEXT("Team-based matchmaking is not available in the Free Edition. Please upgrade to the paid edition on Fab "
             "to use this feature."));
    ::Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Matchmaking"),
        TEXT("MatchmakingNotAvailableFreeEdition"));
#endif
}

REDPOINT_EOS_CODE_GUARD_END()