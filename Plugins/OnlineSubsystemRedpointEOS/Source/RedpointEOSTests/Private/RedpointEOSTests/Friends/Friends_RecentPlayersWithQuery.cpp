// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"
#include "Tests/AutomationCommon.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(945558822, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_RecentPlayersWithQuery,
    "Redpoint.EOS.Friends.RecentPlayersWithQuery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_RecentPlayersWithQuery::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnTaskDone) {
            auto UserId = Instances[0].UserId;
            auto Friends = Instances[0].Subsystem.Pin()->GetFriendsInterface();

            TSharedRef<FDelegateHandle> QueryHandle = MakeShared<FDelegateHandle>();
            *QueryHandle = Friends->AddOnQueryRecentPlayersCompleteDelegate_Handle(
                FOnQueryRecentPlayersCompleteDelegate::CreateLambda(
                    [this,
                     OnTaskDone,
                     QueryHandle,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId](const FUniqueNetId &, const FString &, bool bWasSuccessful, const FString &) {
                        TestTrue("Can query recent players", bWasSuccessful);
                        if (!bWasSuccessful)
                        {
                            FriendsWk.Pin()->ClearOnQueryRecentPlayersCompleteDelegate_Handle(*QueryHandle);
                            OnTaskDone();
                            return;
                        }

                        {
                            TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
                            TestTrue(
                                "Can get recent players",
                                FriendsWk.Pin()->GetRecentPlayers(*UserIdEOS, TEXT(""), RecentPlayers));
                            TestEqual("Recent players equals 0", RecentPlayers.Num(), 0);
                        }

                        OnTaskDone();
                    }));

            if (!TestTrue("Can call QueryRecentPlayers", Friends->QueryRecentPlayers(*UserId, TEXT(""))))
            {
                Friends->ClearOnQueryRecentPlayersCompleteDelegate_Handle(*QueryHandle);
                OnTaskDone();
            }
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()