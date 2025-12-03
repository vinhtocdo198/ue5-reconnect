// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1233915682, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_BlockedPlayersWithQueryThenBlock,
    "Redpoint.EOS.Friends.BlockedPlayersWithQueryThenBlock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_BlockedPlayersWithQueryThenBlock::RunAsyncTest(const TFunction<void()> &OnDone)
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

            // @note: Redpoint EOS no longer requires developers to call the query functions prior to retrieving the
            // friend system state, as the initial state is loaded as part of the login process.

            TSharedRef<FDelegateHandle> QueryHandle = MakeShared<FDelegateHandle>();
            *QueryHandle = Friends->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
                FOnQueryBlockedPlayersCompleteDelegate::CreateLambda(
                    [this,
                     OnTaskDone,
                     QueryHandle,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId,
                     OtherUserId](const FUniqueNetId &, bool bWasSuccessful, const FString &Error) {
                        TestTrue("Can query blocked players", bWasSuccessful);
                        if (!bWasSuccessful)
                        {
                            FriendsWk.Pin()->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(*QueryHandle);
                            OnTaskDone();
                            return;
                        }

                        {
                            TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                            TestTrue(
                                "Can get blocked players",
                                FriendsWk.Pin()->GetBlockedPlayers(*UserIdEOS, BlockedPlayers));
                            TestEqual("Blocked players equals 0", BlockedPlayers.Num(), 0);
                        }

                        TSharedRef<FDelegateHandle> BlockHandle = MakeShared<FDelegateHandle>();
                        *BlockHandle = FriendsWk.Pin()->AddOnBlockedPlayerCompleteDelegate_Handle(
                            0,
                            FOnBlockedPlayerCompleteDelegate::CreateLambda([this, OnTaskDone, FriendsWk, UserIdEOS](
                                                                               int32,
                                                                               bool bWasBlockSuccessful,
                                                                               const FUniqueNetId &,
                                                                               const FString &,
                                                                               const FString &) {
                                TestTrue("BlockPlayer operation succeeds", bWasBlockSuccessful);
                                if (!bWasBlockSuccessful)
                                {
                                    OnTaskDone();
                                    return;
                                }

                                {
                                    TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                                    TestTrue(
                                        "Can get blocked players",
                                        FriendsWk.Pin()->GetBlockedPlayers(*UserIdEOS, BlockedPlayers));
                                    TestEqual("Blocked players equals 1", BlockedPlayers.Num(), 1);
                                }

                                OnTaskDone();
                            }));

                        TArray<FReportPlayedWithUser> Players;
                        Players.Add(FReportPlayedWithUser(OtherUserId.ToSharedRef(), TEXT("")));

                        if (!TestTrue(TEXT("Can call BlockPlayer"), FriendsWk.Pin()->BlockPlayer(0, *OtherUserId)))
                        {
                            FriendsWk.Pin()->ClearOnBlockedPlayerCompleteDelegate_Handle(0, *BlockHandle);
                            OnTaskDone();
                        }
                    }));

            if (!TestTrue("Can call QueryBlockedPlayers", Friends->QueryBlockedPlayers(*UserId)))
            {
                Friends->ClearOnQueryBlockedPlayersCompleteDelegate_Handle(*QueryHandle);
                OnTaskDone();
            }
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()
