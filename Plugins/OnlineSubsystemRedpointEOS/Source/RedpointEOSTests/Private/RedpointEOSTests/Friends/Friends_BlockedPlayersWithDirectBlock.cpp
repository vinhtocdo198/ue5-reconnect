// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2470839011, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_BlockedPlayersWithDirectBlock,
    "Redpoint.EOS.Friends.BlockedPlayersWithDirectBlock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_BlockedPlayersWithDirectBlock::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnTaskDone) {
            auto UserId = Instances[0].UserId;
            auto Friends = Instances[0].Subsystem.Pin()->GetFriendsInterface();

            TSharedRef<FDelegateHandle> BlockHandle = MakeShared<FDelegateHandle>();
            *BlockHandle = Friends->AddOnBlockedPlayerCompleteDelegate_Handle(
                0,
                FOnBlockedPlayerCompleteDelegate::CreateLambda(
                    [this,
                     OnTaskDone,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId](
                        int32 LocalUserNum,
                        bool bWasSuccessful,
                        const FUniqueNetId &UniqueID,
                        const FString &ListName,
                        const FString &ErrorStr) {
                        TestTrue("BlockPlayer operation succeeds", bWasSuccessful);
                        if (!bWasSuccessful)
                        {
                            OnTaskDone();
                            return;
                        }

                        TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                        TestTrue(
                            "Can get blocked players",
                            FriendsWk.Pin()->GetBlockedPlayers(*UserIdEOS, BlockedPlayers));
                        TestEqual("Blocked players equals 1", BlockedPlayers.Num(), 1);

                        OnTaskDone();
                    }));

            TArray<FReportPlayedWithUser> Players;
            Players.Add(FReportPlayedWithUser(Instances[1].UserId.ToSharedRef(), TEXT("")));

            if (!TestTrue(TEXT("Can call BlockPlayer"), Friends->BlockPlayer(0, *Instances[1].UserId)))
            {
                Friends->ClearOnBlockedPlayerCompleteDelegate_Handle(0, *BlockHandle);
                OnTaskDone();
            }
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()