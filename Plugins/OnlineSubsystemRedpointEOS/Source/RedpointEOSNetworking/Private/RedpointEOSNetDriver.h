// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "IpNetDriver.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSNetworking/ForwardDecls.h"
#include "UObject/ObjectMacros.h"

#include "RedpointEOSNetDriver.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(731365916, Redpoint::EOS::Networking, FRedpointEOSNetworkingModule);

UCLASS(transient, config = OnlineSubsystemRedpointEOS)
class REDPOINTEOSNETWORKING_API URedpointEOSNetDriver : public UIpNetDriver
{
    GENERATED_BODY()

    friend ::Redpoint::EOS::Networking::FNetDriverImpl;
    friend ::Redpoint::EOS::Networking::FNetDriverNullImpl;
    friend ::Redpoint::EOS::Networking::FRedpointEOSNetworkingModule;

private:
    TSharedPtr<::Redpoint::EOS::Networking::INetDriver> Impl;

public:
    virtual bool IsAvailable() const override;
    virtual bool IsNetResourceValid() override;
    virtual ISocketSubsystem *GetSocketSubsystem() override;

    virtual void PostInitProperties() override;
    virtual bool InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error) override;
    virtual bool InitListen(FNetworkNotify *InNotify, FURL &ListenURL, bool bReuseAddressAndPort, FString &Error)
        override;

    virtual void InitConnectionlessHandler() override;

    virtual void TickDispatch(float DeltaTime) override;

    virtual void TickFlush(float DeltaTime) override;

    virtual void LowLevelSend(
        TSharedPtr<const FInternetAddr> Address,
        void *Data,
        int32 CountBits,
        FOutPacketTraits &Traits) override;

#if REDPOINT_EOS_UE_5_4_OR_LATER
    virtual void Shutdown() override;
#endif

    virtual void LowLevelDestroy() override;

    TSharedRef<::Redpoint::EOS::Networking::INetDriver> GetImpl() const;

private:
    bool UIpNetDriver_InitConnect(FNetworkNotify *InNotify, const FURL &ConnectURL, FString &Error);
    bool UIpNetDriver_InitListen(FNetworkNotify *InNotify, FURL &LocalURL, bool bReuseAddressAndPort, FString &Error);
    bool UIpNetDriver_IsNetResourceValid();
    ISocketSubsystem *UIpNetDriver_GetSocketSubsystem();
    void UIpNetDriver_InitConnectionlessHandler();
    void UIpNetDriver_TickDispatch(float DeltaTime);
    void UIpNetDriver_TickFlush(float DeltaTime);
    void UIpNetDriver_LowLevelSend(
        TSharedPtr<const FInternetAddr> Address,
        void *Data,
        int32 CountBits,
        FOutPacketTraits &Traits);
    void UIpNetDriver_LowLevelDestroy();

    bool UNetDriver_InitBase(
        bool bInitAsClient,
        FNetworkNotify *InNotify,
        const FURL &URL,
        bool bReuseAddressAndPort,
        FString &Error);
    void UNetDriver_TickDispatch(float DeltaTime);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()