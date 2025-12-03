// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1006528723, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation;

class FOnlinePurchaseInterfaceRedpointSteam
    : public IOnlinePurchase,
      public TSharedFromThis<FOnlinePurchaseInterfaceRedpointSteam, ESPMode::ThreadSafe>
{
private:
    TSharedRef<FSteamOperation> SteamOperation;

public:
    FOnlinePurchaseInterfaceRedpointSteam(TSharedRef<FSteamOperation> InSteamOperation);
    UE_NONCOPYABLE(FOnlinePurchaseInterfaceRedpointSteam);
    virtual ~FOnlinePurchaseInterfaceRedpointSteam() override = default;

    virtual bool IsAllowedToPurchase(const FUniqueNetId &UserId) override;
    virtual void Checkout(
        const FUniqueNetId &UserId,
        const FPurchaseCheckoutRequest &CheckoutRequest,
        const FOnPurchaseCheckoutComplete &Delegate) override;
#if REDPOINT_EOS_UE_5_1_OR_LATER
    virtual void Checkout(
        const FUniqueNetId &UserId,
        const FPurchaseCheckoutRequest &CheckoutRequest,
        const FOnPurchaseReceiptlessCheckoutComplete &Delegate) override;
#endif
    virtual void FinalizePurchase(const FUniqueNetId &UserId, const FString &ReceiptId) override;
    virtual void RedeemCode(
        const FUniqueNetId &UserId,
        const FRedeemCodeRequest &RedeemCodeRequest,
        const FOnPurchaseRedeemCodeComplete &Delegate) override;
    virtual void QueryReceipts(
        const FUniqueNetId &UserId,
        bool bRestoreReceipts,
        const FOnQueryReceiptsComplete &Delegate) override;
    virtual void GetReceipts(const FUniqueNetId &UserId, TArray<FPurchaseReceipt> &OutReceipts) const override;
    virtual void FinalizeReceiptValidationInfo(
        const FUniqueNetId &UserId,
        FString &InReceiptValidationInfo,
        const FOnFinalizeReceiptValidationInfoComplete &Delegate) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1006528723,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem,
    FOnlinePurchaseInterfaceRedpointSteam)
}

#endif