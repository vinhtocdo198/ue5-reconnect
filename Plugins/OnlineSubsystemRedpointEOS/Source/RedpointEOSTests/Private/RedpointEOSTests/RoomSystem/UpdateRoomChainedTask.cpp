// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/UpdateRoomChainedTask.h"

#include "Containers/Ticker.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/RoomAttributeConstraint.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(3383242971, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Rooms::Requests;

FUpdateRoomChainedTask::FUpdateRoomChainedTask(
    int32 InOwnerInstanceId,
    TArray<int32> InJoinedInstanceIds,
    TSharedRef<FRoomIdPtr> InExpectedRoomId,
    TDelegate<void(FUpdateRoomRequest &Request)> InUpdateRoomRequestConfiguration,
    TDelegate<void(const FChainedTaskContextRef &Context, const FRoomRef &CurrentRoom)> InValidateRoom)
    : OwnerInstanceId(InOwnerInstanceId)
    , JoinedInstanceIds(InJoinedInstanceIds)
    , ExpectedRoomId(InExpectedRoomId)
    , UpdateRoomRequestConfiguration(InUpdateRoomRequestConfiguration)
    , ValidateRoom(InValidateRoom)
    , bGotUpdateCallback(false)
    , RoomUpdatedEventHandles()
    , bSawRoomUpdatedEvent()
{
}

void FUpdateRoomChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    for (auto InstanceId : this->JoinedInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        this->bSawRoomUpdatedEvent.Add(InstanceId, false);
        this->RoomUpdatedEventHandles.Add(
            InstanceId,
            RoomSystem->OnRoomUpdated(NAME_AutomationTests)
                .AddSP(this, &FUpdateRoomChainedTask::OnRoomUpdatedEvent, Context, InstanceId));
    }

    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->OwnerInstanceId)).GetRoomSystem();

        auto CurrentRoom =
            RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(this->OwnerInstanceId))))[0];

        auto UpdateRoomRequest = FUpdateRoomRequest();
        this->UpdateRoomRequestConfiguration.ExecuteIfBound(UpdateRoomRequest);

        RoomSystem->UpdateRoom(
            CurrentRoom,
            UpdateRoomRequest,
            IRoomSystem::FOnUpdateRoomComplete::CreateSP(
                this,
                &FUpdateRoomChainedTask::OnUpdateComplete,
                Context,
                FOnComplete::CreateSP(this, &FUpdateRoomChainedTask::OnUnwindingDone, Context, OnDone)));
    }
}

void FUpdateRoomChainedTask::OnUpdateComplete(
    const FError &ErrorCode,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestFalse(TEXT("Expected update callback to not have fired yet"), this->bGotUpdateCallback))
    {
        return;
    }
    this->bGotUpdateCallback = true;

    Context->Assert().TestTrue(
        FString::Printf(
            TEXT("Expected UpdateRoom for lobby provider to succeed, but got error '%s'."),
            *ErrorCode.ToLogString()),
        ErrorCode.WasSuccessful());

    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->OwnerInstanceId)).GetRoomSystem();
    auto CurrentRoom =
        RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(this->OwnerInstanceId))))[0];
    this->ValidateRoom.ExecuteIfBound(Context, CurrentRoom);

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FUpdateRoomChainedTask::OnUpdateCompleteWaitForEvents,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FUpdateRoomChainedTask::OnUpdateCompleteWaitForEvents(
    float DeltaSeconds,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = true;
    for (auto InstanceId : this->JoinedInstanceIds)
    {
        if (!this->bSawRoomUpdatedEvent[InstanceId])
        {
            bHasEvents = false;
        }
    }
    if (!bHasEvents)
    {
        if (FDateTime::UtcNow() > UtcExpiry)
        {
            for (auto InstanceId : this->JoinedInstanceIds)
            {
                Context->Assert().TestTrue(
                    FString::Printf(TEXT("Expected instance %d to receive OnRoomUpdated event."), InstanceId),
                    this->bSawRoomUpdatedEvent[InstanceId]);
            }
            OnDone.ExecuteIfBound(false);
            return false;
        }
        return true;
    }
    OnDone.ExecuteIfBound(true);
    return false;
}

void FUpdateRoomChainedTask::OnRoomUpdatedEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    const FRoomUpdateInfo &RoomUpdateInfo,
    FChainedTaskContextRef Context,
    int32 InstanceId)
{
    if (!Room->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get()))
    {
        return;
    }
    this->bSawRoomUpdatedEvent[InstanceId] = true;
}

void FUpdateRoomChainedTask::OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    for (auto InstanceId : this->JoinedInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        RoomSystem->OnRoomUpdated(NAME_AutomationTests).Remove(this->RoomUpdatedEventHandles[InstanceId]);
    }
    this->RoomUpdatedEventHandles.Empty();

    OnDone.ExecuteIfBound(bSuccessful);
}

}