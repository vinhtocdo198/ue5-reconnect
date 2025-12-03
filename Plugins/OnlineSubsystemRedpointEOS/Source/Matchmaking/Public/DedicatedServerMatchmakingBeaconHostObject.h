// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "DedicatedServerMatchmakingBeaconHost.h"
#include "OnlineBeaconHostObject.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "OnlineBeaconClient.h"
#endif

#include "DedicatedServerMatchmakingBeaconHostObject.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(Transient, NotPlaceable)
class MATCHMAKING_API ADedicatedServerMatchmakingBeaconHostObject : public AOnlineBeaconHostObject
{
    GENERATED_BODY()

private:
    UPROPERTY()
    TObjectPtr<ADedicatedServerMatchmakingBeaconHost> HostOwner;

public:
#if !REDPOINT_EOS_IS_FREE_EDITION
    void Init(class ADedicatedServerMatchmakingBeaconHost *InHostOwner);

    virtual void OnClientConnected(class AOnlineBeaconClient *NewClientActor, class UNetConnection *ClientConnection)
        override;
    virtual AOnlineBeaconClient *SpawnBeaconActor(class UNetConnection *ClientConnection) override;

    bool ReserveForMatch(
        const FUniqueNetIdRepl &ClientUserId,
        const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
        const TArray<int32> &TeamSizes);
#endif
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()