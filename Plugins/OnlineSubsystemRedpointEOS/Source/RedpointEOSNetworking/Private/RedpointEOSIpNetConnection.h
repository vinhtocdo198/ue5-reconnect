// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IpConnection.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/ObjectMacros.h"

#include "RedpointEOSIpNetConnection.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(transient, config = OnlineSubsystemRedpointEOS)
class URedpointEOSIpNetConnection : public UIpConnection
{
    GENERATED_BODY()

public:
    virtual void InitBase(
        UNetDriver *InDriver,
        FSocket *InSocket,
        const FURL &InURL,
        EConnectionState InState,
        int32 InMaxPacket = 0,
        int32 InPacketOverhead = 0) override;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()