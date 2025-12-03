// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/RoomSystem/Presence/AsyncHelpers.h"

#include "Containers/Ticker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3202400436, Redpoint::EOS::Tests::RoomSystem::Presence)
{

TTask<TTuple<FError, FRoomPtr>> CreateRoom(TSharedRef<IRoomSystem> RoomSystem, FCreateRoomRequest CreateRoomRequest)
{
    auto Result = TTask<TTuple<FError, FRoomPtr>>::Deferred();
    RoomSystem->CreateRoom(
        CreateRoomRequest,
        IRoomSystem::FOnCreateRoomComplete::CreateLambda([Result](const FError &ErrorCode, const FRoomPtr &Room) {
            Result.SetValue(TTuple<FError, FRoomPtr>(ErrorCode, Room));
        }));
    return Result;
}

TTask<FError> UpdateRoom(TSharedRef<IRoomSystem> RoomSystem, FRoomRef Room, FUpdateRoomRequest UpdateRoomRequest)
{
    auto Result = TTask<FError>::Deferred();
    RoomSystem->UpdateRoom(
        Room,
        UpdateRoomRequest,
        IRoomSystem::FOnUpdateRoomComplete::CreateLambda([Result](const FError &ErrorCode) {
            Result.SetValue(ErrorCode);
        }));
    return Result;
}

TTask<TTuple<FError, FRoomPtr>> JoinRoom(TSharedRef<IRoomSystem> RoomSystem, FJoinRoomRequest JoinRoomRequest)
{
    auto Result = TTask<TTuple<FError, FRoomPtr>>::Deferred();
    RoomSystem->JoinRoom(
        JoinRoomRequest,
        IRoomSystem::FOnCreateRoomComplete::CreateLambda([Result](const FError &ErrorCode, const FRoomPtr &Room) {
            Result.SetValue(TTuple<FError, FRoomPtr>(ErrorCode, Room));
        }));
    return Result;
}

TTask<FError> LeaveRoom(TSharedRef<IRoomSystem> RoomSystem, FRoomRef Room, FLeaveRoomRequest LeaveRoomRequest)
{
    auto Result = TTask<FError>::Deferred();
    RoomSystem->LeaveRoom(
        Room,
        LeaveRoomRequest,
        IRoomSystem::FOnLeaveRoomComplete::CreateLambda([Result](const FError &ErrorCode) {
            Result.SetValue(ErrorCode);
        }));
    return Result;
}

TTask<void> Delay(float Seconds)
{
    auto Result = TTask<void>::Deferred();
    FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateLambda([Result](float) -> bool {
            Result.SetValue();
            return false;
        }),
        Seconds);
    return Result;
}

TTask<TTuple<FError, FRoomPtr>> JoinRoomReliable(
    TSharedRef<FAutomationTestBase> _Assert,
    TSharedRef<IRoomSystem> _RoomSystem,
    FJoinRoomRequest _JoinRoomRequest)
{
    return (
        [](TSharedRef<IRoomSystem> RoomSystem,
           FJoinRoomRequest JoinRoomRequest) -> TTask<TTuple<FError, FRoomPtr>>::With<ETaskBinding::Lambda> {
            TOptional<FError> ErrorCode;
            FRoomPtr Room;
            int Attempts = 0;
            do
            {
                Tie(ErrorCode, Room) = co_await JoinRoom(RoomSystem, JoinRoomRequest);
                if (ErrorCode->ResultCode() == EOS_EResult::EOS_NotFound)
                {
                    co_await Delay(1.0f);
                }
            } while (ErrorCode->ResultCode() == EOS_EResult::EOS_NotFound && Attempts++ < 30);
            co_return {*ErrorCode, Room};
        })(_RoomSystem, _JoinRoomRequest);
}

TTask<TTuple<FError, TArray<FRoomSearchResultRef>>> SearchRooms(
    TSharedRef<IRoomSystem> RoomSystem,
    FSearchRoomsRequest SearchRoomsRequest)
{
    auto Result = TTask<TTuple<FError, TArray<FRoomSearchResultRef>>>::Deferred();
    RoomSystem->SearchRooms(
        SearchRoomsRequest,
        IRoomSystem::FOnSearchRoomsComplete::CreateLambda(
            [Result](const FError &ErrorCode, const TArray<FRoomSearchResultRef> &RoomSearchResults) {
                Result.SetValue(TTuple<FError, TArray<FRoomSearchResultRef>>(ErrorCode, RoomSearchResults));
            }));
    return Result;
}

TTask<TTuple<FError, TArray<FRoomSearchResultRef>>> SearchRoomsReliable(
    TSharedRef<IRoomSystem> _RoomSystem,
    FSearchRoomsRequest _SearchRoomsRequest)
{
    return (
        [](TSharedRef<IRoomSystem> RoomSystem, FSearchRoomsRequest SearchRoomsRequest)
            -> TTask<TTuple<FError, TArray<FRoomSearchResultRef>>>::With<ETaskBinding::Lambda> {
            TOptional<FError> ErrorCode;
            TArray<FRoomSearchResultRef> SearchResults;
            int Attempts = 0;
            do
            {
                Tie(ErrorCode, SearchResults) = co_await SearchRooms(RoomSystem, SearchRoomsRequest);
                if (SearchResults.Num() == 0)
                {
                    co_await Delay(1.0f);
                }
            } while (SearchResults.Num() == 0 && Attempts++ < 30);
            co_return {*ErrorCode, SearchResults};
        })(_RoomSystem, _SearchRoomsRequest);
}

TTask<FError> SendRoomInvite(
    TSharedRef<IRoomSystem> RoomSystem,
    FRoomRef Room,
    FSendRoomInviteRequest SendInviteRequest)
{
    auto Result = TTask<FError>::Deferred();
    RoomSystem->SendRoomInvite(
        Room,
        SendInviteRequest,
        IRoomSystem::FOnSendRoomInviteComplete::CreateLambda([Result](const FError &ErrorCode) {
            Result.SetValue(ErrorCode);
        }));
    return Result;
}

}

REDPOINT_EOS_CODE_GUARD_END()