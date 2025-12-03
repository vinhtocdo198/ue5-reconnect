// Copyright June Rhodes. All Rights Reserved.

#include "./DedicatedServerMatchmakingBeaconHostObject.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "./DedicatedServerMatchmakingBeaconClient.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

void ADedicatedServerMatchmakingBeaconHostObject::Init(ADedicatedServerMatchmakingBeaconHost *InHostOwner)
{
    this->HostOwner = InHostOwner;
    this->BeaconTypeName = FString("DedicatedServerMatchmakingBeaconClient");
    this->ClientBeaconActorClass = ADedicatedServerMatchmakingBeaconClient::StaticClass();
}

void ADedicatedServerMatchmakingBeaconHostObject::OnClientConnected(
    class AOnlineBeaconClient *NewClientActor,
    class UNetConnection *ClientConnection)
{
    Cast<ADedicatedServerMatchmakingBeaconClient>(NewClientActor)->HostStateOnServer = this;

    Super::OnClientConnected(NewClientActor, ClientConnection);
}

AOnlineBeaconClient *ADedicatedServerMatchmakingBeaconHostObject::SpawnBeaconActor(
    class UNetConnection *ClientConnection)
{
    return Super::SpawnBeaconActor(ClientConnection);
}

bool ADedicatedServerMatchmakingBeaconHostObject::ReserveForMatch(
    const FUniqueNetIdRepl &ClientUserId,
    const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
    const TArray<int32> &TeamSizes)
{
    return this->HostOwner->ReserveForMatch(ClientUserId, MatchMembers, TeamSizes);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()