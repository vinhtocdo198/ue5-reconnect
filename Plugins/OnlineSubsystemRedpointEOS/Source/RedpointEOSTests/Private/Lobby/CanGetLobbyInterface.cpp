// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineLobbyInterface_CanGetLobbyInterface,
    "Redpoint.EOS.OnlineLobbyInterface.CanGetLobbyInterface",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineLobbyInterface_CanGetLobbyInterface::RunAsyncTest(const TFunction<void()> &OnDone)
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

            auto LobbyInterface = Online::GetLobbyInterface(Subsystem.Get());
            TestTrue("Online subsystem provides IOnlineLobby interface", LobbyInterface.IsValid());

            OnDone();
        });
}