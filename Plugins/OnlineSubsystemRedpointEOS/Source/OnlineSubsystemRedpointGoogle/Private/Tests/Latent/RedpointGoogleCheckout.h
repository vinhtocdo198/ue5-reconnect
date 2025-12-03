// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "LogRedpointGoogle.h"
#include "Misc/AutomationTest.h"
#include "OnlineError.h"
#include "OnlineSubsystemRedpointGoogle.h"
#include "OnlineSubsystemRedpointGoogleConstants.h"
#include "Tests/AutomationCommon.h"

class FRedpointGoogleCheckout : public IAutomationLatentCommand
{
private:
    FAutomationTestBase *TestBase;
    FName SubsystemName;
    bool bHasInit;
    bool bIsDone;
    IOnlineIdentityPtr Identity;
    IOnlinePurchasePtr Purchase;
    FPurchaseCheckoutRequest CheckoutRequest;
    EPurchaseTransactionState ExpectedTransactionState;

    void OnFinalizeComplete(const FOnlineError &Result, FString TransactionId);
    void OnPurchaseComplete(const FOnlineError &Result, const TSharedRef<FPurchaseReceipt> &Receipt);

public:
    FRedpointGoogleCheckout(
        FAutomationTestBase *InTestBase,
        FName InSubsystemName,
        const FPurchaseCheckoutRequest &InCheckoutRequest,
        EPurchaseTransactionState bInExpectedTransactionState)
        : TestBase(InTestBase)
        , SubsystemName(InSubsystemName)
        , bHasInit(false)
        , bIsDone(false)
        , Identity()
        , Purchase()
        , CheckoutRequest(InCheckoutRequest)
        , ExpectedTransactionState(bInExpectedTransactionState){};
    virtual ~FRedpointGoogleCheckout(){};
    virtual bool Update() override;
};

#endif // #if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)