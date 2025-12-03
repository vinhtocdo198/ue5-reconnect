// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Subsystems/WorldSubsystem.h"
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Containers/Ticker.h"
#include "GameFramework/OnlineReplStructs.h"
#include "GameFramework/PlayerState.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#endif

#include "RedpointEOSRecentPlayersWorldSubsystem.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS()
class URecentPlayersWorldSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

private:
    void OnActorSpawned(AActor *NewActor);
    bool OnDelayedPlayerStartEvaluation(float DeltaSeconds, TSoftObjectPtr<APlayerState> InPlayerState);
    void DiscoveredPlayerId(const FUniqueNetIdRepl &InPlayerId);

public:
    virtual void Initialize(FSubsystemCollectionBase &Collection) override;

#endif
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()
