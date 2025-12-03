// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/SendRoomInviteChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(1652404932, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Tests::RoomSystem;

FSendRoomInviteChainedTask::FSendRoomInviteChainedTask(
    int32 InActorInstanceId,
    int32 InTargetInstanceId,
    TSharedRef<FRoomIdPtr> InExpectedRoomId,
    TSharedRef<FRoomInvitePtr> OutReceivedRoomInvite)
    : ActorInstanceId(InActorInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , ExpectedRoomId(InExpectedRoomId)
    , ReceivedRoomInvite(OutReceivedRoomInvite)
    , RoomInviteReceivedHandle()
    , bSawRoomInviteReceivedEvent(false)
{
}

void FSendRoomInviteChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto TargetRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->TargetInstanceId)).GetRoomSystem();
    this->RoomInviteReceivedHandle = TargetRoomSystem->OnRoomInviteReceived(NAME_AutomationTests)
                                         .AddSP(this, &FSendRoomInviteChainedTask::OnRoomInviteReceived, Context);

    auto ActorRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
    auto CurrentRoom =
        ActorRoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))))[0];

    ActorRoomSystem->SendRoomInvite(
        CurrentRoom,
        FSendRoomInviteRequest{GetAccountId(Context->User(this->TargetInstanceId))},
        IRoomSystem::FOnSendRoomInviteComplete::CreateSP(
            this,
            &FSendRoomInviteChainedTask::OnSendInviteComplete,
            Context,
            FOnComplete::CreateSP(this, &FSendRoomInviteChainedTask::OnUnwindingDone, Context, OnDone)));
}

void FSendRoomInviteChainedTask::OnSendInviteComplete(
    const FError &ErrorCode,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Expected SendRoomInvite for lobby provider to succeed, but got error '%s'."),
                *ErrorCode.ToLogString()),
            ErrorCode.WasSuccessful()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FSendRoomInviteChainedTask::OnSendInviteCompleteWaitForEvents,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FSendRoomInviteChainedTask::OnSendInviteCompleteWaitForEvents(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = this->bSawRoomInviteReceivedEvent;
    if (!bHasEvents)
    {
        if (FDateTime::UtcNow() > UtcExpiry)
        {
            Context->Assert().TestTrue(
                FString::Printf(
                    TEXT("Expected OnRoomInviteReceived event to fire on instance %d."),
                    this->TargetInstanceId),
                this->bSawRoomInviteReceivedEvent);
            OnDone.ExecuteIfBound(false);
            return false;
        }
        return true;
    }
    OnDone.ExecuteIfBound(true);
    return false;
}

void FSendRoomInviteChainedTask::OnRoomInviteReceived(
    const FRoomEventContext &EventContext,
    const FRoomInviteRef &IncomingRoomInvite,
    FChainedTaskContextRef Context)
{
    if (!IncomingRoomInvite->GetRoomInviteSearchResult()->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()))
    {
        return;
    }
    Context->Assert().TestEqual("Expected event context to be local user.", EventContext.RoomScope, ERoomScope::User);
    Context->Assert().TestEqual(
        "Expected event context local user ID to match expected.",
        EventContext.LocalUserId,
        GetAccountId(Context->User(this->TargetInstanceId)));
    *this->ReceivedRoomInvite = IncomingRoomInvite;
    this->bSawRoomInviteReceivedEvent = true;
}

void FSendRoomInviteChainedTask::OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->TargetInstanceId)).GetRoomSystem();
    RoomSystem->OnRoomInviteReceived(NAME_AutomationTests).Remove(this->RoomInviteReceivedHandle);

    OnDone.ExecuteIfBound(bSuccessful);
}

}