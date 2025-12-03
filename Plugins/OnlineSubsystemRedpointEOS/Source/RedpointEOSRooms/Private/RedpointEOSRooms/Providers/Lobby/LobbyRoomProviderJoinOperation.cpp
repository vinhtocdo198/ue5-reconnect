// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderJoinOperation.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3664703955, Redpoint::EOS::Rooms::Providers::Lobby)
{

using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::Core::Id;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderJoinOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderJoinOperation::FLobbyRoomProviderJoinOperation(const FRoomUserId &InActorUserId)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , JoinByIdLobbyOptions(FJoinLobbyById::Options{
          .LocalUserId = GetProductUserId(InActorUserId),
          .LobbyId = TEXT(""),
          .bPresenceEnabled = false,
          .RTCRoomJoinFlags = Redpoint::EOS::API::RTC::EJoinRoomOptions::None,
          .bRTCRoomUseManualAudioInput = false,
          .bRTCRoomUseManualAudioOutput = false,
          .bRTCRoomLocalAudioDeviceInputStartsMuted = false,
          .bCrossplayOptOut = false,
#if EOS_VERSION_AT_LEAST(1, 17, 0)
          .RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin,
#endif
      })
    , JoinLobbyId()
    , JoinLobbyDetails()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);
}

FLobbyRoomProviderJoinOperation::FLobbyRoomProviderJoinOperation(
    const FRoomUserId &InActorUserId,
    const FString &InJoinLobbyId)
    : FLobbyRoomProviderJoinOperation(InActorUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->JoinLobbyId = InJoinLobbyId;
    this->JoinByIdLobbyOptions.LobbyId = InJoinLobbyId;
}

FLobbyRoomProviderJoinOperation::FLobbyRoomProviderJoinOperation(
    const FRoomUserId &InActorUserId,
    const TReleasableRef<EOS_HLobbyDetails> &InJoinLobbyDetails)
    : FLobbyRoomProviderJoinOperation(InActorUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    this->JoinLobbyDetails = InJoinLobbyDetails;
}

FRoomUserId FLobbyRoomProviderJoinOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderJoinOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomInfo>, FError>(nullptr, Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderJoinOperation::SetMaxMembers(uint32 MaxMembers)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderJoinOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderJoinOperation::AddAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderJoinOperation::RemoveAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderJoinOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderJoinOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderJoinOperation::AddMemberAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderJoinOperation::RemoveMemberAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderJoinOperation::GetMemberInfo(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError>(
        nullptr,
        Redpoint::EOS::API::Errors::Success());
}

bool FLobbyRoomProviderJoinOperation::IsJoinById() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return !this->JoinLobbyDetails.IsValid();
}

FJoinLobbyById::Options &FLobbyRoomProviderJoinOperation::GetJoinLobbyByIdOptions()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->JoinByIdLobbyOptions;
}

FJoinLobby::Options FLobbyRoomProviderJoinOperation::GetJoinLobbyOptions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FJoinLobby::Options{
        .LocalUserId = this->JoinByIdLobbyOptions.LocalUserId,
        .LobbyDetails = this->JoinLobbyDetails.ToSharedRef(),
        .bPresenceEnabled = this->JoinByIdLobbyOptions.bPresenceEnabled,
        .RTCRoomJoinFlags = this->JoinByIdLobbyOptions.RTCRoomJoinFlags,
        .bRTCRoomUseManualAudioInput = this->JoinByIdLobbyOptions.bRTCRoomUseManualAudioInput,
        .bRTCRoomUseManualAudioOutput = this->JoinByIdLobbyOptions.bRTCRoomUseManualAudioOutput,
        .bRTCRoomLocalAudioDeviceInputStartsMuted = this->JoinByIdLobbyOptions.bRTCRoomLocalAudioDeviceInputStartsMuted,
        .bCrossplayOptOut = this->JoinByIdLobbyOptions.bCrossplayOptOut,
#if EOS_VERSION_AT_LEAST(1, 17, 0)
        .RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin,
#endif
    };
}

}

REDPOINT_EOS_CODE_GUARD_END()