// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonStart.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyMatch.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepAbandonNotifyParty.h"
#include "RedpointMatchmaking/EventHandlers/MatchmakingEventHandlerAbandonment.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateAbandonStart> FMatchmakingAlgorithmStepAbandonStart::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateAbandonStart>();
}

void FMatchmakingAlgorithmStepAbandonStart::Start(
    TSharedRef<FMatchmakingEngineTask> Task,
    EMatchmakingAbandonmentType AbandonmentType)
{
    if (AbandonmentType == EMatchmakingAbandonmentType::Restart)
    {
        UE_LOG(LogMatchmaking, Verbose, TEXT("%sRestarting matchmaking..."), *this->GetLogPrefix(*Task));
    }

    if (auto Pins = Task->PinAndContinue())
    {
        if (Task->Request.PartyId.IsValid() && Pins->OnlineParty.IsValid())
        {
            TArray<TSharedRef<const FOnlinePartyId>> JoinedParties;
            if (Pins->OnlineParty->GetJoinedParties(*Task->Request.UserId, JoinedParties))
            {
                bool bIsCurrentlyInParty = false;
                for (const auto &PartyId : JoinedParties)
                {
                    if (*PartyId == *Task->Request.PartyId)
                    {
                        bIsCurrentlyInParty = true;
                        break;
                    }
                }
                if (bIsCurrentlyInParty)
                {
                    checkf(
                        Pins->OnlineParty
                            ->GetPartyMember(*Task->Request.UserId, *Task->Request.PartyId, *Task->Request.UserId)
                            .IsValid(),
                        TEXT("GetJoinedParties reported that we were in the party, but GetPartyMember returned "
                             "nullptr. Your party system implementation is broken."));
                    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_PARTY, AbandonmentType);
                    return;
                }
            }
        }
    }

    this->MoveToStep(*Task, MMSTEP_ABANDON_NOTIFY_MATCH, AbandonmentType);
}

bool FMatchmakingAlgorithmStepAbandonStart::CanHandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        return true;
    }

    return false;
}

bool FMatchmakingAlgorithmStepAbandonStart::HandleInterrupt(
    const TSharedRef<FMatchmakingEngineTask> &Task,
    FName InterruptType)
{
    if (InterruptType.IsEqual(MMINTERRUPT_HAS_SPECIAL_HANDLING_OF_ABANDONMENT) ||
        InterruptType.IsEqual(MMINTERRUPT_ABANDON_CANCEL) || InterruptType.IsEqual(MMINTERRUPT_ABANDON_RESTART))
    {
        // We're already abandoning.
        return true;
    }

    return false;
}

#endif

REDPOINT_EOS_CODE_GUARD_END()