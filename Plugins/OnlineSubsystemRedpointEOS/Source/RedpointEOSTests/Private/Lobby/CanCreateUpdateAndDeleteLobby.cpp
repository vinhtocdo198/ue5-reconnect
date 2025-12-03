// Copyright June Rhodes. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "TestHelpers.h"

IMPLEMENT_ASYNC_AUTOMATION_TEST(
    FOnlineSubsystemEOS_OnlineLobbyInterface_CanCreateUpdateAndDeleteLobby,
    "Redpoint.EOS.OnlineLobbyInterface.CanCreateUpdateAndDeleteLobby",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::ServerContext |
        EAutomationTestFlags::ProductFilter);

void FOnlineSubsystemEOS_OnlineLobbyInterface_CanCreateUpdateAndDeleteLobby::RunAsyncTest(
    const TFunction<void()> &OnDone)
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
                        TSharedPtr<FOnlineLobbyTransaction> UpdateTxn =
                            // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                            LobbyInterface->MakeUpdateLobbyTransaction(CreateUserId, *Lobby->Id);

                        UpdateTxn->Capacity = 8;

                        TSharedPtr<bool> DidFireUpdate = MakeShared<bool>(false);
                        FDelegateHandle EventHandle = LobbyInterface->AddOnLobbyUpdateDelegate_Handle(
                            FOnLobbyUpdateDelegate::CreateLambda([this, UserId, Lobby, DidFireUpdate](
                                                                     const FUniqueNetId &UpdateUserId,
                                                                     const FOnlineLobbyId &LobbyId) {
                                TestTrue("OnLobbyUpdate user ID matches", UpdateUserId == *UserId);
                                TestTrue("OnLobbyUpdate lobby ID matches", LobbyId == *Lobby->Id);
                                *DidFireUpdate = true;
                            }));

                        bool bUpdateStarted = LobbyInterface->UpdateLobby(
                            CreateUserId,
                            *Lobby->Id,
                            *UpdateTxn,
                            FOnLobbyOperationComplete::CreateLambda(
                                [this,
                                 // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                 LobbyInterface,
                                 EventHandle,
                                 DidFireUpdate,
                                 Lobby,
                                 OnDone](const FOnlineError &Error, const FUniqueNetId &UserId) {
                                    TestTrue(
                                        FString::Printf(
                                            TEXT("UpdateLobby call should succeed: %s"),
                                            *Error.ToLogString()),
                                        Error.bSucceeded);
                                    if (Error.bSucceeded)
                                    {
                                        // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                        bool bDeleteStarted = LobbyInterface->DeleteLobby(
                                            UserId,
                                            *Lobby->Id,
                                            FOnLobbyOperationComplete::CreateLambda(
                                                [this,
                                                 // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                                 LobbyInterface,
                                                 EventHandle,
                                                 DidFireUpdate,
                                                 OnDone](const FOnlineError &Error, const FUniqueNetId &UserId) {
                                                    FDelegateHandle EventHandleCopy = EventHandle;
                                                    // NOLINTNEXTLINE(unreal-unsafe-storage-of-oss-pointer)
                                                    LobbyInterface->ClearOnLobbyUpdateDelegate_Handle(EventHandleCopy);
                                                    TestTrue("OnLobbyUpdate event should have fired", *DidFireUpdate);
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
                                            FDelegateHandle EventHandleCopy = EventHandle;
                                            LobbyInterface->ClearOnLobbyUpdateDelegate_Handle(EventHandleCopy);
                                            OnDone();
                                        }
                                    }
                                    else
                                    {
                                        FDelegateHandle EventHandleCopy = EventHandle;
                                        LobbyInterface->ClearOnLobbyUpdateDelegate_Handle(EventHandleCopy);
                                        OnDone();
                                    }
                                }));
                        TestTrue("UpdateLobby should start", bUpdateStarted);
                        if (!bUpdateStarted)
                        {
                            FDelegateHandle EventHandleCopy = EventHandle;
                            LobbyInterface->ClearOnLobbyUpdateDelegate_Handle(EventHandleCopy);
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