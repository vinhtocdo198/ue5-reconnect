// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/KickRoomMemberChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(2325888234, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Rooms::Requests;

FKickRoomMemberChainedTask::FKickRoomMemberChainedTask(
    int32 InActorInstanceId,
    int32 InTargetInstanceId,
    TSharedRef<FRoomIdPtr> InExpectedRoomId,
    bool bInDoubleCall)
    : ActorInstanceId(InActorInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , ExpectedRoomId(InExpectedRoomId)
    , bGotLeaveCallback(false)
    , RoomLeftEventHandle()
    , RoomMemberLeftEventHandle()
    , bSawRoomLeftEvent(false)
    , bSawRoomMemberLeftEvent(false)
    , bDoubleCall(bInDoubleCall)
{
}

void FKickRoomMemberChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto TargetRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->TargetInstanceId)).GetRoomSystem();
    this->RoomLeftEventHandle = TargetRoomSystem->OnRoomLeft(NAME_AutomationTests)
                                    .AddSP(this, &FKickRoomMemberChainedTask::OnRoomLeftEvent, Context);

    auto ActorRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
    auto CurrentRoom =
        ActorRoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))))[0];
    this->RoomMemberLeftEventHandle =
        CurrentRoom->OnRoomMemberLeft().AddSP(this, &FKickRoomMemberChainedTask::OnRoomMemberLeftEvent, Context);

    auto KickRoomMemberRequest = FKickRoomMemberRequest();
    KickRoomMemberRequest.TargetUser = GetAccountId(Context->User(this->TargetInstanceId));
    ActorRoomSystem->KickRoomMember(
        CurrentRoom,
        KickRoomMemberRequest,
        IRoomSystem::FOnKickRoomMemberComplete::CreateSP(
            this,
            &FKickRoomMemberChainedTask::OnKickMemberComplete,
            Context,
            FOnComplete::CreateSP(this, &FKickRoomMemberChainedTask::OnUnwindingDone, Context, OnDone)));

    if (bDoubleCall)
    {
        // Run a second call to detect that we don't crash on anticipated kicks.
        ActorRoomSystem->KickRoomMember(
            CurrentRoom,
            KickRoomMemberRequest,
            IRoomSystem::FOnKickRoomMemberComplete());
    }
}

void FKickRoomMemberChainedTask::OnKickMemberComplete(
    const FError &ErrorCode,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Expected KickRoomMember for lobby provider to succeed, but got error '%s'."),
                *ErrorCode.ToLogString()),
            ErrorCode.WasSuccessful()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FKickRoomMemberChainedTask::OnKickMemberCompleteWaitForEvents,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FKickRoomMemberChainedTask::OnKickMemberCompleteWaitForEvents(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = this->bSawRoomLeftEvent && this->bSawRoomMemberLeftEvent;
    if (!bHasEvents)
    {
        if (FDateTime::UtcNow() > UtcExpiry)
        {
            Context->Assert().TestTrue(
                FString::Printf(TEXT("Expected OnRoomLeft event to fire on instance %d."), this->TargetInstanceId),
                this->bSawRoomLeftEvent);
            Context->Assert().TestTrue(
                FString::Printf(TEXT("Expected OnRoomMemberLeft event to fire on instance %d."), this->ActorInstanceId),
                this->bSawRoomMemberLeftEvent);
            OnDone.ExecuteIfBound(false);
            return false;
        }
        return true;
    }
    OnDone.ExecuteIfBound(true);
    return false;
}

void FKickRoomMemberChainedTask::OnRoomLeftEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason,
    FChainedTaskContextRef Context)
{
    if (!Room->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()) ||
        EventContext.LocalUserId != GetAccountId(Context->User(this->TargetInstanceId)))
    {
        return;
    }
    Context->Assert().TestEqual(
        FString::Printf(
            TEXT("Leave reason (%d) does not match expected leave reason (%d)."),
            (int32)Reason,
            (int32)ERoomLeaveReason::KickedFromRoom),
        Reason,
        ERoomLeaveReason::KickedFromRoom);
    this->bSawRoomLeftEvent = true;
}

void FKickRoomMemberChainedTask::OnRoomMemberLeftEvent(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser,
    ERoomLeaveReason LeaveReason,
    FChainedTaskContextRef Context)
{
    if (!Room->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()))
    {
        return;
    }
    Context->Assert().TestEqual("Expected event context to be local user.", EventContext.RoomScope, ERoomScope::User);
    Context->Assert().TestEqual(
        "Expected event context local user ID to match expected.",
        EventContext.LocalUserId,
        GetAccountId(Context->User(this->ActorInstanceId)));
    Context->Assert().TestEqual(
        "Expected room member target ID to match expected.",
        TargetUser,
        GetAccountId(Context->User(this->TargetInstanceId)));
    this->bSawRoomMemberLeftEvent = true;
}

void FKickRoomMemberChainedTask::OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    {
        auto TargetRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->TargetInstanceId)).GetRoomSystem();
        TargetRoomSystem->OnRoomLeft(NAME_AutomationTests).Remove(this->RoomLeftEventHandle);

        auto ActorRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
        auto CurrentRoom = ActorRoomSystem->GetCurrentRooms(
            GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))))[0];
        CurrentRoom->OnRoomMemberLeft().Remove(this->RoomMemberLeftEventHandle);
    }

    OnDone.ExecuteIfBound(bSuccessful);
}

}