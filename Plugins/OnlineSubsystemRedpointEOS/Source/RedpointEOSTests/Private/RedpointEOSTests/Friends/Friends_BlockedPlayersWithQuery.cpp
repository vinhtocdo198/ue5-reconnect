// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineFriendsInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3505928604, Redpoint::EOS::Tests::Friends)
{

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_Friends_BlockedPlayersWithQuery,
    "Redpoint.EOS.Friends.BlockedPlayersWithQuery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_Friends_BlockedPlayersWithQuery::RunAsyncTest(const TFunction<void()> &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1, 1},
        OnDone,
        FInstanceFactory::EFlag::None,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnTaskDone) {
            auto UserId = Instances[0].UserId;
            auto Friends = Instances[0].Subsystem.Pin()->GetFriendsInterface();

            // @note: Redpoint EOS no longer requires developers to call the query functions prior to retrieving the
            // friend system state, as the initial state is loaded as part of the login process.

            TSharedRef<FDelegateHandle> QueryHandle = MakeShared<FDelegateHandle>();
            *QueryHandle = Friends->AddOnQueryBlockedPlayersCompleteDelegate_Handle(
                FOnQueryBlockedPlayersCompleteDelegate::CreateLambda(
                    [this,
                     OnTaskDone,
                     QueryHandle,
                     FriendsWk = TWeakPtr<IOnlineFriends, ESPMode::ThreadSafe>(Friends),
                     UserIdEOS = UserId](const FUniqueNetId &, bool bWasSuccessful, const FString &) {
                        TestTrue("Can query blocked players", bWasSuccessful);
                        if (!bWasSuccessful)
                        {
                            OnTaskDone();
                            return;
                        }

                        TArray<TSharedRef<FOnlineBlockedPlayer>> BlockedPlayers;
                        TestTrue(
                            "Can get blocked players",
                            FriendsWk.Pin()->GetBlockedPlayers(*UserIdEOS, BlockedPlayers));
                        TestEqual("Blocked players equals 0", BlockedPlayers.Num(), 0);

                        OnTaskDone();
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
