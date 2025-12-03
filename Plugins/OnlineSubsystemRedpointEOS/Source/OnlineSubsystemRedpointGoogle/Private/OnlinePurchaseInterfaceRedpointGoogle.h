// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Purchase/OnlinePurchaseWithFinalizePurchaseCallback.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointGooglePurchase.h"
#include "RedpointGooglePurchaseReceipt.h"

#if EOS_GOOGLE_ENABLED

#include "OnlineIdentityInterfaceRedpointGoogle.h"

class FOnlinePurchaseInterfaceRedpointGoogle
    : public Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Purchase::
          IOnlinePurchaseWithFinalizePurchaseCallback,
      public TSharedFromThis<FOnlinePurchaseInterfaceRedpointGoogle, ESPMode::ThreadSafe>
{
public:
    TSharedRef<FOnlineIdentityInterfaceRedpointGoogle> IdentityGooglePlay;

    /**
     * The receipts associated with a 'Checkout' request, where there is no purchase GUID assigned yet by Google Play.
     *
     * The key is a locally generated transaction ID.
     */
    TMap<FString, TSharedPtr<FRedpointGooglePurchaseReceipt>> PendingReceipts;

    /**
     * The receipts associated with a completed purchase, yet to be acknowledged or consumed, where there is a purchase
     * GUID assigned by Google Play.
     *
     * The key is the purchase GUID assigned by Google Play.
     */
    TMap<FString, TSharedPtr<FRedpointGooglePurchaseReceipt>> CompletedReceipts;

    FOnlinePurchaseInterfaceRedpointGoogle(
        const TSharedRef<FOnlineIdentityInterfaceRedpointGoogle> &InIdentityGooglePlay);
    virtual ~FOnlinePurchaseInterfaceRedpointGoogle();
    UE_NONCOPYABLE(FOnlinePurchaseInterfaceRedpointGoogle);

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
    virtual void FinalizePurchase(const FUniqueNetId &UserId, const FString &ReceiptId, const FString &ReceiptInfo)
        override;
    virtual void AcknowledgePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Purchase::
            IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) override;
    virtual void ConsumePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Purchase::
            IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) override;
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

    void OnOutOfBandPurchasesNotification(FString JsonData, bool bWasSuccessfulReceipts, FString ErrorMessage);

private:
    void ProcessOutOfBandPurchases(
        bool WasSuccessful,
        bool WasCancelled,
        FString ErrorMessage,
        TArray<TSharedRef<const FRedpointGooglePurchase>> Purchases);
};

#endif