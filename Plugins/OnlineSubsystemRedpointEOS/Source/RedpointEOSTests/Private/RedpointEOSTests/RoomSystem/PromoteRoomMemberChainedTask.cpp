// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/PromoteRoomMemberChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"

namespace REDPOINT_EOS_FILE_NS_ID(1399126097, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Tests::RoomSystem;

FPromoteRoomMemberChainedTask::FPromoteRoomMemberChainedTask(
    int32 InActorInstanceId,
    int32 InTargetInstanceId,
    TSet<int32> InObservingInstanceIds,
    TSharedRef<FRoomIdPtr> InExpectedRoomId)
    : ActorInstanceId(InActorInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , ObservingInstanceIds(InObservingInstanceIds)
    , ExpectedRoomId(InExpectedRoomId)
    , RoomMemberPromotedEventHandles()
    , bSawRoomMemberPromotedEvent()
{
    this->ObservingInstanceIds.Add(InActorInstanceId);
    this->ObservingInstanceIds.Add(InTargetInstanceId);
}

void FPromoteRoomMemberChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    for (auto InstanceId : this->ObservingInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        this->bSawRoomMemberPromotedEvent.Add(InstanceId, false);
        this->RoomMemberPromotedEventHandles.Add(
            InstanceId,
            RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
                ->OnRoomMemberPromoted()
                .AddSP(this, &FPromoteRoomMemberChainedTask::OnRoomMemberPromotedEvent, Context, InstanceId));
    }

    {
        auto ActorRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
        auto CurrentRoom = ActorRoomSystem->GetCurrentRooms(
            GetCurrentRoomsRequest(GetAccountId(Context->User(this->ActorInstanceId))))[0];

        auto PromoteRoomMemberRequest = FPromoteRoomMemberRequest();
        PromoteRoomMemberRequest.TargetUser = GetAccountId(Context->User(this->TargetInstanceId));
        ActorRoomSystem->PromoteRoomMember(
            CurrentRoom,
            PromoteRoomMemberRequest,
            IRoomSystem::FOnPromoteRoomMemberComplete::CreateSP(
                this,
                &FPromoteRoomMemberChainedTask::OnPromoteComplete,
                Context,
                FOnComplete::CreateSP(this, &FPromoteRoomMemberChainedTask::OnUnwindingDone, Context, OnDone)));
    }
}

void FPromoteRoomMemberChainedTask::OnPromoteComplete(
    const FError &ErrorCode,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Expected PromoteRoomMember for lobby provider to succeed, but got error '%s'."),
                *ErrorCode.ToLogString()),
            ErrorCode.WasSuccessful()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FPromoteRoomMemberChainedTask::OnPromoteCompleteWaitForEvents,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FPromoteRoomMemberChainedTask::OnPromoteCompleteWaitForEvents(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = true;
    for (auto InstanceId : this->ObservingInstanceIds)
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
            for (const auto &InstanceId : this->ObservingInstanceIds)
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

void FPromoteRoomMemberChainedTask::OnRoomMemberPromotedEvent(
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
        GetAccountId(Context->User(this->TargetInstanceId)));
    this->bSawRoomMemberPromotedEvent[InstanceId] = true;
}

void FPromoteRoomMemberChainedTask::OnUnwindingDone(
    bool bSuccessful,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    for (auto InstanceId : this->ObservingInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
            ->OnRoomMemberPromoted()
            .Remove(this->RoomMemberPromotedEventHandles[InstanceId]);
    }
    this->RoomMemberPromotedEventHandles.Empty();

    OnDone.ExecuteIfBound(bSuccessful);
}

}