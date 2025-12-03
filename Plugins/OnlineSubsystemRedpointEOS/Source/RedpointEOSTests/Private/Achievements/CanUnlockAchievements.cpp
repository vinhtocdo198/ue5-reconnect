// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineAchievementsInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineStats.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineAchievementsInterface_CanUnlockAchievements,
    "Redpoint.EOS.OnlineAchievementsInterface.CanUnlockAchievements",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineAchievementsInterface_CanUnlockAchievements::RunAsyncTest(
    const TFunction<void()> &OnDone)
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

            auto AchievementsWrite = MakeShared<FOnlineAchievementsWrite, ESPMode::ThreadSafe>();
            AchievementsWrite->Properties.Add(TEXT("TestAchievement"), 1.0f);

            Achievements->WriteAchievements(
                *UserId,
                AchievementsWrite,
                FOnAchievementsWrittenDelegate::CreateLambda(
                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                    [this, OnDone](const FUniqueNetId &UserId, const bool bWasSuccessful) {
                        TestTrue("WriteAchievements was successful", bWasSuccessful);
                        OnDone();
                    }));
        });
}