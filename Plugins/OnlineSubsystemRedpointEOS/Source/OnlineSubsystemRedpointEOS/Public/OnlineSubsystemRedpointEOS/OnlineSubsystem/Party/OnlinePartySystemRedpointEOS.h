// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlinePartyInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdMapRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyRequestToJoinInfoRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Social/OnlineSocialInterfaceEOS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineUserEOS.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdMap.h"
#include "RedpointEOSCore/Utils/HasEventRegistration.h"
#include "RedpointEOSCore/Utils/IndexedTable.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(415509281, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Social;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::VoiceChat;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS;

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlinePartySystemRedpointEOS
    : public IOnlinePartySystem,
      public Redpoint::EOS::Core::Utils::IHasEventRegistration,
      public TSharedFromThis<FOnlinePartySystemRedpointEOS>
{
private:
    FPlatformHandle PlatformHandle;
    TSharedRef<IRoomSystem> RoomSystem;
    TSharedRef<FEOSUserFactory> UserFactory;
    TSharedRef<FEOSVoiceManager> VoiceManager;
    TWeakPtr<FOnlineSocialInterfaceEOS> Social;
    Core::Utils::FIndexedTable PartyDatabase;
    Core::Utils::FIndexedTable PartyMemberDatabase;
    Core::Id::TUniqueNetIdMap<TArray<IOnlinePartyRequestToJoinInfoConstRef>> RequestToJoins;
    static const FString DefaultPartyBucketId;

    FDelegateHandle OnRoomJoinedHandle;
    FDelegateHandle OnRoomUpdatedHandle;
    FDelegateHandle OnRoomLeftHandle;
    FDelegateHandle OnRoomInviteReceivedHandle;
    FDelegateHandle OnRoomInviteRemovedHandle;
    FDelegateHandle OnRoomJoinRequestedHandle;
    FDelegateHandle OnRoomLeaveRequestedHandle;

    TSharedRef<Features::FPartyConfigurationRoomFeatureRequest> ConvertPartyConfigurationToFeatureRequest(
        const FPartyConfiguration &PartyConfig) const;
    FRoomPtr GetUserCurrentRoom(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId) const;
    ECreatePartyCompletionResult ConvertErrorToCreatePartyCompletionResult(FError ErrorCode) const;
    EUpdateConfigCompletionResult ConvertErrorToUpdateConfigCompletionResult(FError ErrorCode) const;
    ELeavePartyCompletionResult ConvertErrorToLeavePartyCompletionResult(FError ErrorCode) const;
    ESendPartyInvitationCompletionResult ConvertErrorToSendPartyInvitationCompletionResult(FError ErrorCode) const;
    EKickMemberCompletionResult ConvertErrorToKickMemberCompletionResult(FError ErrorCode) const;
    EPromoteMemberCompletionResult ConvertErrorToPromoteMemberCompletionResult(FError ErrorCode) const;
    EJoinPartyCompletionResult ConvertErrorToJoinPartyCompletionResult(FError ErrorCode) const;

    /**
     * OnRoomJoined fires after the JoinRoom callback, and it's possible to get notified of room joins without going
     * through JoinRoom. Therefore we need to idempotently add the room to our PartyDatabase in both the JoinRoom
     * callback and OnRoomJoined handler so that functions such as GetJoinedParties report the correct information
     * before we return to game code.
     */
    void AddRoomToPartyDatabaseIfNotPresent(FRoomUserId LocalUserId, FRoomRef Room);

    /**
     * As with OnRoomJoined/JoinRoom, we need to idempotently remove rooms from the PartyDatabase so that the view of
     * parties is always correct for game code.
     */
    void RemoveRoomFromPartyDatabaseIfPresent(FRoomUserId LocalUserId, FRoomRef Room);

    /**
     * Join a party in response to overlay events.
     */
    void JoinPartyOnRequest(const FRoomEventContext &EventContext, const FRoomRequestedJoinRef &RoomRequestedJoin);

    /**
     * Leave a party in response to overlay events.
     */
    void LeavePartyOnRequest(const FRoomEventContext &EventContext, const FRoomRequestedLeaveRef &RoomRequestedLeave);

public:
    FOnlinePartySystemRedpointEOS(
        const FPlatformHandle &InPlatformHandle,
        const TSharedRef<IRoomSystem> &InRoomSystem,
        const TSharedRef<FEOSUserFactory> &InUserFactory,
        const TSharedRef<FEOSVoiceManager> &InVoiceManager,
        const TSharedRef<FOnlineSocialInterfaceEOS> &InSocialImpl);
    virtual void RegisterEvents() override;
    UE_NONCOPYABLE(FOnlinePartySystemRedpointEOS);
    virtual ~FOnlinePartySystemRedpointEOS() override;

    virtual void RestoreParties(const FUniqueNetId &LocalUserId, const FOnRestorePartiesComplete &CompletionDelegate)
        override;
    virtual void RestoreInvites(const FUniqueNetId &LocalUserId, const FOnRestoreInvitesComplete &CompletionDelegate)
        override;
    virtual void CleanupParties(const FUniqueNetId &LocalUserId, const FOnCleanupPartiesComplete &CompletionDelegate)
        override;
    virtual bool CreateParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyTypeId PartyTypeId,
        const FPartyConfiguration &PartyConfig,
        const FOnCreatePartyComplete &Delegate = FOnCreatePartyComplete()) override;
    virtual bool UpdateParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FPartyConfiguration &PartyConfig,
        bool bShouldRegenerateReservationKey = false,
        const FOnUpdatePartyComplete &Delegate = FOnUpdatePartyComplete()) override;
    virtual bool JoinParty(
        const FUniqueNetId &LocalUserId,
        const IOnlinePartyJoinInfo &OnlinePartyJoinInfo,
        const FOnJoinPartyComplete &Delegate = FOnJoinPartyComplete()) override;
    virtual void RequestToJoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyTypeId PartyTypeId,
        const FPartyInvitationRecipient &Recipient,
        const FOnRequestToJoinPartyComplete &Delegate = FOnRequestToJoinPartyComplete()) override;
    virtual void ClearRequestToJoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &Sender,
        EPartyRequestToJoinRemovedReason Reason) override;
    virtual void QueryPartyJoinability(
        const FUniqueNetId &LocalUserId,
        const IOnlinePartyJoinInfo &OnlinePartyJoinInfo,
        const FOnQueryPartyJoinabilityCompleteEx &Delegate) override;
    virtual bool RejoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FOnlinePartyTypeId &PartyTypeId,
        const TArray<FUniqueNetIdRef> &FormerMembers,
        const FOnJoinPartyComplete &Delegate = FOnJoinPartyComplete()) override;
    virtual bool LeaveParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FOnLeavePartyComplete &Delegate = FOnLeavePartyComplete()) override;
    virtual bool LeaveParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        bool bSynchronizeLeave,
        const FOnLeavePartyComplete &Delegate = FOnLeavePartyComplete()) override;
    virtual bool ApproveJoinRequest(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        bool bIsApproved,
        int32 DeniedResultCode = 0) override;
    virtual void RespondToQueryJoinability(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        bool bCanJoin,
        int32 DeniedResultCode,
        FOnlinePartyDataConstPtr PartyData) override;
    virtual bool SendInvitation(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FPartyInvitationRecipient &Recipient,
        const FOnSendPartyInvitationComplete &Delegate = FOnSendPartyInvitationComplete()) override;
    virtual void CancelInvitation(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        const FOnlinePartyId &PartyId,
        const FOnCancelPartyInvitationComplete &Delegate = FOnCancelPartyInvitationComplete()) override;
    virtual bool RejectInvitation(const FUniqueNetId &LocalUserId, const FUniqueNetId &SenderId) override;
    virtual void ClearInvitations(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &SenderId,
        const FOnlinePartyId *PartyId = nullptr) override;
    virtual bool KickMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &TargetMemberId,
        const FOnKickPartyMemberComplete &Delegate = FOnKickPartyMemberComplete()) override;
    virtual bool PromoteMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &TargetMemberId,
        const FOnPromotePartyMemberComplete &Delegate = FOnPromotePartyMemberComplete()) override;
    virtual bool UpdatePartyData(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyData) override;
    virtual bool UpdatePartyMemberData(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace,
        const FOnlinePartyData &PartyMemberData) override;
    virtual bool IsMemberLeader(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId) const override;
    virtual uint32 GetPartyMemberCount(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId) const override;
    virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
        const override;
    virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId &LocalUserId, const FOnlinePartyTypeId &PartyTypeId)
        const override;
    virtual FOnlinePartyMemberConstPtr GetPartyMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId) const override;
    virtual FOnlinePartyDataConstPtr GetPartyData(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FName &Namespace) const override;
    virtual FOnlinePartyDataConstPtr GetPartyMemberData(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId,
        const FName &Namespace) const override;
    virtual IOnlinePartyJoinInfoConstPtr GetAdvertisedParty(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &UserId,
        const FOnlinePartyTypeId PartyTypeId) const override;
    virtual bool GetJoinedParties(
        const FUniqueNetId &LocalUserId,
        TArray<TSharedRef<const FOnlinePartyId>> &OutPartyIdArray) const override;
    virtual bool GetPartyMembers(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<FOnlinePartyMemberConstRef> &OutPartyMembersArray) const override;
    virtual bool GetPendingInvites(
        const FUniqueNetId &LocalUserId,
        TArray<IOnlinePartyJoinInfoConstRef> &OutPendingInvitesArray) const override;
    virtual bool GetPendingJoinRequests(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<IOnlinePartyPendingJoinRequestInfoConstRef> &OutPendingJoinRequestArray) const override;
    virtual bool GetPendingInvitedUsers(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<FUniqueNetIdRef> &OutPendingInvitedUserArray) const override;
    virtual bool GetPendingRequestsToJoin(
        const FUniqueNetId &LocalUserId,
        TArray<IOnlinePartyRequestToJoinInfoConstRef> &OutRequestsToJoin) const override;
    virtual FString MakeJoinInfoJson(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId) override;
    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString &JoinInfoJson) override;
    virtual FString MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo &JoinInfo) const override;
    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfo(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
        override;
    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString &Token) const override;
    virtual IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite() override;
    virtual void DumpPartyState() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party
{
REDPOINT_EOS_FILE_NS_EXPORT(
    415509281,
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party,
    FOnlinePartySystemRedpointEOS)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()