// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineSubsystem_CanGetSubsystemAndAuthenticate,
    "Redpoint.EOS.OnlineSubsystem.CanGetSubsystemAndAuthenticate",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineSubsystem_CanGetSubsystemAndAuthenticate::RunAsyncTest(const FOnDone &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](
            const IOnlineSubsystemPtr &Subsystem,
            const TSharedPtr<const FUniqueNetIdEOS> &UserId,
            const FOnDone &OnDone) {
            TestTrue("Can create subsystem and authenticate with device ID", Subsystem.IsValid() && UserId.IsValid());
            OnDone();
        });
}