// Copyright June Rhodes. All Rights Reserved.

#include "HAL/MemoryMisc.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "OnlineError.h"
#include "RedpointEOSTestsModule.h"
#include "TestHelpers.h"
#include "Tests/AutomationCommon.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2503960880, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_RecentPlayersWithDirectAdd,
    "Redpoint.EOS.Friends.RecentPlayersWithDirectAdd",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_RecentPlayersWithDirectAdd::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnTaskDone) {
            auto UserId = Instances[0].UserId;
            auto Friends = Instances[0].Subsystem.Pin()->GetFriendsInterface();

            TSharedRef<int> CallCount = MakeShared<int>(0);
            TSharedRef<int> PlayersAddedCount = MakeShared<int>(0);

            Friends->AddOnRecentPlayersAddedDelegate_Handle(FOnRecentPlayersAddedDelegate::CreateLambda(
                [CallCount,
                 PlayersAddedCount](const FUniqueNetId &, const TArray<TSharedRef<FOnlineRecentPlayer>> &AddedPlayers) {
                    *CallCount += 1;
                    *PlayersAddedCount += AddedPlayers.Num();
                }));

            TArray<FReportPlayedWithUser> Players;
            Players.Add(FReportPlayedWithUser(Instances[1].UserId.ToSharedRef(), TEXT("")));

            Friends->AddRecentPlayers(
                *UserId,
                Players,
                TEXT(""),
                FOnAddRecentPlayersComplete::CreateLambda(
                    [this,
                     OnTaskDone,
                     CallCount,
                     PlayersAddedCount,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId](const FUniqueNetId &, const FOnlineError &Error) {
                        TestTrue("AddRecentPlayers operation succeeds", Error.bSucceeded);
                        TestEqual("OnRecentPlayersAdded fired once", *CallCount, 1);
                        TestEqual("OnRecentPlayersAdded reported one user added", *PlayersAddedCount, 1);
                        if (!Error.bSucceeded)
                        {
                            OnTaskDone();
                            return;
                        }

                        TArray<TSharedRef<FOnlineRecentPlayer>> RecentPlayers;
                        TestTrue(
                            "Can get recent players",
                            FriendsWk.Pin()->GetRecentPlayers(*UserIdEOS, TEXT(""), RecentPlayers));
                        TestEqual("Recent players equals 1", RecentPlayers.Num(), 1);

                        OnTaskDone();
                    }));
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()
