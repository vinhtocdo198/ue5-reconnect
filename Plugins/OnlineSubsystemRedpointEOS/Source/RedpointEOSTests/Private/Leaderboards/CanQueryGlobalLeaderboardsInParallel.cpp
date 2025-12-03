// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineLeaderboardsInterface_CanQueryGlobalLeaderboardsInParallel,
    "Redpoint.EOS.OnlineLeaderboardsInterface.CanQueryGlobalLeaderboardsInParallel",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineLeaderboardsInterface_CanQueryGlobalLeaderboardsInParallel::RunAsyncTest(
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

            auto Leaderboards = Subsystem->GetLeaderboardsInterface();
            TestTrue("Online subsystem provides IOnlineLeaderboards interface", Leaderboards.IsValid());

            if (!Leaderboards.IsValid())
            {
                OnDone();
                return;
            }

            FOnlineLeaderboardReadRef ReadRef1 = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
#if REDPOINT_EOS_UE_5_5_OR_LATER
            ReadRef1->LeaderboardName = FString(TEXT("TestScore"));
#else
            ReadRef1->LeaderboardName = FName(TEXT("TestScore"));
#endif

            FOnlineLeaderboardReadRef ReadRef2 = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
#if REDPOINT_EOS_UE_5_5_OR_LATER
            ReadRef2->LeaderboardName = FString(TEXT("TestScore2"));
#else
            ReadRef2->LeaderboardName = FName(TEXT("TestScore2"));
#endif

            int *Count = new int(0);

            auto CancelCallback1 = RegisterOSSCallback(
                this,
                Leaderboards,
                &IOnlineLeaderboards::AddOnLeaderboardReadCompleteDelegate_Handle,
                &IOnlineLeaderboards::ClearOnLeaderboardReadCompleteDelegate_Handle,
                // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                TFunction<void(bool)>([this, OnDone, Leaderboards, ReadRef1, Count](bool bWasSuccessful) {
                    TestTrue("(1) ReadLeaderboardsAroundRank operation succeeded", bWasSuccessful);

                    (*Count)--;
                    if (*Count == 0)
                    {
                        delete Count;
                        OnDone();
                    }
                }));
            (*Count)++;
            if (!Leaderboards->ReadLeaderboardsAroundRank(0, 100, ReadRef1))
            {
                CancelCallback1();
                TestTrue("(1) ReadLeaderboardsAroundRank operation started", false);
                (*Count)--;
                if (*Count == 0)
                {
                    delete Count;
                    OnDone();
                    return;
                }
            }

            auto CancelCallback2 = RegisterOSSCallback(
                this,
                Leaderboards,
                &IOnlineLeaderboards::AddOnLeaderboardReadCompleteDelegate_Handle,
                &IOnlineLeaderboards::ClearOnLeaderboardReadCompleteDelegate_Handle,
                // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                TFunction<void(bool)>([this, OnDone, Leaderboards, ReadRef2, Count](bool bWasSuccessful) {
                    TestTrue("(2) ReadLeaderboardsAroundRank operation succeeded", bWasSuccessful);

                    (*Count)--;
                    if (*Count == 0)
                    {
                        delete Count;
                        OnDone();
                    }
                }));
            (*Count)++;
            if (!Leaderboards->ReadLeaderboardsAroundRank(0, 100, ReadRef2))
            {
                CancelCallback1();
                TestTrue("(2) ReadLeaderboardsAroundRank operation started", false);
                (*Count)--;
                if (*Count == 0)
                {
                    delete Count;
                    OnDone();
                }
            }
        });
}