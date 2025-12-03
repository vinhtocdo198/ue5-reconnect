// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineError.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"
#include "Tests/AutomationCommon.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2114668918, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_RecentPlayersWithQueryThenAdd,
    "Redpoint.EOS.Friends.RecentPlayersWithQueryThenAdd",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_RecentPlayersWithQueryThenAdd::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnTaskDone) {
            auto UserId = Instances[0].UserId;
            auto Friends = Instances[0].Subsystem.Pin()->GetFriendsInterface();
            auto OtherUserId = Instances[1].UserId;

            TSharedRef<FDelegateHandle> QueryHandle = MakeShared<FDelegateHandle>();
            *QueryHandle = Friends->AddOnQueryRecentPlayersCompleteDelegate_Handle(
                FOnQueryRecentPlayersCompleteDelegate::CreateLambda(
                    [this,
                     OnTaskDone,
                     QueryHandle,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId,
                     OtherUserId](const FUniqueNetId &, const FString &, bool bWasSuccessful, const FString &) {
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

                        TArray<FReportPlayedWithUser> Players;
                        Players.Add(FReportPlayedWithUser(OtherUserId.ToSharedRef(), TEXT("")));

                        FriendsWk.Pin()->AddRecentPlayers(
                            *UserIdEOS,
                            Players,
                            TEXT(""),
                            FOnAddRecentPlayersComplete::CreateLambda([this, OnTaskDone, FriendsWk, UserIdEOS](
                                                                          const FUniqueNetId &,
                                                                          const FOnlineError &Error) {
                                TestTrue("AddRecentPlayers operation succeeds", Error.bSucceeded);
                                if (!Error.bSucceeded)
                                {
                                    OnTaskDone();
                                    return;
                                }

                                {
                                    TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
                                    TestTrue(
                                        "Can get recent players before query",
                                        FriendsWk.Pin()->GetRecentPlayers(*UserIdEOS, TEXT(""), RecentPlayers));
                                    TestEqual("Recent players equals 1 before query", RecentPlayers.Num(), 1);
                                }

                                OnTaskDone();
                            }));
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
