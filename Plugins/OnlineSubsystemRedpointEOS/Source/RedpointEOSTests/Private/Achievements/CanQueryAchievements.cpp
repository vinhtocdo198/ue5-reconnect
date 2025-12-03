// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineAchievementsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineAchievementsInterface_CanQueryAchievements,
    "Redpoint.EOS.OnlineAchievementsInterface.CanQueryAchievements",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineAchievementsInterface_CanQueryAchievements::RunAsyncTest(const TFunction<void()> &OnDone)
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
            if (!(Subsystem.IsValid() && UserId.IsValid()))
            {
                this->AddError(FString::Printf(TEXT("Unable to init subsystem / authenticate")));
                OnDone();
                return;
            }

            auto Achievements = Subsystem->GetAchievementsInterface();
            TestTrue("Online subsystem provides IOnlineAchievements interface", Achievements.IsValid());

            if (!Achievements.IsValid())
            {
                OnDone();
                return;
            }

            Achievements->QueryAchievements(
                *UserId,
                FOnQueryAchievementsCompleteDelegate::CreateLambda(
                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                    [this, Achievements, OnDone](const FUniqueNetId &UserId, const bool bWasSuccessful) {
                        TestTrue("QueryAchievements was successful", bWasSuccessful);

                        FOnlineAchievement AchOut;
                        auto GotAch = Achievements->GetCachedAchievement(UserId, TEXT("TestAchievement"), AchOut);
                        TestEqual("GetCachedAchievement found achievement", GotAch, EOnlineCachedResult::Success);
                        if (GotAch == EOnlineCachedResult::Success)
                        {
                            TestEqual("TestAchievement has expected ID", AchOut.Id, TEXT("TestAchievement"));
                        }

                        OnDone();
                    }));
        });
}