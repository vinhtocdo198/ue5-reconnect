// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CONNECTION_INFORMATION_SENT_FOR_LISTEN_SERVER FName("ConnectionInformationSentForListenServer")

class FMatchmakingAlgorithmStepStateConnectionInformationSentForListenServer : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepConnectionInformationSentForListenServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateConnectionInformationSentForListenServer>,
      public TSharedFromThis<FMatchmakingAlgorithmStepConnectionInformationSentForListenServer>
{
public:
    FMatchmakingAlgorithmStepConnectionInformationSentForListenServer() = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepConnectionInformationSentForListenServer);
    virtual ~FMatchmakingAlgorithmStepConnectionInformationSentForListenServer() override = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CONNECTION_INFORMATION_SENT_FOR_LISTEN_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ConnectionInformationSentForListenServer_Status", "Starting the server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "ConnectionInformationSentForListenServer_Detail",
            "Starting the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateConnectionInformationSentForListenServer> CreateNewState()
        override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()