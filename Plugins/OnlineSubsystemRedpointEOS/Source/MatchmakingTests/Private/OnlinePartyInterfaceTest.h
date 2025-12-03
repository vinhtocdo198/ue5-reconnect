// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlinePartyInterface.h"
#include "RedpointMatchmaking/Utility/UserIdContainers.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FOnlinePartyTest : public FOnlineParty
{
public:
    FOnlinePartyTest(
        const TSharedRef<const FOnlinePartyId> &InPartyId,
        const FOnlinePartyTypeId InPartyTypeId,
        const TSharedRef<const FUniqueNetId> &InLeaderId)
        : FOnlineParty(InPartyId, InPartyTypeId)
    {
        this->LeaderId = InLeaderId;
    }
    virtual bool CanLocalUserInvite(const FUniqueNetId &LocalUserId) const override
    {
        return false;
    }
    virtual bool IsJoinable() const override
    {
        return false;
    }
    virtual TSharedRef<const FPartyConfiguration> GetConfiguration() const override
    {
        return MakeShared<FPartyConfiguration>();
    }
};

class FOnlinePartyInterfaceTest : public IOnlinePartySystem,
                                  public TSharedFromThis<FOnlinePartyInterfaceTest, ESPMode::ThreadSafe>
{
private:
    TSharedPtr<class FTestBackend> Backend;
    // We can cheat because we know during tests a user is only ever in of one party.
    TUserIdMap<TSharedPtr<FOnlinePartyTest>> JoinedParty;

public:
    FOnlinePartyInterfaceTest(TSharedPtr<class FTestBackend> InBackend);
    virtual ~FOnlinePartyInterfaceTest() override = default;

    void JoinPartyAtStartup(const FUniqueNetId &UserId, const FOnlinePartyId &PartyId);
    TSharedPtr<const FOnlinePartyId> CreatePartyAtStartup(const FUniqueNetId &UserId);

    virtual void RestoreParties(const FUniqueNetId &LocalUserId, const FOnRestorePartiesComplete &CompletionDelegate)
        override PURE_VIRTUAL(FOnlinePartyInterfaceTest::RestoreParties, );

    virtual void RestoreInvites(const FUniqueNetId &LocalUserId, const FOnRestoreInvitesComplete &CompletionDelegate)
        override PURE_VIRTUAL(FOnlinePartyInterfaceTest::RestoreInvites, );

    virtual void CleanupParties(const FUniqueNetId &LocalUserId, const FOnCleanupPartiesComplete &CompletionDelegate)
        override PURE_VIRTUAL(FOnlinePartyInterfaceTest::CleanupParties, );

    virtual bool CreateParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyTypeId PartyTypeId,
        const FPartyConfiguration &PartyConfig,
        const FOnCreatePartyComplete &Delegate = FOnCreatePartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::CreateParty, return false;);

    virtual bool UpdateParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FPartyConfiguration &PartyConfig,
        bool bShouldRegenerateReservationKey = false,
        const FOnUpdatePartyComplete &Delegate = FOnUpdatePartyComplete()) override;

    virtual bool JoinParty(
        const FUniqueNetId &LocalUserId,
        const IOnlinePartyJoinInfo &OnlinePartyJoinInfo,
        const FOnJoinPartyComplete &Delegate = FOnJoinPartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::JoinParty, return false;);

    virtual void QueryPartyJoinability(
        const FUniqueNetId &,
        const IOnlinePartyJoinInfo &,
        const FOnQueryPartyJoinabilityCompleteEx &) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::QueryPartyJoinability, );

    virtual bool RejoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FOnlinePartyTypeId &PartyTypeId,
        const TArray<TSharedRef<const FUniqueNetId>> &FormerMembers,
        const FOnJoinPartyComplete &Delegate = FOnJoinPartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::RejoinParty, return false;);

    virtual bool LeaveParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FOnLeavePartyComplete &Delegate = FOnLeavePartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::LeaveParty, return false;);

    virtual bool LeaveParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        bool bSynchronizeLeave,
        const FOnLeavePartyComplete &Delegate = FOnLeavePartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::LeaveParty, return false;);

    virtual bool ApproveJoinRequest(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &RecipientId,
        bool bIsApproved,
        int32 DeniedResultCode = 0) override PURE_VIRTUAL(FOnlinePartyInterfaceTest::ApproveJoinRequest, return false;);

    virtual void RespondToQueryJoinability(
        const FUniqueNetId &,
        const FOnlinePartyId &,
        const FUniqueNetId &,
        bool,
        int32,
        FOnlinePartyDataConstPtr) override PURE_VIRTUAL(FOnlinePartyInterfaceTest::RespondToQueryJoinability, );

    virtual bool SendInvitation(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FPartyInvitationRecipient &Recipient,
        const FOnSendPartyInvitationComplete &Delegate = FOnSendPartyInvitationComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::SendInvitation, return false;);

    virtual void CancelInvitation(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &TargetUserId,
        const FOnlinePartyId &PartyId,
        const FOnCancelPartyInvitationComplete &Delegate = FOnCancelPartyInvitationComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::CancelInvitation, );

    virtual bool RejectInvitation(const FUniqueNetId &LocalUserId, const FUniqueNetId &SenderId) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::RejectInvitation, return false;);

    virtual void ClearInvitations(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &SenderId,
        const FOnlinePartyId *PartyId = nullptr) override PURE_VIRTUAL(FOnlinePartyInterfaceTest::ClearInvitations, );

    virtual bool KickMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &TargetMemberId,
        const FOnKickPartyMemberComplete &Delegate = FOnKickPartyMemberComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::KickMember, return false;);

    virtual bool PromoteMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &TargetMemberId,
        const FOnPromotePartyMemberComplete &Delegate = FOnPromotePartyMemberComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::PromoteMember, return false;);

    virtual bool UpdatePartyData(const FUniqueNetId &, const FOnlinePartyId &, const FName &, const FOnlinePartyData &)
        override;

    virtual bool UpdatePartyMemberData(
        const FUniqueNetId &,
        const FOnlinePartyId &,
        const FName &,
        const FOnlinePartyData &) override;

    virtual bool IsMemberLeader(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId) const override;

    virtual uint32 GetPartyMemberCount(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetPartyMemberCount, return 0;);

    virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId)
        const override;

    virtual FOnlinePartyConstPtr GetParty(const FUniqueNetId &LocalUserId, const FOnlinePartyTypeId &PartyTypeId)
        const override PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetParty, return nullptr;);

    virtual FOnlinePartyMemberConstPtr GetPartyMember(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &MemberId) const override;

    virtual FOnlinePartyDataConstPtr GetPartyData(const FUniqueNetId &, const FOnlinePartyId &, const FName &)
        const override;

    virtual FOnlinePartyDataConstPtr GetPartyMemberData(
        const FUniqueNetId &,
        const FOnlinePartyId &,
        const FUniqueNetId &,
        const FName &) const override;

    virtual IOnlinePartyJoinInfoConstPtr GetAdvertisedParty(
        const FUniqueNetId &LocalUserId,
        const FUniqueNetId &UserId,
        const FOnlinePartyTypeId PartyTypeId) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetAdvertisedParty, return nullptr;);

    virtual bool GetJoinedParties(
        const FUniqueNetId &LocalUserId,
        TArray<TSharedRef<const FOnlinePartyId>> &OutPartyIdArray) const override;

    virtual bool GetPartyMembers(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<FOnlinePartyMemberConstRef> &OutPartyMembersArray) const override;

    virtual bool GetPendingInvites(
        const FUniqueNetId &LocalUserId,
        TArray<IOnlinePartyJoinInfoConstRef> &OutPendingInvitesArray) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetPendingInvites, return false;);

    virtual bool GetPendingJoinRequests(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<IOnlinePartyPendingJoinRequestInfoConstRef> &OutPendingJoinRequestArray) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetPendingJoinRequests, return false;);

    virtual bool GetPendingInvitedUsers(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        TArray<TSharedRef<const FUniqueNetId>> &OutPendingInvitedUserArray) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetPendingInvitedUsers, return false;);

#if REDPOINT_EOS_UE_5_2_OR_LATER
    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfo(const FUniqueNetId &, const FOnlinePartyId &) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::MakeJoinInfo, return nullptr;);
#endif

    virtual FString MakeJoinInfoJson(const FUniqueNetId &LocalUserId, const FOnlinePartyId &PartyId) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::MakeJoinInfoJson, return FString(););

    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromJson(const FString &JoinInfoJson) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::MakeJoinInfoFromJson, return nullptr;);

    virtual FString MakeTokenFromJoinInfo(const IOnlinePartyJoinInfo &JoinInfo) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::MakeTokenFromJoinInfo, return FString(););

    virtual IOnlinePartyJoinInfoConstPtr MakeJoinInfoFromToken(const FString &Token) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::MakeJoinInfoFromToken, return nullptr;);

    virtual IOnlinePartyJoinInfoConstPtr ConsumePendingCommandLineInvite() override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::ConsumePendingCommandLineInvite, return nullptr;);

    virtual void DumpPartyState() override PURE_VIRTUAL(FOnlinePartyInterfaceTest::DumpPartyState, );

    virtual void RequestToJoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyTypeId PartyTypeId,
        const FPartyInvitationRecipient &Recipient,
        const FOnRequestToJoinPartyComplete &Delegate = FOnRequestToJoinPartyComplete()) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::RequestToJoinParty, );

    virtual void ClearRequestToJoinParty(
        const FUniqueNetId &LocalUserId,
        const FOnlinePartyId &PartyId,
        const FUniqueNetId &Sender,
        EPartyRequestToJoinRemovedReason Reason) override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::ClearRequestToJoinParty, );

    virtual bool GetPendingRequestsToJoin(
        const FUniqueNetId &LocalUserId,
        TArray<IOnlinePartyRequestToJoinInfoConstRef> &OutRequestsToJoin) const override
        PURE_VIRTUAL(FOnlinePartyInterfaceTest::GetPendingRequestsToJoin, return false;);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()