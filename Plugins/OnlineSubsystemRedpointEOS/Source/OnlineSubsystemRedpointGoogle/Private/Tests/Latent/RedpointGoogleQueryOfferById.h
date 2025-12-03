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

class FRedpointGoogleQueryOfferById : public IAutomationLatentCommand
{
private:
    FAutomationTestBase *TestBase;
    FName SubsystemName;
    bool bHasInit;
    bool bIsDone;
    IOnlineIdentityPtr Identity;
    IOnlineStoreV2Ptr StoreV2;
    TArray<FUniqueOfferId> ExpectedOfferIds;

    void OnOffersQueried(bool bWasSuccessful, const TArray<FUniqueOfferId> &OfferIds, const FString &Error);

public:
    FRedpointGoogleQueryOfferById(
        FAutomationTestBase *InTestBase,
        FName InSubsystemName,
        TArray<FUniqueOfferId> InOfferIds)
        : TestBase(InTestBase)
        , SubsystemName(InSubsystemName)
        , bHasInit(false)
        , bIsDone(false)
        , Identity()
        , StoreV2()
        , ExpectedOfferIds(InOfferIds){};
    virtual ~FRedpointGoogleQueryOfferById(){};
    virtual bool Update() override;
};

#endif // #if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)