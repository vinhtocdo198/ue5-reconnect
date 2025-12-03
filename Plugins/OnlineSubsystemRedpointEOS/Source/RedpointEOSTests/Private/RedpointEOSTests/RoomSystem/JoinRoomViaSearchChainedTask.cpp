// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/JoinRoomViaSearchChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeatureRequest.h"
#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"
#include "RedpointEOSRooms/RoomAttributeConstraint.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"
#include "RedpointEOSTests/RoomSystem/RoomSystemConstants.h"
#include "RedpointEOSTests/TestUtilities/Logging.h"

namespace REDPOINT_EOS_FILE_NS_ID(1028870339, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FJoinRoomViaSearchChainedTask::FJoinRoomViaSearchChainedTask(
    int32 InActorInstanceId,
    TSet<int32> InObservingInstanceIds,
    FGuid InSearchGuid,
    TSharedRef<FRoomIdPtr> InExpectedRoomId,
    bool bInHasPresence,
    TDelegate<void(FJoinRoomRequest &Request)> InJoinRoomRequestConfiguration,
    bool bInExpectKick)
    : ActorInstanceId(InActorInstanceId)
    , ObservingInstanceIds(InObservingInstanceIds)
    , SearchGuid(InSearchGuid)
    , ExpectedRoomId(InExpectedRoomId)
    , bHasPresence(bInHasPresence)
    , JoinRoomRequestConfiguration(InJoinRoomRequestConfiguration)
    , bExpectKick(bInExpectKick)
    , bGotSearchCallback(false)
    , bGotJoinCallback(false)
    , RoomMemberJoinedEventHandles()
    , RoomJoinedEventHandle()
    , RoomLeftEventHandle()
    , bSawRoomMemberJoinedEvents()
    , bSawRoomJoinedEvent(false)
    , bSawRoomLeftEvent(false)
{
}

void FJoinRoomViaSearchChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Binding join events for %d, for room ID '%s' and GUID '%s'"),
        this->ActorInstanceId,
        *this->ExpectedRoomId->Get()->GetRoomId(),
        *this->SearchGuid.ToString());

    for (const auto &InstanceId : this->ObservingInstanceIds)
    {
        auto OwnerRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        this->bSawRoomMemberJoinedEvents.Add(InstanceId, false);
        this->RoomMemberJoinedEventHandles.Add(
            InstanceId,
            OwnerRoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
                ->OnRoomMemberJoined()
                .AddSP(this, &FJoinRoomViaSearchChainedTask::OnRoomMemberJoinedEvent, Context, InstanceId));
    }

    {
        auto MemberRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
        this->RoomJoinedEventHandle = MemberRoomSystem->OnRoomJoined(NAME_AutomationTests)
                                          .AddSP(this, &FJoinRoomViaSearchChainedTask::OnRoomJoinedEvent, Context);

        if (this->bExpectKick)
        {
            this->RoomLeftEventHandle = MemberRoomSystem->OnRoomLeft(NAME_AutomationTests)
                                            .AddSP(this, &FJoinRoomViaSearchChainedTask::OnRoomLeftEvent, Context);
        }

        auto SearchRoomRequest = FSearchRoomsRequest(
            GetAccountId(Context->User(this->ActorInstanceId)),
            NAME_Lobby,
            NAME_AutomationTests,
            TMap<FString, FRoomAttributeConstraint>{
                {TEXT("TestSearchGuid"),
                 FRoomAttributeConstraint(
                     FRoomAttribute(SearchGuid.ToString()),
                     FRoomAttributeConstraint::EConstraintType::Equal)}});

        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("Searching for rooms from user %d, for room ID '%s' and GUID '%s'"),
            this->ActorInstanceId,
            *this->ExpectedRoomId->Get()->GetRoomId(),
            *this->SearchGuid.ToString());
        MemberRoomSystem->SearchRooms(
            SearchRoomRequest,
            IRoomSystem::FOnSearchRoomsComplete::CreateSP(
                this,
                &FJoinRoomViaSearchChainedTask::OnSearchComplete,
                Context,
                FOnComplete::CreateSP(this, &FJoinRoomViaSearchChainedTask::OnUnwindingDone, Context, OnDone)));
    }
}

void FJoinRoomViaSearchChainedTask::OnSearchComplete(
    const FError &ErrorCode,
    const TArray<FRoomSearchResultRef> &RoomSearchResults,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("OnSearchComplete was fired for user %d, for room ID '%s' and GUID '%s'"),
        this->ActorInstanceId,
        *this->ExpectedRoomId->Get()->GetRoomId(),
        *this->SearchGuid.ToString());

    if (!Context->Assert().TestFalse(TEXT("Expected search callback to not have fired yet"), this->bGotSearchCallback))
    {
        return;
    }
    this->bGotSearchCallback = true;

    if (!Context->Assert().TestTrue(
            FString::Printf(
                TEXT("Expected second SearchRooms for lobby provider to succeed, but got error '%s'."),
                *ErrorCode.ToLogString()),
            ErrorCode.WasSuccessful()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Check that we got a result.
    if (!Context->Assert().TestEqual(
            TEXT("Expected to get exactly one search result in second SearchRooms"),
            RoomSearchResults.Num(),
            1))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();

    // Join our search result.
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Joining found search result for user %d, for room ID '%s' and GUID '%s'"),
        this->ActorInstanceId,
        *this->ExpectedRoomId->Get()->GetRoomId(),
        *this->SearchGuid.ToString());
    auto JoinRoomRequest =
        FJoinRoomRequest(ERoomScope::User, GetAccountId(Context->User(this->ActorInstanceId)), RoomSearchResults[0]);
    if (this->bHasPresence)
    {
        JoinRoomRequest.AddFeatureRequest(MakeShared<FPresenceRoomFeatureRequest>(true));
    }
    if (this->JoinRoomRequestConfiguration.IsBound())
    {
        this->JoinRoomRequestConfiguration.Execute(JoinRoomRequest);
    }
    RoomSystem->JoinRoom(
        JoinRoomRequest,
        IRoomSystem::FOnJoinRoomComplete::CreateSP(
            this,
            &FJoinRoomViaSearchChainedTask::OnJoinComplete,
            Context,
            OnDone));
}

void FJoinRoomViaSearchChainedTask::OnJoinComplete(
    const FError &ErrorCode,
    const FRoomPtr &Room,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!Context->Assert().TestFalse(TEXT("Expected join callback to not have fired yet"), this->bGotJoinCallback))
    {
        return;
    }
    this->bGotJoinCallback = true;

    Context->Assert().TestTrue(
        FString::Printf(
            TEXT("Expected JoinRoom for lobby provider to succeed, but got error '%s'."),
            *ErrorCode.ToLogString()),
        ErrorCode.WasSuccessful());

    if (!Context->Assert().TestTrue(
            FString::Printf(TEXT("Expected JoinRoom to return a valid room."), *ErrorCode.ToLogString()),
            Room.IsValid()))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Successfully joined search result for user %d, for room ID '%s' and GUID '%s'"),
        this->ActorInstanceId,
        *this->ExpectedRoomId->Get()->GetRoomId(),
        *this->SearchGuid.ToString());

    // Start a ticker that will wait for events to arrive.
    FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(
        this,
        &FJoinRoomViaSearchChainedTask::OnJoinCompleteWaitForEvents,
        Room,
        Context,
        OnDone,
        FDateTime::UtcNow() + FTimespan::FromSeconds(10)));
}

bool FJoinRoomViaSearchChainedTask::OnJoinCompleteWaitForEvents(
    float DeltaSeconds,
    FRoomPtr Room,
    FChainedTaskContextRef Context,
    FOnComplete OnDone,
    FDateTime UtcExpiry)
{
    bool bHasEvents = true;
    if (!this->bSawRoomJoinedEvent)
    {
        bHasEvents = false;
    }
    if (this->bExpectKick && !this->bSawRoomLeftEvent)
    {
        bHasEvents = false;
    }
    for (const auto &InstanceId : this->ObservingInstanceIds)
    {
        if (!this->bSawRoomMemberJoinedEvents[InstanceId])
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
                    FString::Printf(
                        TEXT("Expected OnRoomMemberJoined event to fire on instance %d observing instance %d joining."),
                        InstanceId,
                        this->ActorInstanceId),
                    this->bSawRoomMemberJoinedEvents[InstanceId]);
            }
            Context->Assert().TestTrue(
                FString::Printf(TEXT("Expected OnRoomLeft event to fire on instance %d."), this->ActorInstanceId),
                !this->bExpectKick || this->bSawRoomLeftEvent);
            Context->Assert().TestTrue(
                FString::Printf(TEXT("Expected OnRoomJoined event to fire on instance %d."), this->ActorInstanceId),
                this->bSawRoomJoinedEvent);
            OnDone.ExecuteIfBound(false);
            return false;
        }
        return true;
    }
    OnDone.ExecuteIfBound(true);
    return false;
}

void FJoinRoomViaSearchChainedTask::OnRoomMemberJoinedEvent(
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
        GetAccountId(Context->User(this->ActorInstanceId)));
    this->bSawRoomMemberJoinedEvents[InstanceId] = true;
}

void FJoinRoomViaSearchChainedTask::OnRoomJoinedEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomJoinReason Reason,
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
        "Expected join reason to be RequestedByJoinRoom.",
        Reason,
        ERoomJoinReason::RequestedByJoinRoom);
    this->bSawRoomJoinedEvent = true;
}

void FJoinRoomViaSearchChainedTask::OnRoomLeftEvent(
    const FRoomEventContext &EventContext,
    const FRoomRef &Room,
    ERoomLeaveReason Reason,
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
        "Expected leave reason to be KickedFromRoom.",
        Reason,
        ERoomLeaveReason::KickedFromRoom);
    this->bSawRoomLeftEvent = true;
}

void FJoinRoomViaSearchChainedTask::OnUnwindingDone(
    bool bSuccessful,
    FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    UE_LOG(
        LogRedpointEOSTests,
        Verbose,
        TEXT("Unbinding join events for %d, for room ID '%s' and GUID '%s'"),
        this->ActorInstanceId,
        *this->ExpectedRoomId->Get()->GetRoomId(),
        *this->SearchGuid.ToString());

    for (auto InstanceId : this->ObservingInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))))[0]
            ->OnRoomMemberJoined()
            .Remove(this->RoomMemberJoinedEventHandles[InstanceId]);
    }

    {
        auto MemberRoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(this->ActorInstanceId)).GetRoomSystem();
        MemberRoomSystem->OnRoomJoined(NAME_AutomationTests).Remove(this->RoomJoinedEventHandle);
        this->RoomJoinedEventHandle.Reset();

        if (this->bExpectKick)
        {
            MemberRoomSystem->OnRoomLeft(NAME_AutomationTests).Remove(this->RoomLeftEventHandle);
            this->RoomLeftEventHandle.Reset();
        }

        UE_LOG(
            LogRedpointEOSTests,
            Verbose,
            TEXT("Returning from chained task for %d, for room ID '%s' and GUID '%s'"),
            this->ActorInstanceId,
            *this->ExpectedRoomId->Get()->GetRoomId(),
            *this->SearchGuid.ToString());
    }

    OnDone.ExecuteIfBound(bSuccessful);
}

}