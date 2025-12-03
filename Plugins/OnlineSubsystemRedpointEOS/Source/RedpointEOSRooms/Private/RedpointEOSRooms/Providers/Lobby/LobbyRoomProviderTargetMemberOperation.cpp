// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderTargetMemberOperation.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4005217892, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderTargetMemberOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderTargetMemberOperation::FLobbyRoomProviderTargetMemberOperation(
    const FRoomUserId &InActorUserId,
    const FString &InLobbyId,
    const FRoomUserId &InTargetUserId)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , LobbyId(InLobbyId)
    , TargetUserId(InTargetUserId)
{
}

FRoomUserId FLobbyRoomProviderTargetMemberOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderTargetMemberOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomInfo>, FError>(nullptr, Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderTargetMemberOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderTargetMemberOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderTargetMemberOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderTargetMemberOperation::
    GetMemberInfo(const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError>(
        nullptr,
        Redpoint::EOS::API::Errors::Success());
}

const FString &FLobbyRoomProviderTargetMemberOperation::GetLobbyId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyId;
}

const FRoomUserId &FLobbyRoomProviderTargetMemberOperation::GetTargetUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->TargetUserId;
}

}

REDPOINT_EOS_CODE_GUARD_END()