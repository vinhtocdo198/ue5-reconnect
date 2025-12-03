// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlinePresenceInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlinePresenceInterface_CanQueryPresence,
    "Redpoint.EOS.OnlinePresenceInterface.CanQueryPresence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlinePresenceInterface_CanQueryPresence::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::EpicGames,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnDone) {
            auto Host = Instances[0];

            auto Presence = Host.Subsystem.Pin()->GetPresenceInterface();
            TestTrue("Online subsystem provides IOnlinePresence interface", Presence.IsValid());

            if (!Presence.IsValid())
            {
                OnDone();
                return;
            }

            Presence->QueryPresence(
                *Host.UserId,
                IOnlinePresence::FOnPresenceTaskCompleteDelegate::CreateLambda(
                    [this, OnDone, Host](const class FUniqueNetId &UserId, const bool bWasSuccessful) {
                        TestTrue(TEXT("Presence was queried"), bWasSuccessful);
                        TestTrue(TEXT("User id matched expected value"), UserId == *Host.UserId);
                        OnDone();
                    }));
        });
}