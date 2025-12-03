// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/LeaveRoomChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(113233376, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Rooms::Requests;

FLeaveRoomChainedTask::FLeaveRoomChainedTask(
    int32 InActorInstanceId,
    TMap<int32, ERoomLeaveReason> InLeavingInstanceIdsAndReasons,
    TSet<int32> InPromotedInstanceIds,
    TSharedRef<FRoomIdPtr> InExpectedRoomId)
    : ActorInstanceId(InActorInstanceId)
    , LeavingInstanceIdsAndReasons(InLeavingInstanceIdsAndReasons)
    , PromotedInstanceIds(InPromotedInstanceIds)
    , ExpectedRoomId(InExpectedRoomId)
    , bGotLeaveCallback(false)
    , RoomLeftEventHandles()
    , RoomMemberPromotedEventHandles()
    , bSawRoomLeftEvent()
    , bSawRoomMemberPromotedEvent()
{
}

void FLeaveRoomChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    for (const auto &KV : this->LeavingInstanceIdsAndReasons)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(KV.Key)).GetRoomSystem();
        this->bSawRoomLeftEvent.Add(KV.Key, false);
        this->RoomLeftEventHandles.Add(
            KV.Key,
            RoomSystem->OnRoomLeft(NAME_AutomationTests)
                .AddSP(this, &FLeaveRoomChainedTask::OnRoomLeftEvent, Context, KV.Key, KV.Value));
    }
    for (auto InstanceId : this->PromotedInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        this->bSawRoomMemberPromotedEvent.Add(InstanceId, false);
        this->RoomMemberPromotedEventHandles.Add(
            InstanceId,
            RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
                ->OnRoomMemberPromoted()
                .AddSP(this, &FLeaveRoomChainedTask::OnRoomMemberPromotedEvent, Context, InstanceId));
    }

    {
        auto ActorRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
        auto CurrentRoom = ActorRoomSystem->GetCurrentRooms(
            GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))))[0];

        auto LeaveRoomRequest = FLeaveRoomRequest();
        ActorRoomSystem->LeaveRoom(
            CurrentRoom,
            LeaveRoomRequest,
            IRoomSystem::FOnLeaveRoomComplete::CreateSP(
                this,
                &FLeaveRoomChainedTask::OnLeaveComplete,
                Context,
                FOnComplete::CreateSP(this, &FLeaveRoomChainedTask::OnUnwindingDone, Context, OnDone)));
    }
}

void FLeaveRoomChainedTask::OnLeaveComplete(const FError &ErrorCode, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    if (!Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Expected LeaveRoom for lobby provider to succeed, but got error '%s'."),
                *ErrorCode.ToLogString()),
            ErrorCode.WasSuccessful()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Check that we no longer see the room in the rooms list.
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
    auto CurrentRooms =
        RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))));
    Context->Assert().TestEqual(TEXT("Expected user to no longer be in any rooms."), CurrentRooms.Num(), 0);

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FLeaveRoomChainedTask::OnLeaveCompleteWaitForEvents,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FLeaveRoomChainedTask::OnLeaveCompleteWaitForEvents(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = true;
    for (const auto &KV : this->LeavingInstanceIdsAndReasons)
    {
        if (!this->bSawRoomLeftEvent[KV.Key])
        {
            bHasEvents = false;
            break;
        }
    }
    for (auto InstanceId : this->PromotedInstanceIds)
    {
        if (!this->bSawRoomMemberPromotedEvent[InstanceId])
        {
            bHasEvents = false;
            break;
        }
    }
    if (!bHasEvents)
    {
        if (FDateTime::UtcNow() > UtcExpiry)
        {
            for (const auto &KV : this->LeavingInstanceIdsAndReasons)
            {
                Context->Assert().TestTrue(
                    FString::Printf(TEXT("Expected OnRoomLeft event to fire on instance %d."), KV.Key),
                    this->bSawRoomLeftEvent[KV.Key]);
            }
            for (const auto &InstanceId : this->PromotedInstanceIds)
            {
                Context->Assert().TestTrue(
                    FString::Printf(TEXT("Expected OnRoomMemberPromoted event to fire on instance %d."), InstanceId),
                    this->bSawRoomMemberPromotedEvent[InstanceId]);
            }
            OnDone.ExecuteIfBound(false);
            return false;
        }
        return true;
    }
    OnDone.ExecuteIfBound(true);
    return false;
}

void FLeaveRoomChainedTask::OnRoomLeftEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason,
    FChainedTaskContextRef Context,
    int32 InstanceId,
    ERoomLeaveReason ExpectedLeaveReason)
{
    if (!Room->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()) ||
        EventContext.LocalUserId != GetAccountId(Context->User(InstanceId)))
    {
        return;
    }
    Context->Assert().TestEqual(
        FString::Printf(
            TEXT("Leave reason (%d) does not match expected leave reason (%d)."),
            (int32)Reason,
            (int32)ExpectedLeaveReason),
        Reason,
        ExpectedLeaveReason);
    this->bSawRoomLeftEvent[InstanceId] = true;
}

void FLeaveRoomChainedTask::OnRoomMemberPromotedEvent(
    const FRoomEventContext &EventContext,
    const TSharedRef<IRoom> &Room,
    FRoomUserId TargetUser,
    FChainedTaskContextRef Context,
    int32 InstanceId)
{
    if (!Room->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()))
    {
        return;
    }
    Context->Assert().TestEqual("Expected event context to be local user.", EventContext.RoomScope, ERoomScope::User);
    Context->Assert().TestEqual(
        "Expected event context local user ID to match expected.",
        EventContext.LocalUserId,
        GetAccountId(Context->User(InstanceId)));
    Context->Assert().TestEqual(
        "Expected room member target ID to match expected.",
        TargetUser,
        GetAccountId(Context->User(InstanceId)));
    this->bSawRoomMemberPromotedEvent[InstanceId] = true;
}

void FLeaveRoomChainedTask::OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    for (const auto &KV : this->LeavingInstanceIdsAndReasons)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(KV.Key)).GetRoomSystem();
        RoomSystem->OnRoomLeft(NAME_AutomationTests).Remove(this->RoomLeftEventHandles[KV.Key]);
    }
    for (auto InstanceId : this->PromotedInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
            ->OnRoomMemberPromoted()
            .Remove(this->RoomMemberPromotedEventHandles[InstanceId]);
    }
    this->RoomLeftEventHandles.Empty();
    this->RoomMemberPromotedEventHandles.Empty();

    OnDone.ExecuteIfBound(bSuccessful);
}

}