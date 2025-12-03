// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/Algorithm/MatchmakingAlgorithmStep.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

#define MMSTEP_CONNECTION_INFORMATION_SENT_FOR_DEDICATED_SERVER FName("ConnectionInformationSentForDedicatedServer")

class FMatchmakingAlgorithmStepStateConnectionInformationSentForDedicatedServer : public IMatchmakingAlgorithmStepState
{
};

class FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer
    : public IMatchmakingAlgorithmStep<FMatchmakingAlgorithmStepStateConnectionInformationSentForDedicatedServer>,
      public TSharedFromThis<FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer>
{
public:
    FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer() = default;
    UE_NONCOPYABLE(FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer);
    virtual ~FMatchmakingAlgorithmStepConnectionInformationSentForDedicatedServer() override = default;

    virtual FName GetName() const override
    {
        return MMSTEP_CONNECTION_INFORMATION_SENT_FOR_DEDICATED_SERVER;
    }

    virtual FText GetStatus() const override
    {
        return NSLOCTEXT("Matchmaking", "ConnectionInformationSentForDedicatedServer_Status", "Finding a server");
    }

    virtual FText GetDetail() const override
    {
        return NSLOCTEXT(
            "Matchmaking",
            "ConnectionInformationSentForDedicatedServer_Detail",
            "Reserving the game server...");
    }

    virtual TSharedPtr<FMatchmakingAlgorithmStepStateConnectionInformationSentForDedicatedServer> CreateNewState()
        override;
    virtual void Start(TSharedRef<FMatchmakingEngineTask> Task) override;
};

#endif

REDPOINT_EOS_CODE_GUARD_END()