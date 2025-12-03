// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "RedpointEOSRooms/RoomMap.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSRooms/RoomSystem.h"

class FOnlineSubsystemEOS;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2303545192, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineLobbyInterfaceEOS
    : public IOnlineLobby,
      public TSharedFromThis<FOnlineLobbyInterfaceEOS, ESPMode::ThreadSafe>
{
    friend FOnlineSubsystemEOS;

private:
    TSharedRef<IRoomSystem> RoomSystem;
    TWeakPtr<FOnlineSubsystemEOS, ESPMode::ThreadSafe> EOSSubsystem;
    TSharedPtr<FEOSVoiceManager> VoiceManager;
    TUserIdMap<TMap<FString, FRoomSearchResultPtr>> LastSearchResultsByUser;
    FDelegateHandle OnRoomUpdatedHandle;
    FDelegateHandle OnRoomLeftHandle;
    TRoomMap<FDelegateHandle> OnRoomMemberJoinedHandles;
    TRoomMap<FDelegateHandle> OnRoomMemberUpdatedHandles;
    TRoomMap<FDelegateHandle> OnRoomMemberPromotedHandles;
    TRoomMap<FDelegateHandle> OnRoomMemberLeftHandles;

public:
    FOnlineLobbyInterfaceEOS(
        const TSharedRef<IRoomSystem> &InRoomSystem,
        const TSharedRef<class FOnlineSubsystemEOS, ESPMode::ThreadSafe> &InSubsystem,
        const TSharedRef<FEOSVoiceManager> &InVoiceManager);
    UE_NONCOPYABLE(FOnlineLobbyInterfaceEOS);
    virtual ~FOnlineLobbyInterfaceEOS();
    void RegisterEvents();

private:
    void RegisterEventsForNewRoom(FRoomRef Room);
    void OnRoomUpdated(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        const FRoomUpdateInfo &RoomUpdateInfo);
    void OnRoomLeft(const FRoomEventContext &EventContext, const FRoomRef &Room, ERoomLeaveReason Reason);
    void OnRoomMemberJoined(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser);
    void OnRoomMemberUpdated(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        const FRoomMemberUpdateInfo &RoomMemberUpdateInfo);
    void OnRoomMemberPromoted(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser);
    void OnRoomMemberLeft(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        ERoomLeaveReason LeaveReason);

    FRoomPtr GetCurrentRoomByPredicate(FRoomUserId UserId, TFunction<bool(const FRoomRef &Room)> Predicate);

public:
    virtual FDateTime GetUtcNow() override;

    virtual TSharedPtr<FOnlineLobbyTransaction> MakeCreateLobbyTransaction(const FUniqueNetId &UserId) override;
    virtual TSharedPtr<FOnlineLobbyTransaction> MakeUpdateLobbyTransaction(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId) override;
    virtual TSharedPtr<FOnlineLobbyMemberTransaction> MakeUpdateLobbyMemberTransaction(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId) override;

    virtual bool CreateLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyTransaction &Transaction,
        FOnLobbyCreateOrConnectComplete OnComplete = FOnLobbyCreateOrConnectComplete()) override;
    virtual bool UpdateLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FOnlineLobbyTransaction &Transaction,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;
    virtual bool DeleteLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool ConnectLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyCreateOrConnectComplete OnComplete = FOnLobbyCreateOrConnectComplete()) override;
    virtual bool DisconnectLobby(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool UpdateMemberSelf(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FOnlineLobbyMemberTransaction &Transaction,
        FOnLobbyOperationComplete OnComplete = FOnLobbyOperationComplete()) override;

    virtual bool GetMemberCount(const FUniqueNetId &UserId, const FOnlineLobbyId &LobbyId, int32 &OutMemberCount)
        override;
    virtual bool GetMemberUserId(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        int32 MemberIndex,
        TSharedPtr<const FUniqueNetId> &OutMemberId) override;

    virtual bool GetMemberMetadataValue(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        const FString &MetadataKey,
        FVariantData &OutMetadataValue) override;

    virtual bool Search(
        const FUniqueNetId &UserId,
        const FOnlineLobbySearchQuery &Query,
        FOnLobbySearchComplete OnComplete = FOnLobbySearchComplete()) override;

    virtual bool GetLobbyMetadataValue(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FString &MetadataKey,
        FVariantData &OutMetadataValue) override;

    virtual TSharedPtr<FOnlineLobbyId> ParseSerializedLobbyId(const FString &InLobbyId) override;

    virtual bool KickMember(
        const FUniqueNetId &UserId,
        const FOnlineLobbyId &LobbyId,
        const FUniqueNetId &MemberId,
        FOnLobbyOperationComplete OnComplete) override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2303545192,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Lobby,
    FOnlineLobbyInterfaceEOS)
}

#endif