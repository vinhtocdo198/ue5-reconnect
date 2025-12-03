// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineLobbyInterface_CanCreateAndDeleteLobby,
    "Redpoint.EOS.OnlineLobbyInterface.CanCreateAndDeleteLobby",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineLobbyInterface_CanCreateAndDeleteLobby::RunAsyncTest(const TFunction<void()> &OnDone)
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

            TSharedPtr<FOnlineLobbyTransaction> Txn = LobbyInterface->MakeCreateLobbyTransaction(*UserId);
            TestTrue("CreateLobbyTransaction should return new transaction", Txn.IsValid());

            Txn->Capacity = 4;
            Txn->Locked = true;
            Txn->Public = true;

            bool bCreateStarted = LobbyInterface->CreateLobby(
                *UserId,
                *Txn,
                FOnLobbyCreateOrConnectComplete::CreateLambda([this,
                                                               // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                                               LobbyInterface,
                                                               UserId,
                                                               OnDone](
                                                                  const FOnlineError &Error,
                                                                  const FUniqueNetId &CreateUserId,
                                                                  const TSharedPtr<class FOnlineLobby> &Lobby) {
                    TestTrue(
                        FString::Printf(TEXT("CreateLobby call should succeed: %s"), *Error.ToLogString()),
                        Error.bSucceeded);
                    if (Error.bSucceeded)
                    {
                        TSharedPtr<bool> DidFire = MakeShared<bool>(false);
                        FDelegateHandle EventHandle =
                            // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                            LobbyInterface->AddOnLobbyDeleteDelegate_Handle(
                                FOnLobbyDeleteDelegate::CreateLambda([this, UserId, Lobby, DidFire](
                                                                         const FUniqueNetId &DeleteUserId,
                                                                         const FOnlineLobbyId &LobbyId) {
                                    TestTrue("OnLobbyDelete user ID matches", DeleteUserId == *UserId);
                                    TestTrue("OnLobbyDelete lobby ID matches", LobbyId == *Lobby->Id);
                                    *DidFire = true;
                                }));
                        bool bDeleteStarted = LobbyInterface->DeleteLobby(
                            CreateUserId,
                            *Lobby->Id,
                            FOnLobbyOperationComplete::CreateLambda(
                                // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                [this, EventHandle, LobbyInterface, DidFire, OnDone](
                                    const FOnlineError &Error,
                                    const FUniqueNetId &UserId) {
                                    FDelegateHandle EventHandleCopy = EventHandle;
                                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                    LobbyInterface->ClearOnLobbyDeleteDelegate_Handle(EventHandleCopy);
                                    TestTrue("OnLobbyDelete event should have fired", *DidFire);
                                    TestTrue(
                                        FString::Printf(
                                            TEXT("DeleteLobby call should succeed: %s"),
                                            *Error.ToLogString()),
                                        Error.bSucceeded);
                                    OnDone();
                                }));
                        TestTrue("DeleteLobby should start", bDeleteStarted);
                        if (!bDeleteStarted)
                        {
                            LobbyInterface->ClearOnLobbyDeleteDelegate_Handle(EventHandle);
                            OnDone();
                        }
                    }
                    else
                    {
                        OnDone();
                    }
                }));
            TestTrue("CreateLobby should start", bCreateStarted);
            if (!bCreateStarted)
            {
                OnDone();
            }
        });
}