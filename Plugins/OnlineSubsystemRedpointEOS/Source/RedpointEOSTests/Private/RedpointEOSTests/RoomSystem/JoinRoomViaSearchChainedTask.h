// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSRooms/Requests/JoinRoomRequest.h"
#include "RedpointEOSRooms/Room.h"
#include "RedpointEOSRooms/RoomJoinReason.h"
#include "RedpointEOSRooms/RoomLeaveReason.h"
#include "RedpointEOSRooms/RoomSearchResult.h"
#include "RedpointEOSTests/TestUtilities/ChainedTask/ChainedTask.h"
#include "RedpointEOSTests/TestUtilities/ChainedTaskContext.h"

namespace REDPOINT_EOS_FILE_NS_ID(1028870339, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;

class FJoinRoomViaSearchChainedTask : public FChainedTask, public TSharedFromThis<FJoinRoomViaSearchChainedTask>
{
private:
    int32 ActorInstanceId;
    TSet<int32> ObservingInstanceIds;
    FGuid SearchGuid;
    TSharedRef<FRoomIdPtr> ExpectedRoomId;
    bool bHasPresence;
    TDelegate<void(FJoinRoomRequest &Request)> JoinRoomRequestConfiguration;
    bool bExpectKick;
    bool bGotSearchCallback;
    bool bGotJoinCallback;
    TMap<int32, FDelegateHandle> RoomMemberJoinedEventHandles;
    FDelegateHandle RoomJoinedEventHandle;
    FDelegateHandle RoomLeftEventHandle;
    TMap<int32, bool> bSawRoomMemberJoinedEvents;
    bool bSawRoomJoinedEvent;
    bool bSawRoomLeftEvent;

public:
    FJoinRoomViaSearchChainedTask(
        int32 InActorInstanceId,
        TSet<int32> InObservingInstanceIds,
        FGuid InSearchGuid,
        TSharedRef<FRoomIdPtr> InExpectedRoomId,
        bool bInHasPresence,
        TDelegate<void(FJoinRoomRequest &Request)> InJoinRoomRequestConfiguration =
            TDelegate<void(FJoinRoomRequest &Request)>(),
        bool bInExpectKick = false);
    UE_NONCOPYABLE(FJoinRoomViaSearchChainedTask);
    virtual ~FJoinRoomViaSearchChainedTask() = default;

    virtual void ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone);

private:
    void OnSearchComplete(
        const FError &ErrorCode,
        const TArray<FRoomSearchResultRef> &RoomSearchResults,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    void OnJoinComplete(
        const FError &ErrorCode,
        const FRoomPtr &Room,
        FChainedTaskContextRef Context,
        FOnComplete OnDone);

    bool OnJoinCompleteWaitForEvents(
        float DeltaSeconds,
        FRoomPtr Room,
        FChainedTaskContextRef Context,
        FOnComplete OnDone,
        FDateTime UtcExpiry);

    void OnRoomMemberJoinedEvent(
        const FRoomEventContext &EventContext,
        const TSharedRef<IRoom> &Room,
        FRoomUserId TargetUser,
        FChainedTaskContextRef Context,
        int32 InstanceId);

    void OnRoomJoinedEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        ERoomJoinReason Reason,
        FChainedTaskContextRef Context);

    void OnRoomLeftEvent(
        const FRoomEventContext &EventContext,
        const FRoomRef &Room,
        ERoomLeaveReason Reason,
        FChainedTaskContextRef Context);

    void OnUnwindingDone(bool bSuccessful, FChainedTaskContextRef Context, FOnComplete OnDone);
};

}

namespace Redpoint::EOS::Tests::RoomSystem
{
REDPOINT_EOS_FILE_NS_EXPORT(1028870339, Redpoint::EOS::Tests::RoomSystem, FJoinRoomViaSearchChainedTask)
}