// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1721094416, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{

class FOnlineAsyncTaskSteamPurchase : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
private:
    bool bInit;
    FPurchaseCheckoutRequest CheckoutRequest;
    FOnPurchaseCheckoutComplete Delegate;
    SteamAPICall_t CallbackHandle;
    SteamInventoryStartPurchaseResult_t CallbackResults;
    FString FailureContext;
    uint64 OrderId;
    uint64 TransactionId;

public:
    FOnlineAsyncTaskSteamPurchase(
        const FPurchaseCheckoutRequest &InCheckoutRequest,
        const FOnPurchaseCheckoutComplete &InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamPurchase);
    virtual ~FOnlineAsyncTaskSteamPurchase() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamPurchase");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1721094416,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamPurchase)
}

#endif