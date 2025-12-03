// Copyright June Rhodes. All Rights Reserved.

#if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "Misc/AutomationTest.h"
#include "OnlineSubsystemRedpointGoogle.h"
#include "OnlineSubsystemRedpointGoogleConstants.h"
#include "Tests/AutomationCommon.h"
#include "Tests/Latent/RedpointGoogleDeferredCheckout.h"
#include "Tests/Latent/RedpointGoogleLoginOnAndroid.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FOnlineSubsystemRedpointGoogle_EOS_CanDetectPurchaseDeferredSuccess,
    "OnlineSubsystemRedpointGoogle.EOS.CanDetectPurchaseDeferredSuccess",
    EAutomationTestFlags::ClientContext | EAutomationTestFlags::ProductFilter);

bool FOnlineSubsystemRedpointGoogle_EOS_CanDetectPurchaseDeferredSuccess::RunTest(const FString &InParam)
{
    FAutomationTestFramework::Get().SetRequestedTestFilter(EAutomationTestFlags::ProductFilter);
    FAutomationTestFramework::Get().EnqueueLatentCommand(
        MakeShareable(new FRedpointGoogleLoginOnAndroid(this, FName(TEXT("RedpointEOS")))));

    FPurchaseCheckoutRequest CheckoutWithItem;
    CheckoutWithItem.AddPurchaseOffer(TEXT(""), TEXT("test_iap"), 1);
    FAutomationTestFramework::Get().EnqueueLatentCommand(MakeShareable(new FRedpointGoogleDeferredCheckout(
        this,
        FName(TEXT("RedpointEOS")),
        CheckoutWithItem,
        EPurchaseTransactionState::Purchased)));

    return true;
}

#endif // #if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)
