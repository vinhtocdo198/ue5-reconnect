// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Lobby/NotifyJoinLobbyAccepted.h"
#include "RedpointEOSAPI/Lobby/NotifyLeaveLobbyRequested.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyInviteAccepted.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyInviteReceived.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyInviteRejected.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberStatusReceived.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyMemberUpdateReceived.h"
#include "RedpointEOSAPI/Lobby/NotifyLobbyUpdateReceived.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSRooms/Providers/RoomProvider.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(293683484, Redpoint::EOS::Rooms, IRoomSystemImpl)

namespace REDPOINT_EOS_FILE_NS_ID(4289788093, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Rooms::Providers;
using namespace ::Redpoint::EOS::API;

extern const FName LobbyRoomProviderName;

class FLobbyRoomProvider : public FRoomProvider, public Redpoint::EOS::Core::Utils::IHasEventRegistration
{
private:
    IRoomSystemImpl *RoomSystemImpl;
    FPlatformHandle PlatformHandle;
    FOnRoomJoinRequestedFromSDKEvent OnRoomJoinRequestedFromSDKEventValue;
    FOnRoomLeaveRequestedFromSDKEvent OnRoomLeaveRequestedFromSDKEventValue;
    FOnRoomInviteLocallyReceivedFromSDKEvent OnRoomInviteLocallyReceivedFromSDKEventValue;
    FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEventValue;
    FOnRoomInviteLocallyFullyRejectedFromSDKEvent OnRoomInviteLocallyFullyRejectedFromSDKEventValue;

    FEventHandlePtr OnNativeLobbyUpdateReceivedEventHandle;
    FEventHandlePtr OnNativeLobbyMemberUpdateReceivedEventHandle;
    FEventHandlePtr OnNativeLobbyMemberStatusReceivedEventHandle;
    FEventHandlePtr OnNativeLobbyInviteReceivedEventHandle;
    FEventHandlePtr OnNativeLobbyInviteAcceptedEventHandle;
    FEventHandlePtr OnNativeLobbyInviteRejectedEventHandle;
    FEventHandlePtr OnNativeJoinLobbyAcceptedEventHandle;
    FEventHandlePtr OnNativeLeaveLobbyRequestedEventHandle;

public:
    FLobbyRoomProvider(IRoomSystemImpl *InRoomSystemImpl, const FPlatformHandle &InPlatformHandle);
    UE_NONCOPYABLE(FLobbyRoomProvider);
    virtual ~FLobbyRoomProvider() override = default;

    virtual void RegisterEvents() override;

    virtual FName GetProviderName() const override;

private:
    void OnNativeLobbyUpdateReceived(const Redpoint::EOS::API::Lobby::FNotifyLobbyUpdateReceived::Result &Result);
    void OnNativeLobbyMemberUpdateReceived(
        const Redpoint::EOS::API::Lobby::FNotifyLobbyMemberUpdateReceived::Result &Result);
    void OnNativeLobbyMemberStatusReceived(
        const Redpoint::EOS::API::Lobby::FNotifyLobbyMemberStatusReceived::Result &Result);
    void OnNativeLobbyInviteReceived(const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteReceived::Result &Result);
    void OnNativeLobbyInviteAccepted(const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteAccepted::Result &Result);
    void OnNativeLobbyInviteRejected(const Redpoint::EOS::API::Lobby::FNotifyLobbyInviteRejected::Result &Result);
    void OnNativeJoinLobbyAccepted(const Redpoint::EOS::API::Lobby::FNotifyJoinLobbyAccepted::Result &Result);
    void OnNativeLeaveLobbyRequested(const Redpoint::EOS::API::Lobby::FNotifyLeaveLobbyRequested::Result &Result);

protected:
    virtual FRoomProviderWriteOperationRef CreateCreateRoomOperation(
        const FRoomUserId &UserId,
        const FName &RoomNamespace) override;
    virtual void ExecuteCreateRoomOperation(
        const FRoomProviderWriteOperationRef &CreateOperation,
        const FOnExecuteCreateRoomOperationComplete &OnComplete) override;
    virtual void ExecutePostCreateRoomOperation(
        const FRoomProviderWriteOperationRef &PostCreateOperation,
        const FOnExecutePostCreateRoomOperationComplete &OnComplete) override;
    virtual void CancelPostCreateRoomOperation(
        const FRoomProviderWriteOperationRef &PostCreateOperation,
        const FOnCancelPostCreateRoomOperationComplete &OnComplete) override;

    virtual FRoomProviderWriteOperationRef CreateUpdateRoomOperation(const FRoomUserId &UserId, const FRoomRef &Room)
        override;
    virtual void ExecuteUpdateRoomOperation(
        const FRoomProviderWriteOperationRef &UpdateOperation,
        const FOnExecuteUpdateRoomOperationComplete &OnComplete) override;

    virtual FRoomProviderSearchOperationRef CreateSearchRoomsOperation(
        const FRoomUserId &UserId,
        TOptional<uint32> MaxResults) override;
    virtual void ExecuteSearchRoomsOperation(
        const FRoomProviderSearchOperationRef &SearchOperation,
        const FOnExecuteSearchRoomsOperationComplete &OnComplete) override;

private:
    void ExecuteSearchRoomsOperationWithTimeout(
        const FRoomProviderSearchOperationRef &SearchOperation,
        const FOnExecuteSearchRoomsOperationComplete &OnComplete,
        const float CurrentAttemptCount);

protected:
    virtual FRoomProviderWriteOperationRef CreateJoinRoomByIdOperation(
        const FRoomUserId &UserId,
        const FRoomIdRef &RoomId) override;
    virtual FRoomProviderWriteOperationRef CreateJoinRoomBySearchResultOperation(
        const FRoomUserId &UserId,
        const FRoomSearchResultRef &RoomSearchResult) override;
    virtual void ExecuteJoinRoomOperation(
        const FRoomProviderWriteOperationRef &JoinOperation,
        const FOnExecuteJoinRoomOperationComplete &OnComplete) override;
    virtual void ExecutePostJoinRoomOperation(
        const FRoomProviderWriteOperationRef &PostJoinOperation,
        const FOnExecutePostJoinRoomOperationComplete &OnComplete) override;
    virtual void CancelPostJoinRoomOperation(
        const FRoomProviderWriteOperationRef &PostJoinOperation,
        const FOnCancelPostJoinRoomOperationComplete &OnComplete) override;

    virtual FRoomProviderActionOperationRef CreateLeaveRoomOperation(const FRoomUserId &UserId, const FRoomRef &Room)
        override;
    virtual void ExecuteLeaveRoomOperation(
        const FRoomProviderActionOperationRef &LeaveOperation,
        const FOnExecuteLeaveRoomOperationComplete &OnComplete) override;

    virtual FRoomProviderActionOperationRef CreatePromoteRoomMemberOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) override;
    virtual void ExecutePromoteRoomMemberOperation(
        const FRoomProviderActionOperationRef &PromoteOperation,
        const FOnExecutePromoteRoomMemberOperationComplete &OnComplete) override;

    virtual FRoomProviderActionOperationRef CreateKickRoomMemberOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) override;
    virtual void ExecuteKickRoomMemberOperation(
        const FRoomProviderActionOperationRef &KickOperation,
        const FOnExecuteKickRoomMemberOperationComplete &OnComplete) override;

    virtual FRoomProviderActionOperationRef CreateSendRoomInviteOperation(
        const FRoomUserId &UserId,
        const FRoomRef &Room,
        const FRoomUserId &TargetUserId) override;
    virtual void ExecuteSendRoomInviteOperation(
        const FRoomProviderActionOperationRef &KickOperation,
        const FOnExecuteSendRoomInviteOperationComplete &OnComplete) override;

    virtual FRoomProviderReadOperationRef CreateReadRoomOperation(const FRoomUserId &UserId, const FRoomRef &Room)
        override;

public:
    virtual void RestoreRoomInvites(
        const FRestoreRoomInvitesRequest &Request,
        const IRoomSystem::FOnRestoreRoomInvitesComplete &OnComplete) override;

    virtual void RejectRoomInvite(
        const FRoomInviteRef &RoomInvite,
        const IRoomSystem::FOnRejectRoomInviteComplete &OnComplete) override;

    virtual FOnRoomJoinRequestedFromSDKEvent &OnRoomJoinRequestedFromSDKEvent() override;
    virtual FOnRoomLeaveRequestedFromSDKEvent &OnRoomLeaveRequestedFromSDKEvent() override;
    virtual FOnRoomInviteLocallyReceivedFromSDKEvent &OnRoomInviteLocallyReceivedFromSDKEvent() override;
    virtual FOnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent &OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent()
        override;
    virtual FOnRoomInviteLocallyFullyRejectedFromSDKEvent &OnRoomInviteLocallyFullyRejectedFromSDKEvent() override;

    virtual void Reset() override;
};

}

namespace Redpoint::EOS::Rooms::Providers::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(4289788093, Redpoint::EOS::Rooms::Providers::Lobby, LobbyRoomProviderName)
REDPOINT_EOS_FILE_NS_EXPORT(4289788093, Redpoint::EOS::Rooms::Providers::Lobby, FLobbyRoomProvider)
}

REDPOINT_EOS_CODE_GUARD_END()