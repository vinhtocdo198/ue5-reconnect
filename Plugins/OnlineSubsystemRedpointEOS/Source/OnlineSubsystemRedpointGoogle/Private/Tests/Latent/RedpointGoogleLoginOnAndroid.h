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

class FRedpointGoogleLoginOnAndroid : public IAutomationLatentCommand
{
private:
    FAutomationTestBase *TestBase;
    FName SubsystemName;
    FDelegateHandle LoginDelegateHandle;
    bool bHasInit;
    bool bIsDone;
    IOnlineIdentityPtr Identity;

    void OnLoginComplete(int32 LocalUserNum, bool WasSuccessful, const FUniqueNetId &UserId, const FString &Error);

public:
    FRedpointGoogleLoginOnAndroid(FAutomationTestBase *InTestBase, FName InSubsystemName)
        : TestBase(InTestBase)
        , SubsystemName(InSubsystemName)
        , LoginDelegateHandle()
        , bHasInit(false)
        , bIsDone(false)
        , Identity(){};
    virtual ~FRedpointGoogleLoginOnAndroid(){};
    virtual bool Update() override;
};

#endif // #if EOS_GOOGLE_ENABLED && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)