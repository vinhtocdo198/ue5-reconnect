// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineStatsInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineError.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineStatsInterface_CanCallSingularQueryStats,
    "Redpoint.EOS.OnlineStatsInterface.CanCallSingularQueryStats",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineStatsInterface_CanCallSingularQueryStats::RunAsyncTest(const TFunction<void()> &OnDone)
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

            auto Stats = Subsystem->GetStatsInterface();
            TestTrue("Online subsystem provides IOnlineStats interface", Stats.IsValid());

            if (!Stats.IsValid())
            {
                OnDone();
                return;
            }

            Stats->QueryStats(
                UserId.ToSharedRef(),
                UserId.ToSharedRef(),
                FOnlineStatsQueryUserStatsComplete::CreateLambda(
                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                    [this, OnDone, UserId, Stats](
                        const FOnlineError &ResultState,
                        const TSharedPtr<const FOnlineStatsUserStats> &QueriedStats) {
                        TestTrue("QueryStats singular operation succeeded", ResultState.bSucceeded);
                        if (!ResultState.bSucceeded)
                        {
                            OnDone();
                            return;
                        }

                        // NOTE: We can't test anything more than that, because a singular QueryStats
                        // operation may return no rows (if the user hasn't sent stats before), and we can't
                        // just do a write operation beforehand because stats are eventually consistent.
                        //
                        // This test primarily exists just to make sure there aren't any crashes or async
                        // bugs in the implementation.

                        OnDone();
                    }));
        });
}