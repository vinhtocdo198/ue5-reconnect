// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineIdentityInterface_IdTokenAvailableIfSdkSupportsIt,
    "Redpoint.EOS.OnlineIdentityInterface.IdTokenAvailableIfSdkSupportsIt",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineIdentityInterface_IdTokenAvailableIfSdkSupportsIt::RunAsyncTest(const FOnDone &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](TArray<FMultiplayerScenarioInstance> Instances, const FOnDone &OnDone) {
            FMultiplayerScenarioInstance Instance = Instances[0];

            auto Identity = Instance.Subsystem.Pin()->GetIdentityInterface();
            TestTrue("Online subsystem provides IOnlineIdentity interface", Identity.IsValid());

            if (!Identity.IsValid())
            {
                OnDone();
                return;
            }

            TestFalse("GetAuthToken must return value", Identity->GetAuthToken(0).IsEmpty());
            TestFalse(
                "GetAccessToken must return value",
                Identity->GetUserAccount(*Identity->GetUniquePlayerId(0))->GetAccessToken().IsEmpty());
            OnDone();
        });
}
