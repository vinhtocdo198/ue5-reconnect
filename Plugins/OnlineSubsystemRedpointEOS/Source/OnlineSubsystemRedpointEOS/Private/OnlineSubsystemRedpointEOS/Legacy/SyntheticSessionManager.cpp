// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Legacy/SyntheticSessionManager.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/SubsystemNames.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/Utils/CallContextFactory.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatform/RuntimePlatformIntegration.h"
#include "RedpointEOSPresence/PresenceSystem.h"
#include "RedpointEOSPresence/PresenceUserAdvertisedRoom.h"
#include "RedpointEOSRooms/RoomId.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2530062524, Redpoint::EOS::OnlineSubsystemRedpointEOS::Legacy)
{
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Presence;
using namespace ::Redpoint::EOS::Rooms;

FName FSyntheticSessionManager::GetSystemName()
{
    return FName(TEXT("FSyntheticSessionManager"));
}

TSharedRef<FSyntheticSessionManager> FSyntheticSessionManager::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FSyntheticSessionManager>(InPlatformHandle);
}

FSyntheticSessionManager::FSyntheticSessionManager(const FPlatformHandle &InPlatformHandle)
    : PlatformHandle(InPlatformHandle)
{
}

void FSyntheticSessionManager::OnRoomAdvertisementInviteReceived(
    const UE::Online::FAccountId &LocalUserId,
    const FUserInfoPtr &SenderUser,
    const FRoomIdRef &RoomId,
    const FName &RoomNamespace)
{
    if (!RoomId->GetProviderName().IsEqual(FName(TEXT("Session"))) || !RoomNamespace.IsEqual(FName(TEXT("Synthetic"))))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("%s: Ignoring OnRoomAdvertisementInviteReceived event as it is not for a synthetic session."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Error,
        TEXT("%s: Ignoring OnRoomAdvertisementInviteReceived event as this is not handled by the synthetic session "
             "manager."),
        ANSI_TO_TCHAR(__FUNCTION__));
}

void FSyntheticSessionManager::OnRoomAdvertisementInviteAccepted(
    const UE::Online::FAccountId &LocalUserId,
    const FUserInfoPtr &SenderUser,
    const FRoomIdRef &RoomId,
    const FName &RoomNamespace)
{
    if (!RoomId->GetProviderName().IsEqual(FName(TEXT("Session"))) || !RoomNamespace.IsEqual(FName(TEXT("Synthetic"))))
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Verbose,
            TEXT("%s: Ignoring OnRoomAdvertisementInviteAccepted event as it is not for a synthetic session."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    auto World = FWorldResolution::GetWorld(this->PlatformHandle->GetEnvironment()->InstanceName, true);
    if (!World.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: Ignoring OnRoomAdvertisementInviteAccepted event as there is no current world."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    auto *OSS = Online::GetSubsystem(World.Get(), REDPOINT_EOS_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: Ignoring OnRoomAdvertisementInviteAccepted event as there is no current Redpoint EOS online "
                 "subsystem for this world."),
            ANSI_TO_TCHAR(__FUNCTION__));
        return;
    }

    UE_LOG(
        LogOnlineSubsystemRedpointEOS,
        Verbose,
        TEXT("Searching for session ID '%s' in response to room advertisement invite accepted, inside the synthetic "
             "session manager."),
        *RoomId->GetRoomId());
    OSS->GetSessionInterface()->FindSessionById(
        *GetUniqueNetIdRef(LocalUserId),
        *OSS->GetSessionInterface()->CreateSessionIdFromString(RoomId->GetRoomId()),
        *GetUniqueNetIdRef(LocalUserId),
        FOnSingleSessionResultCompleteDelegate::CreateSPLambda(
            this,
            [SessionWk = TWeakPtr<IOnlineSession>(OSS->GetSessionInterface()),
             IdentityByController = GetUniqueNetIdRef(LocalUserId),
             RoomId](int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult &SessionResult) {
                if (!bWasSuccessful)
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("Received invite from synthetic session, but could not resolve session ID to session "
                             "handle. Make sure the session is still publicly advertised."));
                    return;
                }

                auto Session = SessionWk.Pin();
                if (!Session.IsValid())
                {
                    UE_LOG(
                        LogOnlineSubsystemRedpointEOS,
                        Error,
                        TEXT("Received invite from synthetic session, but the session OSSv1 implementation was no "
                             "longer valid."));
                    return;
                }

                // Tell the game that we accepted the invite. This mirrors the same behaviour that you get when
                // accepting a session invite in the EOS overlay.
                UE_LOG(
                    LogOnlineSubsystemRedpointEOS,
                    Verbose,
                    TEXT("Triggering OnSessionUserInviteAcceptedDelegate for EOS sessions interface after search "
                         "result was found for FindSessionById inside the synthetic session manager."),
                    *RoomId->GetRoomId());
                Session->TriggerOnSessionUserInviteAcceptedDelegates(
                    true,
                    LocalUserNum,
                    IdentityByController,
                    SessionResult);
            }));
}

void FSyntheticSessionManager::OnStartSystem(const FIdentityUserRef &User, const FSimpleDelegate &OnComplete)
{
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, User->GetUserSlot());
    for (const auto &Integration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        this->OnRoomAdvertisementInviteReceivedDelegateHandles.FindOrAdd(User->GetUserSlot())
            .Add(
                Integration->GetPresenceService(),
                Integration->GetPresenceService()
                    ->OnRoomAdvertisementInviteReceived(CallContext)
                    .AddSP(this, &FSyntheticSessionManager::OnRoomAdvertisementInviteReceived));
        this->OnRoomAdvertisementInviteAcceptedDelegateHandles.FindOrAdd(User->GetUserSlot())
            .Add(
                Integration->GetPresenceService(),
                Integration->GetPresenceService()
                    ->OnRoomAdvertisementInviteAccepted(CallContext)
                    .AddSP(this, &FSyntheticSessionManager::OnRoomAdvertisementInviteAccepted));
    }
    OnComplete.ExecuteIfBound();
}

void FSyntheticSessionManager::OnStopSystem(const FIdentityUserRef &User, const FSimpleDelegate &OnComplete)
{
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, User->GetUserSlot());
    for (const auto &IntegrationKV :
         this->OnRoomAdvertisementInviteReceivedDelegateHandles.FindOrAdd(User->GetUserSlot()))
    {
        IntegrationKV.Key->OnRoomAdvertisementInviteReceived(CallContext).Remove(IntegrationKV.Value);
    }
    for (const auto &IntegrationKV :
         this->OnRoomAdvertisementInviteAcceptedDelegateHandles.FindOrAdd(User->GetUserSlot()))
    {
        IntegrationKV.Key->OnRoomAdvertisementInviteAccepted(CallContext).Remove(IntegrationKV.Value);
    }
    this->OnRoomAdvertisementInviteReceivedDelegateHandles.Remove(User->GetUserSlot());
    this->OnRoomAdvertisementInviteAcceptedDelegateHandles.Remove(User->GetUserSlot());
    OnComplete.ExecuteIfBound();
}

void FSyntheticSessionManager::UpsertSyntheticSession(
    const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
    int32 MaxPlayers,
    const FOnComplete &OnComplete)
{
    // We advertise the session for all local users, so that for split-screen games all of the users appear as in the
    // session.
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    FMultiOperation<FIdentityUserRef>::RunSP(
        this->AsShared(),
        IdentitySystem->GetUsers(),
        [this, SessionId, MaxPlayers](FIdentityUserRef User, TFunction<void()> OnUserDone) -> bool {
            // Construct the call context.
            auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, User->GetUserSlot());

            // Make an advertised "room" that we can advertise without a real room.
            auto AdvertisedRoom = FPresenceUserAdvertisedRoom(
                MakeShared<FRoomId>(FName(TEXT("Session")), SessionId->ToString()),
                FName(TEXT("Synthetic")));
            AdvertisedRoom.CurrentMemberCount = 1;
            AdvertisedRoom.MaximumMemberCount = static_cast<uint32>(MaxPlayers);
            AdvertisedRoom.bJoinable = true;
            AdvertisedRoom.SecretPassword = TEXT("");
            AdvertisedRoom.bRequiresSecretPassword = false;
            AdvertisedRoom.bIsMatch = true;
            AdvertisedRoom.bMatchStarted = false;

            // Advertise the room on all runtime platform integrations.
            FMultiOperation<TSharedRef<IRuntimePlatformIntegration>>::RunSP(
                this->AsShared(),
                FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                [CallContext, AdvertisedRoom](
                    TSharedRef<IRuntimePlatformIntegration> Integration,
                    TFunction<void()> OnIntegrationDone) -> bool {
                    Integration->GetPresenceService()->UpsertRoomAdvertisement(
                        CallContext,
                        AdvertisedRoom,
                        false,
                        IRuntimePlatformPresenceService::FOnUpsertRoomAdvertisementComplete::CreateLambda(
                            [OnIntegrationDone](FError ErrorCode) {
                                if (!ErrorCode.WasSuccessful())
                                {
                                    UE_LOG(
                                        LogOnlineSubsystemRedpointEOS,
                                        Warning,
                                        TEXT("Unable to advertise synthetic session on runtime platform: %s"),
                                        *ErrorCode.ToLogString());
                                }
                                OnIntegrationDone();
                            }));
                    return true;
                },
                OnUserDone);

            return true;
        },
        [OnComplete]() {
            OnComplete.ExecuteIfBound(Errors::Success());
        });
}

void FSyntheticSessionManager::DeleteSyntheticSession(
    const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
    const FOnComplete &OnComplete)
{
    // Remove the session advertisement for all local users.
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    FMultiOperation<FIdentityUserRef>::RunSP(
        this->AsShared(),
        IdentitySystem->GetUsers(),
        [this, SessionId](FIdentityUserRef User, TFunction<void()> OnUserDone) -> bool {
            // Construct the call context.
            auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, User->GetUserSlot());

            // Define the fake room ID that we would have used for the original advertisement.
            auto RoomId = MakeShared<FRoomId>(FName(TEXT("Session")), SessionId->ToString());

            // Remove the room advertisement on all runtime platform integrations.
            FMultiOperation<TSharedRef<IRuntimePlatformIntegration>>::RunSP(
                this->AsShared(),
                FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                [CallContext, RoomId](
                    TSharedRef<IRuntimePlatformIntegration> Integration,
                    TFunction<void()> OnIntegrationDone) -> bool {
                    Integration->GetPresenceService()->DeleteRoomAdvertisement(
                        CallContext,
                        RoomId,
                        false,
                        IRuntimePlatformPresenceService::FOnDeleteRoomAdvertisementComplete::CreateLambda(
                            [OnIntegrationDone](FError ErrorCode) {
                                if (!ErrorCode.WasSuccessful())
                                {
                                    UE_LOG(
                                        LogOnlineSubsystemRedpointEOS,
                                        Warning,
                                        TEXT("Unable to advertise synthetic session on runtime platform: %s"),
                                        *ErrorCode.ToLogString());
                                }
                                OnIntegrationDone();
                            }));
                    return true;
                },
                OnUserDone);

            return true;
        },
        [OnComplete]() {
            OnComplete.ExecuteIfBound(Errors::Success());
        });
}

void FSyntheticSessionManager::SendInvitationToSession(
    int32 LocalUserNum,
    const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId,
    const TSharedPtr<const FUniqueNetId> &RecipientId,
    const FOnComplete &OnComplete)
{
    if (!SessionId.IsValid() || !RecipientId.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("One or more parameters for SendInvitationToSession were invalid!")));
        return;
    }

    // Get the local user that will be sending this invitation.
    auto IdentitySystem = this->PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(LocalUserNum);
    if (!IdentityUser.IsValid())
    {
        OnComplete.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("There is no local user signed in to send the invitation from.")));
        return;
    }

    // Construct the call context.
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, IdentityUser->GetUserSlot());

    // Define the fake room ID that we'll send.
    auto RoomId = MakeShared<FRoomId>(FName(TEXT("Session")), SessionId->ToString());

    // Figure out how the recipient ID maps to EOS_EExternalAccountType. The legacy session implementation already
    // handles wrangling the unified/non-unified/synthetic friend IDs into recipient IDs that represent the original
    // OSSv1 subsystem we'd intend the send to go to.
    using FInfo = TTuple<EOS_EExternalAccountType, FString>;
    TOptional<FInfo> RecipientExternalInfo;
    if (RecipientId->GetType().IsEqual(STEAM_SUBSYSTEM))
    {
        RecipientExternalInfo = FInfo(EOS_EExternalAccountType::EOS_EAT_STEAM, RecipientId->ToString());
    }
    else if (RecipientId->GetType().IsEqual(FName(TEXT("GDK"))))
    {
        RecipientExternalInfo = FInfo(EOS_EExternalAccountType::EOS_EAT_XBL, RecipientId->ToString());
    }
    else if (
        RecipientId->GetType().IsEqual(FName(TEXT("PS4"))) || RecipientId->GetType().IsEqual(FName(TEXT("PS5"))) ||
        RecipientId->GetType().IsEqual(FName(TEXT("Sony"))))
    {
        RecipientExternalInfo = FInfo(EOS_EExternalAccountType::EOS_EAT_PSN, RecipientId->ToString());
    }
    else if (RecipientId->GetType().IsEqual(FName(TEXT("Switch"))))
    {
        RecipientExternalInfo = FInfo(EOS_EExternalAccountType::EOS_EAT_NINTENDO, RecipientId->ToString());
    }
    if (!RecipientExternalInfo.IsSet())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: The recipient user ID is not of a supported OSSv1 type ('%s')."),
            ANSI_TO_TCHAR(__FUNCTION__),
            *RecipientId->GetType().ToString());
        OnComplete.ExecuteIfBound(Errors::InvalidRequest(
            ANSI_TO_TCHAR(__FUNCTION__),
            FString::Printf(
                TEXT("The recipient user ID is not of a supported OSSv1 type ('%s')."),
                *RecipientId->GetType().ToString())));
        return;
    }

    // Send the room invite via runtime platform integrations.
    auto PresenceSystem = this->PlatformHandle->GetSystem<IPresenceSystem>();
    PresenceSystem->WaitForReconciledPresence(
        IdentityUser->GetUserId(),
        FSimpleDelegate::CreateSPLambda(
            this,
            [this, CallContext, RecipientExternalInfo, RoomId, OnComplete]() {
                FMultiOperation<TSharedRef<IRuntimePlatformIntegration>>::RunSP(
                    this->AsShared(),
                    FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations(),
                    [CallContext, RecipientExternalInfo, RoomId](
                        TSharedRef<IRuntimePlatformIntegration> Integration,
                        TFunction<void()> OnIntegrationDone) -> bool {
                        Integration->GetPresenceService()->SendRoomInviteToNonUnifiedFriend(
                            CallContext,
                            RecipientExternalInfo->Get<0>(),
                            RecipientExternalInfo->Get<1>(),
                            RoomId,
                            IRuntimePlatformPresenceService::FOnSendRoomInviteToNonUnifiedFriendComplete::CreateLambda(
                                [OnIntegrationDone](FError ErrorCode) {
                                    if (!ErrorCode.WasSuccessful())
                                    {
                                        UE_LOG(
                                            LogOnlineSubsystemRedpointEOS,
                                            Warning,
                                            TEXT("Unable to send session invite over runtime platform: %s"),
                                            *ErrorCode.ToLogString());
                                    }
                                    OnIntegrationDone();
                                }));
                        return true;
                    },
                    [OnComplete]() {
                        OnComplete.ExecuteIfBound(Errors::Success());
                    });
            }));
}

bool FSyntheticSessionManager::OpenSessionInvitationUI(
    int32 LocalUserNum,
    const TSharedPtr<const FUniqueNetIdEOSSession> &SessionId)
{
    if (!SessionId.IsValid())
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Warning,
            TEXT("One or more parameters for OpenSessionInvitationUI were invalid!"));
        return false;
    }

    // Construct the call context.
    auto CallContext = ConstructRuntimePlatformServiceCallContext(this->PlatformHandle, LocalUserNum);

    // Define the fake room ID that we'll send.
    auto RoomId = MakeShared<FRoomId>(FName(TEXT("Session")), SessionId->ToString());

    // Try to open the UI via the runtime platform integrations.
    bool bAnySuccessful = false;
    for (const auto &Integration :
         FModule::GetModuleChecked().GetRuntimePlatformRegistry()->GetRuntimePlatformIntegrations())
    {
        if (Integration->GetPresenceService()->TryOpenRoomInviteRecipientSelection(CallContext, RoomId))
        {
            bAnySuccessful = true;
        }
    }
    return bAnySuccessful;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()