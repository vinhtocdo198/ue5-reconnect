// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineAchievementsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineAchievementsInterface_CanQueryAchievementDefinitions,
    "Redpoint.EOS.OnlineAchievementsInterface.CanQueryAchievementDefinitions",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineAchievementsInterface_CanQueryAchievementDefinitions::RunAsyncTest(
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

            Achievements->QueryAchievementDescriptions(
                *UserId,
                FOnQueryAchievementsCompleteDelegate::CreateLambda(
                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                    [this, Achievements, OnDone](const FUniqueNetId &UserId, const bool bWasSuccessful) {
                        TestTrue("QueryAchievementDescriptions was successful", bWasSuccessful);

                        FOnlineAchievementDesc DescOut;
                        auto GotDesc = Achievements->GetCachedAchievementDescription(TEXT("TestAchievement"), DescOut);
                        TestEqual(
                            "GetCachedAchievementDescription found achievement description",
                            GotDesc,
                            EOnlineCachedResult::Success);
                        if (GotDesc == EOnlineCachedResult::Success)
                        {
                            TestEqual("Fetched achievement description is not hidden", DescOut.bIsHidden, false);
                            TestEqual(
                                "Fetched achievement description title has expected value",
                                DescOut.Title.ToString(),
                                TEXT("Test Achievement"));
                            TestEqual(
                                "Fetched achievement description LockedDesc has expected value",
                                DescOut.LockedDesc.ToString(),
                                TEXT("Not unlocked, yet..."));
                            TestEqual(
                                "Fetched achievement description UnlockedDesc has expected value",
                                DescOut.UnlockedDesc.ToString(),
                                TEXT("You got this achievement."));
                        }

                        OnDone();
                    }));
        });
}