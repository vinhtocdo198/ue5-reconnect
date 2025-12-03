// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/CreateRoomChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"

namespace REDPOINT_EOS_FILE_NS_ID(2771099875, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::RoomSystem;

FCreateRoomChainedTask::FCreateRoomChainedTask(
    int32 InOwnerInstanceId,
    FGuid InSearchGuid,
    TSharedRef<FRoomIdPtr> InExpectedRoomId,
    TDelegate<void(FCreateRoomRequest &Request)> InCreateRoomRequestConfiguration)
    : OwnerInstanceId(InOwnerInstanceId)
    , SearchGuid(InSearchGuid)
    , ExpectedRoomId(InExpectedRoomId)
    , CreateRoomRequestConfiguration(InCreateRoomRequestConfiguration)
    , bGotCreateCallback(false)
    , RoomJoinedEventHandle()
    , bSawRoomJoinedEvent(false)
{
}

void FCreateRoomChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->OwnerInstanceId)).GetRoomSystem();
    this->RoomJoinedEventHandle =
        RoomSystem->OnRoomJoined(NAME_AutomationTests).AddSP(this, &FCreateRoomChainedTask::OnRoomJoinedEvent, Context);

    auto CreateRoomRequest = FCreateRoomRequest(
        ERoomScope::User,
        GetAccountId(Context->User(this->OwnerInstanceId)),
        NAME_Lobby,
        NAME_AutomationTests);
    CreateRoomRequest.Attributes.Add(
        TEXT("TestSearchGuid"),
        FRoomAttribute(this->SearchGuid.ToString(), FRoomAttribute::EVisibility::Public));

    this->CreateRoomRequestConfiguration.ExecuteIfBound(CreateRoomRequest);

    RoomSystem->CreateRoom(
        CreateRoomRequest,
        IRoomSystem::FOnCreateRoomComplete::CreateSP(
            this,
            &FCreateRoomChainedTask::OnCreateComplete,
            Context,
            FOnComplete::CreateSP(this, &FCreateRoomChainedTask::OnUnwindingDone, Context, OnDone)));
}

void FCreateRoomChainedTask::OnCreateComplete(
    const FError &ErrorCode,
    const FRoomPtr &Room,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestFalse(TEXT("Expected create callback to not have fired yet"), this->bGotCreateCallback))
    {
        return;
    }
    this->bGotCreateCallback = true;

    Context->Assert().TestTrue(
        FString::Printf(
            TEXT("Expected CreateRoom for lobby provider to succeed, but got error '%s'."),
            *ErrorCode.ToLogString()),
        ErrorCode.WasSuccessful());
    if (!Context->Assert().TestTrue(
            FString::Printf(TEXT("Expected CreateRoom to return a valid room."), *ErrorCode.ToLogString()),
            Room.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    Context->Assert().TestTrue(TEXT("Expected OnRoomJoined event to fire."), this->bSawRoomJoinedEvent);

    *this->ExpectedRoomId = Room->GetRoomId();
    OnDone.ExecuteIfBound(true);
}

void FCreateRoomChainedTask::OnRoomJoinedEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomJoinReason Reason,
    FChainedTaskContextRef Context)
{
    Context->Assert().TestEqual("Expected event context to be local user.", EventContext.RoomScope, ERoomScope::User);
    Context->Assert().TestEqual(
        "Expected event context local user ID to match expected.",
        EventContext.LocalUserId,
        GetAccountId(Context->User(this->OwnerInstanceId)));
    Context->Assert().TestEqual(
        "Expected join reason to be RequestedByCreateRoom.",
        Reason,
        ERoomJoinReason::RequestedByCreateRoom);
    this->bSawRoomJoinedEvent = true;
}

void FCreateRoomChainedTask::OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone)
{
    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->OwnerInstanceId)).GetRoomSystem();
    RoomSystem->OnRoomJoined(NAME_AutomationTests).Remove(this->RoomJoinedEventHandle);
    this->RoomJoinedEventHandle.Reset();

    OnDone.ExecuteIfBound(bSuccessful);
}

}