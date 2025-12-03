// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatformIntegrationEpic/Services/EpicRuntimePlatformPresenceService.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Presence/CopyPresence.h"
#include "RedpointEOSAPI/Presence/CreatePresenceModification.h"
#include "RedpointEOSAPI/Presence/NotifyOnPresenceChanged.h"
#include "RedpointEOSAPI/Presence/QueryPresence.h"
#include "RedpointEOSAPI/Presence/SetPresence.h"
#include "RedpointEOSAPI/PresenceModification/SetData.h"
#include "RedpointEOSAPI/PresenceModification/SetJoinInfo.h"
#include "RedpointEOSAPI/PresenceModification/SetRawRichText.h"
#include "RedpointEOSAPI/PresenceModification/SetStatus.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountId.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSPlatformIntegrationEpic/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3348887799, Redpoint::EOS::Platform::Integration::Epic::Services)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Presence;
using namespace ::Redpoint::EOS::API::PresenceModification;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Platform::Integration::Epic;

FEpicRuntimePlatformPresenceService::FEpicRuntimePlatformPresenceService()
    : PresenceDelegatesByPlatform()
{
}

TOptional<bool> FEpicRuntimePlatformPresenceService::IsLocalUserOnline(
    const FRuntimePlatformServiceCallContextRef &CallContext) const
{
    return TOptional<bool>();
}

void FEpicRuntimePlatformPresenceService::UpdateLocalUserPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserState &InDesiredPresenceState,
    const FOnLocalUserPresenceUpdated &InCompletionDelegate)
{
    // Get the Epic Games account ID from the identity system.
    auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
    auto LocalUser = IdentitySystem->GetUser(CallContext->LocalUserNum);
    if (!LocalUser.IsValid())
    {
        InCompletionDelegate.ExecuteIfBound(
            Errors::AccessDenied(ANSI_TO_TCHAR(__FUNCTION__), TEXT("The local user is not signed in.")));
        return;
    }

    auto CrossPlatformAccountId = LocalUser->GetCrossPlatformAccountId();
    if (!CrossPlatformAccountId.IsValid() || !CrossPlatformAccountId->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID))
    {
        InCompletionDelegate.ExecuteIfBound(Errors::AccessDenied(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The local user is not signed into an Epic Games account.")));
        return;
    }

    auto EpicGamesAccountId =
        StaticCastSharedRef<const FEpicGamesCrossPlatformAccountId>(CrossPlatformAccountId.ToSharedRef())
            ->GetEpicAccountId();

    // Create the presence modification handle.
    EOS_EResult ResultCode;
    auto Modification = FCreatePresenceModification::Execute(
        CallContext->PlatformHandle,
        FCreatePresenceModification::Options{EpicGamesAccountId},
        ResultCode);
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        InCompletionDelegate.ExecuteIfBound(ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to create presence modification handle."),
            ResultCode));
        return;
    }

    // Set the activity status for presence.
    EOS_Presence_EStatus Status;
    switch (InDesiredPresenceState.ActivityState)
    {
    case EPresenceUserActivityState::Offline:
        Status = EOS_Presence_EStatus::EOS_PS_Offline;
        break;
    case EPresenceUserActivityState::DoNotDisturb:
        Status = EOS_Presence_EStatus::EOS_PS_DoNotDisturb;
        break;
    case EPresenceUserActivityState::Idle:
        Status = EOS_Presence_EStatus::EOS_PS_Away;
        break;
    case EPresenceUserActivityState::Online:
    default:
        Status = EOS_Presence_EStatus::EOS_PS_Online;
        break;
    }
    ResultCode = FSetStatus::Execute(Modification, FSetStatus::Options{Status});
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        InCompletionDelegate.ExecuteIfBound(ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to set presence status on presence modification handle."),
            ResultCode));
        return;
    }

    // Set the rich text presence status.
    ResultCode = FSetRawRichText::Execute(
        Modification,
        FSetRawRichText::Options{InDesiredPresenceState.GetStatusTextFormatted().ToString()});
    if (ResultCode != EOS_EResult::EOS_Success)
    {
        InCompletionDelegate.ExecuteIfBound(ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to set presence rich text on presence modification handle."),
            ResultCode));
        return;
    }

    // Set custom data.
    FString JoinInfo;
    TMap<FString, FString> Records;
    for (const auto &KV : InDesiredPresenceState.CustomAttributes)
    {
        if (KV.Key == TEXT("JoinInfo_JoinData"))
        {
            JoinInfo = KV.Value.GetString();
        }
        else if (KV.Value.Type() == FRoomAttribute::EType::String)
        {
            Records.Add(KV.Key, KV.Value.GetString());
        }
    }
    if (Records.Num() > 0)
    {
        ResultCode = FSetData::Execute(Modification, FSetData::Options{Records});
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            InCompletionDelegate.ExecuteIfBound(ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to set presence custom data on presence modification handle."),
                ResultCode));
            return;
        }
    }

    // @todo: Delete old custom data.

    // Set the join info if available.
    if (!JoinInfo.IsEmpty())
    {
        ResultCode = FSetJoinInfo::Execute(Modification, FSetJoinInfo::Options{JoinInfo});
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            InCompletionDelegate.ExecuteIfBound(ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to set presence custom join info on presence modification handle."),
                ResultCode));
            return;
        }
    }

    // Apply the presence update.
    FSetPresence::Execute(
        CallContext->PlatformHandle,
        FSetPresence::Options{EpicGamesAccountId, Modification},
        FSetPresence::CompletionDelegate::CreateSPLambda(
            this,
            [InCompletionDelegate](const FSetPresence::Result &InResult) {
                if (InResult.ResultCode == EOS_EResult::EOS_Success)
                {
                    InCompletionDelegate.ExecuteIfBound(Errors::Success());
                }
                else
                {
                    InCompletionDelegate.ExecuteIfBound(ConvertError(
                        ANSI_TO_TCHAR(__FUNCTION__),
                        TEXT("Unable to update presence."),
                        InResult.ResultCode));
                }
            }));
}

void FEpicRuntimePlatformPresenceService::UpsertRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FPresenceUserAdvertisedRoom &InAdvertisedRoom,
    bool bIsParty,
    const FOnUpsertRoomAdvertisementComplete &InCompletionDelegate)
{
    // @note: Epic doesn't need room advertisements since they're controlled directly by the EOS SDK. We also don't need
    // to fire received or accepted events because the EOS SDK fires events for us for the overlay.
    InCompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FEpicRuntimePlatformPresenceService::DeleteRoomAdvertisement(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &InRoomId,
    bool bIsParty,
    const FOnDeleteRoomAdvertisementComplete &InCompletionDelegate)
{
    // @note: Epic doesn't need room advertisements since they're controlled directly by the EOS SDK.
    InCompletionDelegate.ExecuteIfBound(Errors::Success());
}

void FEpicRuntimePlatformPresenceService::OnPlatformShutdown(
    FPlatformInstance &Instance,
    FPlatformHandle PlatformHandle)
{
    this->PresenceDelegatesByPlatform.Remove(PlatformHandle);
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteReceived &FEpicRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteReceived(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    if (!this->PresenceDelegatesByPlatform.Contains(CallContext->PlatformHandle))
    {
        this->PresenceDelegatesByPlatform.Add(CallContext->PlatformHandle, FDelegateStorage());
        CallContext->PlatformHandle->OnShutdown().AddSP(
            this->AsShared(),
            &FEpicRuntimePlatformPresenceService::OnPlatformShutdown,
            CallContext->PlatformHandle);
    }
    return this->PresenceDelegatesByPlatform[CallContext->PlatformHandle]
        .OnRoomAdvertisementInviteReceivedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

IRuntimePlatformPresenceService::FOnRoomAdvertisementInviteAccepted &FEpicRuntimePlatformPresenceService::
    OnRoomAdvertisementInviteAccepted(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    if (!this->PresenceDelegatesByPlatform.Contains(CallContext->PlatformHandle))
    {
        this->PresenceDelegatesByPlatform.Add(CallContext->PlatformHandle, FDelegateStorage());
        CallContext->PlatformHandle->OnShutdown().AddSP(
            this->AsShared(),
            &FEpicRuntimePlatformPresenceService::OnPlatformShutdown,
            CallContext->PlatformHandle);
    }
    return this->PresenceDelegatesByPlatform[CallContext->PlatformHandle]
        .OnRoomAdvertisementInviteAcceptedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

TOptional<EPresenceUserActivityState> FEpicRuntimePlatformPresenceService::GetNonUnifiedFriendPresence(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId)
{
    if (FriendAccountType != EOS_EExternalAccountType::EOS_EAT_EPIC)
    {
        return TOptional<EPresenceUserActivityState>();
    }

    auto IdentitySystem = CallContext->PlatformHandle->GetSystem<IIdentitySystem>();
    auto LocalUser = IdentitySystem->GetUser(CallContext->LocalUserNum);
    if (!LocalUser.IsValid() || !LocalUser->GetCrossPlatformAccountId().IsValid() ||
        !LocalUser->GetCrossPlatformAccountId()->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID))
    {
        return TOptional<EPresenceUserActivityState>();
    }

    EOS_EResult CopyResultCode;
    FCopyPresence::Result CopyResult;
    FCopyPresence::Execute(
        CallContext->PlatformHandle,
        FCopyPresence::Options{
            StaticCastSharedPtr<const FEpicGamesCrossPlatformAccountId>(LocalUser->GetCrossPlatformAccountId())
                ->GetEpicAccountId(),
            GetEpicGamesAccountId(FriendAccountId)},
        CopyResultCode,
        CopyResult);
    if (CopyResultCode == EOS_EResult::EOS_Success)
    {
        EPresenceUserActivityState State = EPresenceUserActivityState::Offline;
        switch (CopyResult.Status)
        {
        case EOS_Presence_EStatus::EOS_PS_Online:
            State = EPresenceUserActivityState::Online;
            break;
        case EOS_Presence_EStatus::EOS_PS_Away:
        case EOS_Presence_EStatus::EOS_PS_ExtendedAway:
            State = EPresenceUserActivityState::Idle;
            break;
        case EOS_Presence_EStatus::EOS_PS_DoNotDisturb:
            State = EPresenceUserActivityState::DoNotDisturb;
            break;
        default:
            State = EPresenceUserActivityState::Offline;
            break;
        }
        return State;
    }
    else if (CopyResultCode == EOS_EResult::EOS_NotFound)
    {
        UE_LOG(
            LogRedpointEOSPlatformIntegrationEpic,
            Error,
            TEXT("The local Epic account '%s' does not have cached presence for target Epic account '%s'."),
            *GetEpicGamesUserIdString(
                StaticCastSharedPtr<const FEpicGamesCrossPlatformAccountId>(LocalUser->GetCrossPlatformAccountId())
                    ->GetEpicAccountId()),
            *GetEpicGamesUserIdString(GetEpicGamesAccountId(FriendAccountId)),
            (int)CopyResult.Status);
    }

    return TOptional<EPresenceUserActivityState>();
}

IRuntimePlatformPresenceService::FOnNonUnifiedFriendPresenceChanged &FEpicRuntimePlatformPresenceService::
    OnNonUnifiedFriendPresenceChanged(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    if (!this->PresenceDelegatesByPlatform.Contains(CallContext->PlatformHandle))
    {
        this->PresenceDelegatesByPlatform.Add(CallContext->PlatformHandle, FDelegateStorage());
        CallContext->PlatformHandle->OnShutdown().AddSP(
            this->AsShared(),
            &FEpicRuntimePlatformPresenceService::OnPlatformShutdown,
            CallContext->PlatformHandle);
    }
    return this->PresenceDelegatesByPlatform[CallContext->PlatformHandle]
        .OnNonUnifiedFriendPresenceChangedDelegate.FindOrAdd(CallContext->LocalUserNum);
}

void FEpicRuntimePlatformPresenceService::SendRoomInviteToNonUnifiedFriend(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const EOS_EExternalAccountType &FriendAccountType,
    const FString &FriendAccountId,
    const FRoomIdRef &RoomId,
    const FOnSendRoomInviteToNonUnifiedFriendComplete &InCompletionDelegate)
{
    // @note: Epic doesn't need room advertisements since they're controlled directly by the EOS SDK. We also don't need
    // to fire received or accepted events because the EOS SDK fires events for us for the overlay.
    //
    // We return NotImplemented here so the original caller will try to send the invite via the EOS SDK.
    InCompletionDelegate.ExecuteIfBound(Errors::NotImplemented(
        ANSI_TO_TCHAR(__FUNCTION__),
        TEXT("Sending an invite to an Epic Games friend should happen by calling the EOS SDK. It is not possible to "
             "programmatically send invites to Epic Games friends who have never played the game.")));
}

bool FEpicRuntimePlatformPresenceService::TryOpenRoomInviteRecipientSelection(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FRoomIdRef &RoomId)
{
    // Epic does not have an overlay for inviting friends to a specific session/party.
    return false;
}

void FEpicRuntimePlatformPresenceService::RegisterEvents(
    const FRuntimePlatformServiceCallContextRef &CallContext,
    const FSimpleDelegate &OnComplete)
{
    if (!this->PresenceDelegatesByPlatform.Contains(CallContext->PlatformHandle))
    {
        this->PresenceDelegatesByPlatform.Add(CallContext->PlatformHandle, FDelegateStorage());
        CallContext->PlatformHandle->OnShutdown().AddSP(
            this->AsShared(),
            &FEpicRuntimePlatformPresenceService::OnPlatformShutdown,
            CallContext->PlatformHandle);
    }

    this->PresenceDelegatesByPlatform[CallContext->PlatformHandle].OnPresenceChanged =
        FNotifyOnPresenceChanged::Register(
            CallContext->PlatformHandle,
            FNotifyOnPresenceChanged::Options{},
            FNotifyOnPresenceChanged::EventDelegate::CreateSPLambda(
                this,
                [this, CallContext](const FNotifyOnPresenceChanged::Result &Result) {
                    if (GetEpicGamesUserIdString(Result.LocalUserId) == GetEpicGamesUserIdString(Result.PresenceUserId))
                    {
                        // We do not fire this event when the presence user is the local user; this event only fires for
                        // friends.
                        return;
                    }

                    EOS_EResult CopyResultCode;
                    FCopyPresence::Result CopyResult;
                    FCopyPresence::Execute(
                        CallContext->PlatformHandle,
                        FCopyPresence::Options{Result.LocalUserId, Result.PresenceUserId},
                        CopyResultCode,
                        CopyResult);
                    if (CopyResultCode == EOS_EResult::EOS_Success)
                    {
                        EPresenceUserActivityState State = EPresenceUserActivityState::Offline;
                        switch (CopyResult.Status)
                        {
                        case EOS_Presence_EStatus::EOS_PS_Online:
                            State = EPresenceUserActivityState::Online;
                            break;
                        case EOS_Presence_EStatus::EOS_PS_Away:
                        case EOS_Presence_EStatus::EOS_PS_ExtendedAway:
                            State = EPresenceUserActivityState::Idle;
                            break;
                        case EOS_Presence_EStatus::EOS_PS_DoNotDisturb:
                            State = EPresenceUserActivityState::DoNotDisturb;
                            break;
                        default:
                            State = EPresenceUserActivityState::Offline;
                            break;
                        }
                        UE_LOG(
                            LogRedpointEOSPlatformIntegrationEpic,
                            Verbose,
                            TEXT("Notified that local Epic account '%s' has target Epic account '%s' in new presence "
                                 "state of %d."),
                            *GetEpicGamesUserIdString(Result.LocalUserId),
                            *GetEpicGamesUserIdString(Result.PresenceUserId),
                            (int)CopyResult.Status);
                        this->OnNonUnifiedFriendPresenceChanged(CallContext)
                            .Broadcast(
                                CallContext->LocalUserNum,
                                EOS_EExternalAccountType::EOS_EAT_EPIC,
                                GetEpicGamesUserIdString(Result.PresenceUserId),
                                State);
                    }
                    else
                    {
                        UE_LOG(
                            LogRedpointEOSPlatformIntegrationEpic,
                            Error,
                            TEXT("The local Epic account '%s' does not have cached presence for target Epic account "
                                 "'%s', even after we received the NotifyOnPresenceChanged event for the target "
                                 "account."),
                            *GetEpicGamesUserIdString(Result.LocalUserId),
                            *GetEpicGamesUserIdString(Result.PresenceUserId));
                    }
                }));
    OnComplete.ExecuteIfBound();
}

void FEpicRuntimePlatformPresenceService::UnregisterEvents(const FRuntimePlatformServiceCallContextRef &CallContext)
{
    this->PresenceDelegatesByPlatform.Remove(CallContext->PlatformHandle);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()