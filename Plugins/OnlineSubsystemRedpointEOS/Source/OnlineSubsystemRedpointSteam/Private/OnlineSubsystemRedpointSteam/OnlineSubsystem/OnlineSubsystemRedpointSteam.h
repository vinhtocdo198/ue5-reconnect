// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemImplBase.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/Constants.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/ForwardDecls.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)
#include "OnlineSubsystemSteam.h"
#endif

#if defined(REDPOINT_EOS_STEAM_ENABLED)

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(
    2897439011,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation,
    FSteamOperation)

namespace REDPOINT_EOS_FILE_NS_ID(3906369459, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation;

class FOnlineSubsystemRedpointSteam : public FOnlineSubsystemImplBase,
                                      public TSharedFromThis<FOnlineSubsystemRedpointSteam, ESPMode::ThreadSafe>
{
private:
    TSharedPtr<FSteamOperation> OperationImpl;
    TSharedPtr<FOnlineAvatarInterfaceRedpointSteam, ESPMode::ThreadSafe> AvatarImpl;
    TSharedPtr<FOnlineEntitlementsInterfaceRedpointSteam, ESPMode::ThreadSafe> EntitlementsImpl;
    TSharedPtr<FOnlineStoreInterfaceV2RedpointSteam, ESPMode::ThreadSafe> StoreV2Impl;
    TSharedPtr<FOnlinePurchaseInterfaceRedpointSteam, ESPMode::ThreadSafe> PurchaseImpl;
    FString WebApiKey;

public:
    UE_NONCOPYABLE(FOnlineSubsystemRedpointSteam);
    FOnlineSubsystemRedpointSteam() = delete;
    FOnlineSubsystemRedpointSteam(FName InInstanceName);
    ~FOnlineSubsystemRedpointSteam();

    virtual bool IsEnabled() const override;

    virtual IOnlineEntitlementsPtr GetEntitlementsInterface() const override;
    virtual IOnlineStoreV2Ptr GetStoreV2Interface() const override;
    virtual IOnlinePurchasePtr GetPurchaseInterface() const override;

    // Our custom interfaces. Note that even those these APIs return a UObject*, we return
    // non-UObjects that are TSharedFromThis.
    virtual class UObject *GetNamedInterface(FName InterfaceName) override;
    virtual void SetNamedInterface(FName InterfaceName, class UObject *NewInterface) override
    {
        checkf(false, TEXT("FOnlineSubsystemEOS::SetNamedInterface is not supported"));
    };

    virtual bool Init() override;
    virtual bool Shutdown() override;

    virtual FString GetAppId() const override;
    virtual FText GetOnlineServiceName() const override;

    FString GetWebApiKey() const;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3906369459,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem,
    FOnlineSubsystemRedpointSteam)
}

#endif