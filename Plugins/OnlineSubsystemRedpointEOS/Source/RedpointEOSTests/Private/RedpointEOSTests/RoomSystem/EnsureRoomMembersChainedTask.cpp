// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/EnsureRoomMembersChainedTask.h"

#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSTests/RoomSystem/GetCurrentRoomsRequestHelper.h"

namespace REDPOINT_EOS_FILE_NS_ID(4232089186, Redpoint::EOS::Tests::RoomSystem)
{
using namespace ::Redpoint::EOS::Tests::RoomSystem;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Requests;
using namespace ::Redpoint::EOS::Tests::TestUtilities;
using namespace ::Redpoint::EOS::Tests::TestUtilities::ChainedTask;
using namespace ::Redpoint::EOS::Core::Id;

FEnsureRoomMembersChainedTask::FEnsureRoomMembersChainedTask(
    TSet<int32> InPresentInstanceIds,
    TSet<int32> InNotPresentInstanceIds,
    TSharedRef<FRoomIdPtr> InExpectedRoomId)
    : PresentInstanceIds(InPresentInstanceIds)
    , NotPresentInstanceIds(InNotPresentInstanceIds)
    , ExpectedRoomId(InExpectedRoomId)
{
}

void FEnsureRoomMembersChainedTask::ExecuteAsync(const FChainedTaskContextRef &Context, const FOnComplete &OnDone)
{
    TSet<FRoomUserId> PresentUserIds;
    for (const auto &InstanceId : this->PresentInstanceIds)
    {
        PresentUserIds.Add(GetAccountId(Context->User(InstanceId)));
    }
    TSet<FRoomUserId> NotPresentUserIds;
    for (const auto &InstanceId : this->NotPresentInstanceIds)
    {
        NotPresentUserIds.Add(GetAccountId(Context->User(InstanceId)));
    }

    bool bSucceeded = true;
    for (const auto &InstanceId : this->PresentInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        auto CurrentRooms =
            RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))));
        if (!Context->Assert().TestEqual(
                FString::Printf(TEXT("Expected instance %d to have 1 room."), InstanceId),
                CurrentRooms.Num(),
                1))
        {
            bSucceeded = false;
            continue;
        }
        auto CurrentRoom = CurrentRooms[0];
        if (!Context->Assert().TestTrue(
                FString::Printf(
                    TEXT("Expected instance %d's only room to have ID '%s'."),
                    InstanceId,
                    *this->ExpectedRoomId->Get()->ToString()),
                CurrentRoom->GetRoomId()->IsEqual(*this->ExpectedRoomId->Get())))
        {
            bSucceeded = false;
            continue;
        }
        auto RoomMembers = CurrentRoom->GetMembers();
        if (!Context->Assert().TestEqual(
                FString::Printf(
                    TEXT("Expected instance %d's only room to have %d members."),
                    InstanceId,
                    this->PresentInstanceIds.Num()),
                RoomMembers.Num(),
                this->PresentInstanceIds.Num()))
        {
            bSucceeded = false;
            continue;
        }
        TSet<FRoomUserId> RoomMemberIds;
        for (const auto &RoomMember : RoomMembers)
        {
            RoomMemberIds.Add(RoomMember->GetUserId());
        }
        for (const auto &UserId : PresentUserIds)
        {
            if (!Context->Assert().TestTrue(
                    FString::Printf(
                        TEXT("Expected instance %d's room to have '%s' as a member."),
                        InstanceId,
                        *GetUserIdString(UserId)),
                    RoomMemberIds.Contains(UserId)))
            {
                bSucceeded = false;
                continue;
            }
        }
        for (const auto &UserId : NotPresentUserIds)
        {
            if (!Context->Assert().TestFalse(
                    FString::Printf(
                        TEXT("Expected instance %d's room to not have '%s' as a member."),
                        InstanceId,
                        *GetUserIdString(UserId)),
                    RoomMemberIds.Contains(UserId)))
            {
                bSucceeded = false;
                continue;
            }
        }
    }
    for (const auto &InstanceId : this->NotPresentInstanceIds)
    {
        auto RoomSystem = ((FOnlineSubsystemEOS &)Context->Instance(InstanceId)).GetRoomSystem();
        auto CurrentRooms =
            RoomSystem->GetCurrentRooms(GetCurrentRoomsRequest(GetAccountId(Context->User(InstanceId))));
        if (!Context->Assert().TestEqual(
                FString::Printf(TEXT("Expected instance %d to have 0 rooms."), InstanceId),
                CurrentRooms.Num(),
                0))
        {
            bSucceeded = false;
            continue;
        }
    }
    OnDone.ExecuteIfBound(bSucceeded);
}

}