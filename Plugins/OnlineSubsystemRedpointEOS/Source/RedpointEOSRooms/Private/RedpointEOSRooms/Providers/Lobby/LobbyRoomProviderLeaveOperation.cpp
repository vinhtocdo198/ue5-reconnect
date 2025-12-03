// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderLeaveOperation.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4122519946, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderLeaveOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderLeaveOperation::FLobbyRoomProviderLeaveOperation(
    const FRoomUserId &InActorUserId,
    const FString &InLobbyId)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , LobbyId(InLobbyId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);
}

FRoomUserId FLobbyRoomProviderLeaveOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderLeaveOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomInfo>, FError>(nullptr, Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderLeaveOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderLeaveOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderLeaveOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderLeaveOperation::GetMemberInfo(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError>(
        nullptr,
        Redpoint::EOS::API::Errors::Success());
}

const FString &FLobbyRoomProviderLeaveOperation::GetLobbyId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyId;
}

}

REDPOINT_EOS_CODE_GUARD_END()