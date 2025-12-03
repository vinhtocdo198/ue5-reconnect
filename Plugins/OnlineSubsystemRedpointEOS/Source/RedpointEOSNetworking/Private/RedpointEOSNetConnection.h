// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/NetConnection.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/ForwardDecls.h"

#include "RedpointEOSNetConnection.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(transient, config = OnlineSubsystemRedpointEOS)
class URedpointEOSNetConnection : public UNetConnection
{
    GENERATED_BODY()

    friend ::Redpoint::EOS::Networking::FNetDriverImpl;
    friend ::Redpoint::EOS::Networking::FNetConnectionImpl;

private:
    TSharedPtr<::Redpoint::EOS::Networking::FNetConnectionImpl> Impl;

public:
    virtual void InitHandler() override;

    virtual void InitBase(
        UNetDriver *InDriver,
        FSocket *InSocket,
        const FURL &InURL,
        EConnectionState InState,
        int32 InMaxPacket = 0,
        int32 InPacketOverhead = 0) override;
    virtual void InitRemoteConnection(
        UNetDriver *InDriver,
        FSocket *InSocket,
        const FURL &InURL,
        const FInternetAddr &InRemoteAddr,
        EConnectionState InState,
        int32 InMaxPacket = 0,
        int32 InPacketOverhead = 0) override;
    virtual void InitLocalConnection(
        UNetDriver *InDriver,
        FSocket *InSocket,
        const FURL &InURL,
        EConnectionState InState,
        int32 InMaxPacket = 0,
        int32 InPacketOverhead = 0) override;

    virtual void LowLevelSend(void *Data, int32 CountBits, FOutPacketTraits &Traits) override;
    virtual FString LowLevelGetRemoteAddress(bool bAppendPort = false) override;
    virtual FString LowLevelDescribe() override;

    TSharedRef<::Redpoint::EOS::Networking::FNetConnectionImpl> GetImpl() const;
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()