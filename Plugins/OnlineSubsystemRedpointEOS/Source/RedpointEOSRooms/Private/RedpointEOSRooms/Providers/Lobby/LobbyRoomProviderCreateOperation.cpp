// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderCreateOperation.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4170781936, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms;
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::Core::Id;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderCreateOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderCreateOperation::FLobbyRoomProviderCreateOperation(
    const FRoomUserId &InActorUserId,
    const FName &InRoomNamespace)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , RoomNamespace(InRoomNamespace)
    , CreateLobbyOptions(FCreateLobby::Options{
          .LocalUserId = GetProductUserId(InActorUserId),
          .MaxLobbyMembers = 1,
          .PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY,
          .bPresenceEnabled = false,
          .bAllowInvites = false,
          .BucketId = InRoomNamespace.ToString(),
          .bDisableHostMigration = false,
          .bEnableRTCRoom = false,
          .RTCRoomJoinFlags = Redpoint::EOS::API::RTC::EJoinRoomOptions::None,
          .bRTCRoomUseManualAudioInput = false,
          .bRTCRoomUseManualAudioOutput = false,
          .bRTCRoomLocalAudioDeviceInputStartsMuted = false,
          .LobbyId = TEXT(""),
          .bEnableJoinById = false,
          .bRejoinAfterKickRequiresInvite = false,
          .AllowedPlatformIds = TArray<Redpoint::EOS::API::Common::EOnlinePlatformType>(),
          .bCrossplayOptOut = false,
#if EOS_VERSION_AT_LEAST(1, 17, 0)
          .RTCRoomJoinActionType = EOS_ELobbyRTCRoomJoinActionType::EOS_LRRJAT_AutomaticJoin,
#endif
      })
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);
}

FRoomUserId FLobbyRoomProviderCreateOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderCreateOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomInfo>, FError>(nullptr, Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderCreateOperation::SetMaxMembers(uint32 MaxMembers)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderCreateOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderCreateOperation::AddAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderCreateOperation::RemoveAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderCreateOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderCreateOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TMap<FString, FRoomAttribute>, FError>(
        TMap<FString, FRoomAttribute>(),
        Redpoint::EOS::API::Errors::Success());
}

FError FLobbyRoomProviderCreateOperation::AddMemberAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderCreateOperation::RemoveMemberAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderCreateOperation::
    GetMemberInfo(const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError>(
        nullptr,
        Redpoint::EOS::API::Errors::Success());
}

FCreateLobby::Options &FLobbyRoomProviderCreateOperation::GetCreateLobbyOptions()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

#if !NO_LOGGING
    if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
    {
        // Set debug JSON since we're now at the point where we're going to use the options to create the lobby.
        auto CreateObject = MakeShared<FJsonObject>();
        CreateObject->SetNumberField("MaxMembers", (double)*this->CreateLobbyOptions.MaxLobbyMembers);
        switch (*this->CreateLobbyOptions.PermissionLevel)
        {
        case EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY:
            CreateObject->SetStringField("PermissionLevel", "InviteOnly");
            break;
        case EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE:
            CreateObject->SetStringField("PermissionLevel", "JoinViaPresence");
            break;
        case EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED:
            CreateObject->SetStringField("PermissionLevel", "PublicAdvertised");
            break;
        default:
            CreateObject->SetStringField("PermissionLevel", "Unknown");
            break;
        }
        CreateObject->SetBoolField("PresenceEnabled", *this->CreateLobbyOptions.bPresenceEnabled);
        CreateObject->SetBoolField("AllowInvites", *this->CreateLobbyOptions.bAllowInvites);
        CreateObject->SetStringField("BucketId", *this->CreateLobbyOptions.BucketId);
        CreateObject->SetBoolField("DisableHostMigration", *this->CreateLobbyOptions.bDisableHostMigration);
        CreateObject->SetBoolField("EnableRTCRoom", *this->CreateLobbyOptions.bEnableRTCRoom);
        CreateObject->SetNumberField("RTCRoomJoinFlags", (double)*this->CreateLobbyOptions.RTCRoomJoinFlags);
        CreateObject->SetBoolField("RTCRoomUseManualAudioInput", *this->CreateLobbyOptions.bRTCRoomUseManualAudioInput);
        CreateObject->SetBoolField(
            "RTCRoomUseManualAudioOutput",
            *this->CreateLobbyOptions.bRTCRoomUseManualAudioOutput);
        CreateObject->SetBoolField(
            "RTCRoomLocalAudioDeviceInputStartsMuted",
            *this->CreateLobbyOptions.bRTCRoomLocalAudioDeviceInputStartsMuted);
        CreateObject->SetStringField("LobbyId", *this->CreateLobbyOptions.LobbyId);
        CreateObject->SetBoolField("EnableJoinById", *this->CreateLobbyOptions.bEnableJoinById);
        CreateObject->SetBoolField(
            "RejoinAfterKickRequiresInvite",
            *this->CreateLobbyOptions.bRejoinAfterKickRequiresInvite);
        // @todo: AllowedPlatformIds
        CreateObject->SetBoolField("CrossplayOptOut", *this->CreateLobbyOptions.bCrossplayOptOut);

        this->DebugJson->SetObjectField("CreateOp", CreateObject);
    }
#endif

    return this->CreateLobbyOptions;
}

const FName &FLobbyRoomProviderCreateOperation::GetRoomNamespace()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomNamespace;
}

}

REDPOINT_EOS_CODE_GUARD_END()