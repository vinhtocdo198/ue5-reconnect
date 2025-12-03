// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Purchase/OnlinePurchaseWithFinalizePurchaseCallback.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UserIdMap.h"

EOS_ENABLE_STRICT_WARNINGS

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePurchaseInterfaceSynthetic :
#if PLATFORM_ANDROID
    public Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Purchase::
        IOnlinePurchaseWithFinalizePurchaseCallback,
#else  // #if PLATFORM_ANDROID
    public IOnlinePurchase,
#endif // #if PLATFORM_ANDROID
    public TSharedFromThis<FOnlinePurchaseInterfaceSynthetic, ESPMode::ThreadSafe>
{
private:
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSRef IdentityEOS;
    TUserIdMap<TSharedPtr<struct FNativeEventBinding>> NativeEventBindingsRegisteredPerUser;

    void OnNativeUnexpectedPurchaseReceipt(
        const FUniqueNetId &UserId,
        TSharedPtr<struct FNativeEventBinding> NativeEventBinding);
    void OnLoginStatusChanged(
        int32 LocalUserNum,
        ELoginStatus::Type OldStatus,
        ELoginStatus::Type NewStatus,
        const FUniqueNetId &UserId);

public:
    FOnlinePurchaseInterfaceSynthetic(
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSRef
            InIdentityEOS)
        : IdentityEOS(MoveTemp(InIdentityEOS)) {};
    void RegisterEvents();
    UE_NONCOPYABLE(FOnlinePurchaseInterfaceSynthetic);
    virtual ~FOnlinePurchaseInterfaceSynthetic() override = default;

    virtual bool IsAllowedToPurchase(const FUniqueNetId &UserId) override;
    virtual void Checkout(
        const FUniqueNetId &UserId,
        const FPurchaseCheckoutRequest &CheckoutRequest,
        const FOnPurchaseCheckoutComplete &Delegate) override;
    virtual void FinalizePurchase(const FUniqueNetId &UserId, const FString &ReceiptId) override;
    virtual void FinalizePurchase(const FUniqueNetId &UserId, const FString &ReceiptId, const FString &ReceiptInfo)
        override;
#if PLATFORM_ANDROID
    virtual void AcknowledgePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) override;
    virtual void ConsumePurchaseWithCallback(
        const FUniqueNetId &UserId,
        const FString &ReceiptId,
        const IOnlinePurchaseWithFinalizePurchaseCallback::FOnFinalizeReceiptComplete &Delegate) override;
#endif // #if PLATFORM_ANDROID
#if REDPOINT_EOS_UE_5_1_OR_LATER
    virtual void Checkout(
        const FUniqueNetId &UserId,
        const FPurchaseCheckoutRequest &CheckoutRequest,
        const FOnPurchaseReceiptlessCheckoutComplete &Delegate) override;
#endif
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

EOS_DISABLE_STRICT_WARNINGS