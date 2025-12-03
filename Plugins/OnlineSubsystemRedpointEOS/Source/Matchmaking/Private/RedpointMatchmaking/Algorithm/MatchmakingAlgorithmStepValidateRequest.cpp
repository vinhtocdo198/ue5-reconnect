// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepValidateRequest.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepLockParty.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepWaitForPartyLeader.h"
#include "RedpointMatchmaking/MatchmakingErrors.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateValidateRequest> FMatchmakingAlgorithmStepValidateRequest::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateValidateRequest>();
}

void FMatchmakingAlgorithmStepValidateRequest::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    if (!Task->Request.Identity.IsValid())
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingIdentityImplementation());
        return;
    }
    if (!Task->Request.Lobby.IsValid())
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingLobbyImplementation());
        return;
    }

    if (!Task->Request.UserId.IsValid())
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingUserId());
        return;
    }

    if (!Task->PinAndContinue().IsValid())
    {
        this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingRequiredInterfaces());
        return;
    }

    // Reset this in case we went through abandon restart.
    Task->bResponsibleForLobbyDeletionStep = false;

    if (Task->Request.HostConfiguration.IsValid())
    {
        if (Task->Request.HostConfiguration->TeamCapacities.Num() == 0)
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::AtLeastOneTeamRequired());
            return;
        }

        if (!Task->Request.HostConfiguration->SkillStatPrefix.IsEmpty() && !Task->Request.Stats.IsValid())
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingStatsImplementation());
            return;
        }

        // Operating as a host, lock the party.
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_LOCK_PARTY);
    }
    else
    {
        if (!Task->Request.PartySystem.IsValid())
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingPartyImplementationOnClient());
            return;
        }
        if (!Task->Request.PartyId.IsValid())
        {
            this->PermanentError(*Task, OnlineMatchmaking::Errors::MissingPartyIdOnClient());
            return;
        }

        // Operating as a client, wait for the party leader to indicate we're matchmaking.
        EMIT_TASK_COUNTER_TRACE(Task);
        this->MoveToStep(*Task, MMSTEP_WAIT_FOR_PARTY_LEADER);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()