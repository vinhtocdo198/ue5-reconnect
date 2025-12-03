// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "DedicatedServerMatchmakingBeaconHostObject.h"
#include "OnlineBeaconClient.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "DedicatedServerMatchmakingBeaconClientMatchMember.h"
#endif

#include "DedicatedServerMatchmakingBeaconClient.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

#if !REDPOINT_EOS_IS_FREE_EDITION

DECLARE_DELEGATE(FDedicatedServerMatchmakingBeaconClientGenericEvent);
DECLARE_DELEGATE_OneParam(FDedicatedServerMatchmakingBeaconClientBooleanEvent, bool /* bWasSuccessful */);

#endif

UCLASS(Transient, NotPlaceable)
class MATCHMAKING_API ADedicatedServerMatchmakingBeaconClient : public AOnlineBeaconClient
{
    GENERATED_BODY()

    friend class ADedicatedServerMatchmakingBeaconHostObject;

private:
    UPROPERTY()
    TObjectPtr<ADedicatedServerMatchmakingBeaconHostObject> HostStateOnServer;

public:
#if !REDPOINT_EOS_IS_FREE_EDITION
    FDedicatedServerMatchmakingBeaconClientGenericEvent OnConnectedDelegate;
    FDedicatedServerMatchmakingBeaconClientGenericEvent OnFailureDelegate;
    FDedicatedServerMatchmakingBeaconClientBooleanEvent OnReserveForMatchResultDelegate;

    virtual void OnConnected() override;
    virtual void OnFailure() override;
#endif

    UFUNCTION(Server, Reliable)
    virtual void ReserveForMatch(
        const TArray<FDedicatedServerMatchmakingBeaconClientMatchMember> &MatchMembers,
        const TArray<int32> &TeamSizes);

    UFUNCTION(Client, Reliable)
    virtual void OnReserveForMatchResult(bool bWasSuccessful);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()