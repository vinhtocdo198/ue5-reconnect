// Copyright June Rhodes. All Rights Reserved.

#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineSessionInterface.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepConnectionInformationSentForListenServer.h"
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStepSendSessionIdOrConnectionString.h"
#include "RedpointMatchmaking/MatchmakingConstants.h"
#include "RedpointMatchmaking/MatchmakingLog.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

TSharedPtr<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionStringForListenServer>
FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer::CreateNewState()
{
    return MakeShared<FMatchmakingAlgorithmStepStateSendSessionIdOrConnectionStringForListenServer>();
}

void FMatchmakingAlgorithmStepSendSessionIdOrConnectionStringForListenServer::Start(
    TSharedRef<FMatchmakingEngineTask> Task)
{
    if (auto Pins = Task->PinAndContinue())
    {
        FString ConnectionKey;
        FString ConnectionValue;

        if (Task->Request.CompletionBehaviour.bConnectViaSessionID)
        {
            auto Session = Pins->OnlineSession->GetNamedSession(NAME_GameSession);
            ConnectionKey = MatchmakingMetadataConnectionSessionId;
            ConnectionValue = Session->GetSessionIdStr();
        }
        else
        {
            FString ConnectInfo;
            Pins->OnlineSession->GetResolvedConnectString(NAME_GamePort, ConnectInfo, NAME_GamePort);
            ConnectionKey = MatchmakingMetadataConnectionUrl;
            ConnectionValue = ConnectInfo;
        }

        this->MoveToStep(
            *Task,
            MMSTEP_SEND_SESSION_ID_OR_CONNECTION_STRING,
            MMSTEP_CONNECTION_INFORMATION_SENT_FOR_LISTEN_SERVER,
            ConnectionKey,
            ConnectionValue);
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()