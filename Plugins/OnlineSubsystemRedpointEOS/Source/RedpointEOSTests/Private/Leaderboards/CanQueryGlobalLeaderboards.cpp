// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineLeaderboardInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineLeaderboardsInterface_CanQueryGlobalLeaderboards,
    "Redpoint.EOS.OnlineLeaderboardsInterface.CanQueryGlobalLeaderboards",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineLeaderboardsInterface_CanQueryGlobalLeaderboards::RunAsyncTest(
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

            FOnlineLeaderboardReadRef ReadRef = MakeShared<FOnlineLeaderboardRead, ESPMode::ThreadSafe>();
#if REDPOINT_EOS_UE_5_5_OR_LATER
            ReadRef->LeaderboardName = FString(TEXT("TestScore"));
#else
            ReadRef->LeaderboardName = FName(TEXT("TestScore"));
#endif

            auto CancelCallback = RegisterOSSCallback(
                this,
                Leaderboards,
                &IOnlineLeaderboards::AddOnLeaderboardReadCompleteDelegate_Handle,
                &IOnlineLeaderboards::ClearOnLeaderboardReadCompleteDelegate_Handle,
                // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                TFunction<void(bool)>([this, OnDone, Leaderboards, ReadRef](bool bWasSuccessful) {
                    TestTrue("ReadLeaderboardsAroundRank operation succeeded", bWasSuccessful);

                    // We *SHOULD* have at least one row in the global leaderboard, given that we've
                    // pre-populated it with data.
                    TestTrue("Global leaderboard has at least one row", ReadRef->Rows.Num() > 0);

                    if (ReadRef->Rows.Num() > 0)
                    {
                        auto FirstRow = ReadRef->Rows[0];

                        // If we have at least one row, test that it has the expected columns.
                        TestEqual("First row has rank 1", FirstRow.Rank, 1);
                        TestTrue("First row has player ID", FirstRow.PlayerId.IsValid());
                        TestTrue("First row has score column", FirstRow.Columns.Contains(TEXT("Score")));
                    }

                    OnDone();
                }));
            if (!Leaderboards->ReadLeaderboardsAroundRank(0, 100, ReadRef))
            {
                CancelCallback();
                TestTrue("ReadLeaderboardsAroundRank operation started", false);
                OnDone();
            }
        });
}