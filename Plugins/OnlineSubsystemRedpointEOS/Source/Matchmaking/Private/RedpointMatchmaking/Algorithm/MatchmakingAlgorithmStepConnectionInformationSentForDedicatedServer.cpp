// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepJoinServerSession.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateConnectionInformationSentForDedicatedServer>
FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateConnectionInformationSentForDedicatedServer>();
}

void FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer::Start(
    TSharedRef<FMatchmakingEngineTask> Task)
{
    // Other players have all been informed of the connection. Join the server ourselves.
    FString NextStep;
    FString ConnectionInfo;
    Task->EventData[TEXT("dedicated.next_step")].GetValue(NextStep);
    Task->EventData[TEXT("dedicated.connection_info")].GetValue(ConnectionInfo);

    if (NextStep == (MMSTEP_JOIN_SERVER_SESSION).ToString())
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sConnecting to dedicated server via session ID '%s'"),
            *this->GetLogPrefix(*Task),
            *ConnectionInfo);
    }
    else
    {
        UE_LOG(
            LogMatchmaking,
            Verbose,
            TEXT("%sConnecting to dedicated server via IP address '%s'"),
            *this->GetLogPrefix(*Task),
            *ConnectionInfo);
    }

    Task->bResponsibleForLobbyDeletionStep = true;
    this->MoveToStep(*Task, FName(*NextStep), ConnectionInfo);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()