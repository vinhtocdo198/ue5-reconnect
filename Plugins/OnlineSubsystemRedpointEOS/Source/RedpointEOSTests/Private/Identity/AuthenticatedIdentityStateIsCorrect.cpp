// Copyright June Rhodes. All Rights Reserved.

#include "Interfaces/OnlineIdentityInterface.h"
#include "Misc/AutomationTest.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineIdentityInterface_AuthenticatedIdentityStateIsCorrect,
    "Redpoint.EOS.OnlineIdentityInterface.AuthenticatedIdentityStateIsCorrect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineIdentityInterface_AuthenticatedIdentityStateIsCorrect::RunAsyncTest(
    const FOnDone &OnDone)
{
    FInstanceFactory::CreateInstances(
        this,
        TArray<int32>{1},
        OnDone,
        FInstanceFactory::EFlag::EpicGames,
        [this](const TArray<FMultiplayerScenarioInstance> &Instances, const FOnDone &OnDone) {
            FMultiplayerScenarioInstance Instance = Instances[0];

            auto Identity = Instance.Subsystem.Pin()->GetIdentityInterface();
            TestTrue("Online subsystem provides IOnlineIdentity interface", Identity.IsValid());

            if (!Identity.IsValid())
            {
                OnDone();
                return;
            }

            TestTrue("Local user num 0 is logged in", Identity->GetLoginStatus(0) == ELoginStatus::LoggedIn);
            TestEqual("Local user num 0 nickname matches", Identity->GetPlayerNickname(0), TEXT("redpointqa0001"));

            TSharedPtr<const FUniqueNetId> Id = Identity->GetUniquePlayerId(0);
            TestTrue("Local user num 0 has unique net ID", Id.IsValid());

            if (Id.IsValid())
            {
                TSharedPtr<const FUniqueNetId> IdRecreated = Identity->CreateUniquePlayerId(Id->ToString());
                if (TestTrue("Local user ID can be recreated through CreateUniquePlayerId", IdRecreated.IsValid()))
                {
                    TestEqual(
                        "Local user by ID nickname matches",
                        Identity->GetPlayerNickname(*Id),
                        TEXT("redpointqa0001"));
                    TestEqual(
                        "Local user by recreated ID nickname matches",
                        Identity->GetPlayerNickname(*IdRecreated),
                        TEXT("redpointqa0001"));

                    TSharedPtr<FUserOnlineAccount> LocalUserAccount = Identity->GetUserAccount(*Id);
                    TestTrue("Local user by ID has local user account", LocalUserAccount.IsValid());
                    TestTrue(
                        "Local user by recreated ID has local user account",
                        Identity->GetUserAccount(*IdRecreated).IsValid());

                    if (TestEqual(
                            "Local user ID has unique net ID of EOS type",
                            Id->GetType(),
                            FName(TEXT("RedpointEOS"))))
                    {
                        TSharedPtr<const FUniqueNetIdEOS> IdEOS = StaticCastSharedPtr<const FUniqueNetIdEOS>(Id);

                        if (LocalUserAccount.IsValid())
                        {
                            FString AccountPUID;
                            if (TestTrue(
                                    "Account has PUID attribute",
                                    LocalUserAccount->GetUserAttribute(TEXT("productUserId"), AccountPUID)))
                            {
                                TestEqual(
                                    "Account PUID attribute matches net ID PUID",
                                    AccountPUID,
                                    IdEOS->GetProductUserIdString());
                            }

                            FString CanLink;
                            if (TestTrue(
                                    "Account has crossPlatform.canLink attribute",
                                    LocalUserAccount->GetAuthAttribute(TEXT("crossPlatform.canLink"), CanLink)))
                            {
                                // Don't need to check value, just need to check it exists.
                            }

                            TestEqual(
                                "Account display name matches",
                                LocalUserAccount->GetDisplayName(),
                                TEXT("redpointqa0001"));
                        }
                    }
                }
            }

            OnDone();
        });
}