// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Party/JoinPartyViaOverlayChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/RoomInvite.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(621343221, Redpoint::EOS::Tests::Party)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

FJoinPartyViaOverlayChainedTask::FJoinPartyViaOverlayChainedTask(
    int32 InInstanceId,
    TSharedRef<TSharedPtr<const IOnlinePartyJoinInfo>> InStoredPartyJoinInfo,
    bool bInAutoApprove,
    bool bInShouldApprove)
    : InstanceId(InInstanceId)
    , StoredPartyJoinInfo(InStoredPartyJoinInfo)
    , bAutoApprove(bInAutoApprove)
    , bShouldApprove(bInShouldApprove)
    , OnPartyJoinedDelegateHandle()
    , OnPartyRequestToJoinReceivedDelegateHandle()
{
}

void FJoinPartyViaOverlayChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    if (!Context->Assert().TestTrue(TEXT("Expected to have party info to join!"), this->StoredPartyJoinInfo->IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();

    auto Invites = RoomSystem->GetCurrentRoomInvites(GetAccountId(Context->User(this->InstanceId)), NAME_Game);
    if (!Context->Assert().TestTrue(TEXT("Expected to have at least 1 room invite."), Invites.Num() > 0))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }
    FRoomInvitePtr RoomInvite = nullptr;
    for (const auto &Invite : Invites)
    {
        if (Invite->GetRoomInviteSearchResult()->GetRoomId()->GetRoomId() ==
            this->StoredPartyJoinInfo->Get()->GetPartyId()->ToString())
        {
            RoomInvite = Invite;
            break;
        }
    }
    if (!Context->Assert().TestTrue(TEXT("Found room invite that matches party invite."), RoomInvite.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    if (this->bAutoApprove)
    {
        this->OnPartyJoinedDelegateHandle = Context->Instance(this->InstanceId)
                                                .GetPartyInterface()
                                                ->AddOnPartyJoinedDelegate_Handle(FOnPartyJoinedDelegate::CreateSP(
                                                    this,
                                                    &FJoinPartyViaOverlayChainedTask::OnPartyJoined,
                                                    Context,
                                                    OnDone));
    }
    else
    {
        this->OnPartyRequestToJoinReceivedDelegateHandle =
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->AddOnPartyRequestToJoinReceivedDelegate_Handle(FOnPartyRequestToJoinReceivedDelegate::CreateSP(
                    this,
                    &FJoinPartyViaOverlayChainedTask::OnPartyRequestToJoinReceived,
                    Context,
                    OnDone));
    }

    this->TimerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateSP(this, &FJoinPartyViaOverlayChainedTask::OnTimeout, Context, OnDone),
        30.0f);

    // Simulate an overlay "accept" event using internal API access on the room system.
    RoomSystem->OnRoomInviteLocallyAcceptedJoinRequiredFromSDKEvent(
        RoomInvite->GetRoomInviteId(),
        RoomInvite->GetRoomInviteRecipientUserId());
}

bool FJoinPartyViaOverlayChainedTask::OnTimeout(float, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    Context->Assert().TestTrue(TEXT("Expected operation to complete within 30 seconds."), false);

    if (this->OnPartyJoinedDelegateHandle.IsValid())
    {
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->ClearOnPartyJoinedDelegate_Handle(this->OnPartyJoinedDelegateHandle);
    }

    if (this->OnPartyRequestToJoinReceivedDelegateHandle.IsValid())
    {
        Context->Instance(this->InstanceId)
            .GetPartyInterface()
            ->ClearOnPartyRequestToJoinReceivedDelegate_Handle(this->OnPartyRequestToJoinReceivedDelegateHandle);
    }

    OnDone.ExecuteIfBound(false);

    return false;
}

void FJoinPartyViaOverlayChainedTask::OnPartyJoined(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().AddInfo(FString::Printf(TEXT("Got OnPartyJoined event for party ID '%s'."), *PartyId.ToString()));

    if (PartyId != *this->StoredPartyJoinInfo->Get()->GetPartyId())
    {
        return;
    }

    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->ClearOnPartyJoinedDelegate_Handle(this->OnPartyJoinedDelegateHandle);
    this->OnPartyJoinedDelegateHandle.Reset();

    FTSTicker::GetCoreTicker().RemoveTicker(this->TimerHandle);
    this->TimerHandle.Reset();

    OnDone.ExecuteIfBound(true);
}

void FJoinPartyViaOverlayChainedTask::OnPartyRequestToJoinReceived(
    const FUniqueNetId &LocalUserId,
    const FOnlinePartyId &PartyId,
    const FUniqueNetId &RequesterId,
    const IOnlinePartyRequestToJoinInfo &Request,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    Context->Assert().AddInfo(
        FString::Printf(TEXT("Got OnPartyRequestToJoinReceived event for party ID '%s'."), *PartyId.ToString()));

    if (PartyId != *this->StoredPartyJoinInfo->Get()->GetPartyId())
    {
        return;
    }

    Context->Instance(this->InstanceId)
        .GetPartyInterface()
        ->ClearOnPartyRequestToJoinReceivedDelegate_Handle(this->OnPartyRequestToJoinReceivedDelegateHandle);
    this->OnPartyRequestToJoinReceivedDelegateHandle.Reset();

    if (this->bShouldApprove)
    {
        this->OnPartyJoinedDelegateHandle = Context->Instance(this->InstanceId)
                                                .GetPartyInterface()
                                                ->AddOnPartyJoinedDelegate_Handle(FOnPartyJoinedDelegate::CreateSP(
                                                    this,
                                                    &FJoinPartyViaOverlayChainedTask::OnPartyJoined,
                                                    Context,
                                                    OnDone));
    }

    // @todo: We should test the "request to join removed" event.

    if (!Context->Assert().TestTrue(
            TEXT("Expected ApproveJoinRequest call to start."),
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->ApproveJoinRequest(LocalUserId, PartyId, LocalUserId, this->bShouldApprove)))
    {
        if (this->bShouldApprove)
        {
            Context->Instance(this->InstanceId)
                .GetPartyInterface()
                ->ClearOnPartyJoinedDelegate_Handle(this->OnPartyJoinedDelegateHandle);
            this->OnPartyJoinedDelegateHandle.Reset();
        }

        FTSTicker::GetCoreTicker().RemoveTicker(this->TimerHandle);
        this->TimerHandle.Reset();

        OnDone.ExecuteIfBound(true);
        return;
    }

    // If not approving, there will be no more events.
    if (!this->bShouldApprove)
    {
        FTSTicker::GetCoreTicker().RemoveTicker(this->TimerHandle);
        this->TimerHandle.Reset();

        OnDone.ExecuteIfBound(true);
        return;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()