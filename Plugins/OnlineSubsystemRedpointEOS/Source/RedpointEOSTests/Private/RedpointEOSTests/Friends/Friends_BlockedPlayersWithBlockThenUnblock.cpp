// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2468118363, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_BlockedPlayersWithBlockThenUnblock,
    "Redpoint.EOS.Friends.BlockedPlayersWithBlockThenUnblock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_BlockedPlayersWithBlockThenUnblock::RunAsyncTest(const TFunction<void()> &OnDone)
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
                     UserIdEOS = UserId,
                     TargetUserId = Instances[1].UserId](
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

                        {
                            TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                            TestTrue(
                                "Can get blocked players",
                                FriendsWk.Pin()->GetBlockedPlayers(*UserIdEOS, BlockedPlayers));
                            TestEqual("Blocked players equals 1", BlockedPlayers.Num(), 1);
                        }

                        TSharedRef<FDelegateHandle> UnblockHandle = MakeShared<FDelegateHandle>();
                        *UnblockHandle = FriendsWk.Pin()->AddOnUnblockedPlayerCompleteDelegate_Handle(
                            0,
                            FOnUnblockedPlayerCompleteDelegate::CreateLambda([this, OnTaskDone, FriendsWk, UserIdEOS](
                                                                                 int32,
                                                                                 bool bWasUnblockSuccessful,
                                                                                 const FUniqueNetId &,
                                                                                 const FString &,
                                                                                 const FString &) {
                                TestTrue("UnblockPlayer operation succeeds", bWasUnblockSuccessful);
                                if (!bWasUnblockSuccessful)
                                {
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

                                OnTaskDone();
                            }));

                        if (!TestTrue(TEXT("Can call UnblockPlayer"), FriendsWk.Pin()->UnblockPlayer(0, *TargetUserId)))
                        {
                            FriendsWk.Pin()->ClearOnUnblockedPlayerCompleteDelegate_Handle(0, *UnblockHandle);
                            OnTaskDone();
                        }
                    }));

            if (!TestTrue(TEXT("Can call BlockPlayer"), Friends->BlockPlayer(0, *Instances[1].UserId)))
            {
                Friends->ClearOnBlockedPlayerCompleteDelegate_Handle(0, *BlockHandle);
                OnTaskDone();
            }
        });
}

}

REDPOINT_EOS_CODE_GUARD_END()