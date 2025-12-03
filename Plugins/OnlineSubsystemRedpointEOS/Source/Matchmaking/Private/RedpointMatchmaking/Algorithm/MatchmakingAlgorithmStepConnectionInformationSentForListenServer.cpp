// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForListenServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepCompleteMatchFireEvent.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepDeleteMatchmakingLobby.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateConnectionInformationSentForListenServer>
FMatchmakingAlgorithmStepConnectionInformationSentForListenServer::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateConnectionInformationSentForListenServer>();
}

void FMatchmakingAlgorithmStepConnectionInformationSentForListenServer::Start(TSharedRef<FMatchmakingEngineTask> Task)
{
    // The match is complete. Client players will now join the server.
    UE_LOG(
        LogMatchmaking,
        Verbose,
        TEXT("%sMatchmaking complete after starting up listen server"),
        *this->GetLogPrefix(*Task));
    this->MoveToStep(*Task, MMSTEP_DELETE_MATCHMAKING_LOBBY, MMSTEP_COMPLETE_MATCH_FIRE_EVENT);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()