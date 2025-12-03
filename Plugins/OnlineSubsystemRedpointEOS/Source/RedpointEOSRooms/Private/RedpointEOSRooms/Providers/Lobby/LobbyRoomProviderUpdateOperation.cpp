// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderUpdateOperation.h"

#include "RedpointEOSAPI/LobbyModification/AddAttribute.h"
#include "RedpointEOSAPI/LobbyModification/AddMemberAttribute.h"
#include "RedpointEOSAPI/LobbyModification/RemoveAttribute.h"
#include "RedpointEOSAPI/LobbyModification/RemoveMemberAttribute.h"
#include "RedpointEOSAPI/LobbyModification/SetMaxMembers.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Utils/JsonGetOrAdd.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1439984043, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API::LobbyModification;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderUpdateOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetMutableDebugJson();
}

#endif

FLobbyRoomProviderUpdateOperation::FLobbyRoomProviderUpdateOperation(
    const FRoomUserId &InActorUserId,
    const FString &InLobbyId,
    const FName &InRoomNamespace,
    TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
    TReleasableRef<EOS_HLobbyModification> InLobbyModification)
    : FLobbyRoomProviderReadOperation(InActorUserId, InLobbyDetails, false)
    , ActorUserId(InActorUserId)
    , LobbyId(InLobbyId)
    , RoomNamespace(InRoomNamespace)
    , LobbyModification(InLobbyModification)
    , UpdateLobbyModificationResultCode(EOS_EResult::EOS_Success)
{
}

FLobbyRoomProviderUpdateOperation::FLobbyRoomProviderUpdateOperation(
    const FRoomUserId &InActorUserId,
    const FString &InLobbyId,
    const FName &InRoomNamespace,
    TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
    EOS_EResult InUpdateLobbyModificationResultCode)
    : FLobbyRoomProviderReadOperation(InActorUserId, InLobbyDetails, false)
    , ActorUserId(InActorUserId)
    , LobbyId(InLobbyId)
    , RoomNamespace(InRoomNamespace)
    , LobbyModification(nullptr)
    , UpdateLobbyModificationResultCode(InUpdateLobbyModificationResultCode)
{
}

#if !NO_LOGGING

TSharedRef<FJsonObject> FLobbyRoomProviderUpdateOperation::AddUpdateToDebugJson(
    const FString &OpName,
    const EOS_EResult &ResultCode)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->AddUpdateToDebugJson(OpName, ConvertError(ResultCode));
}

TSharedRef<FJsonObject> FLobbyRoomProviderUpdateOperation::AddUpdateToDebugJson(
    const FString &OpName,
    const FError &ErrorCode)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Rooms::Utils;

    auto OperationObject = MakeShared<FJsonObject>();
    OperationObject->SetStringField("Name", OpName);
    OperationObject->SetStringField("Error", ErrorCode.ToLogString());

    auto UpdateObject = FJsonGetOrAdd::GetOrAddObjectField(this->DebugJson, "UpdateOp");
    FJsonGetOrAdd::AddToArrayField(UpdateObject, "Operations", MakeShared<FJsonValueObject>(OperationObject));

    return OperationObject;
}

#endif

FRoomUserId FLobbyRoomProviderUpdateOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderUpdateOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetInfo();
}

FError FLobbyRoomProviderUpdateOperation::SetMaxMembers(uint32 MaxMembers)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbyModification.IsValid())
    {
        EOS_EResult ResultCode =
            FSetMaxMembers::Execute(this->LobbyModification.ToSharedRef(), FSetMaxMembers::Options{MaxMembers});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddUpdateToDebugJson("SetMaxMembers", ResultCode);
            OperationObject->SetNumberField("MaxMembers", (double)MaxMembers);
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to update maximum members on lobby."),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderUpdateOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetAttributes();
}

FError FLobbyRoomProviderUpdateOperation::AddAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbyModification.IsValid())
    {
        EOS_EResult ResultCode = FAddAttribute::Execute(
            this->LobbyModification.ToSharedRef(),
            FAddAttribute::Options{Value.ToAttributeData(Key), Value.GetLobbyAttributeVisibility()});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddUpdateToDebugJson("AddAttribute", ResultCode);
            OperationObject->SetStringField("AttributeKey", Key);
            OperationObject->SetStringField(
                "AttributeVisibility",
                Value.GetLobbyAttributeVisibility() == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC ? "Public"
                                                                                                     : "Private");
            switch (Value.Type())
            {
            case FRoomAttribute::EType::String:
                OperationObject->SetStringField("AttributeValue", Value.GetString());
                break;
            case FRoomAttribute::EType::Int64:
                OperationObject->SetStringField(
                    "AttributeValue",
                    FString::Printf(TEXT("(int64) %lld"), Value.GetInt64()));
                break;
            case FRoomAttribute::EType::Double:
                OperationObject->SetNumberField("AttributeValue", Value.GetDouble());
                break;
            case FRoomAttribute::EType::Bool:
                OperationObject->SetBoolField("AttributeValue", Value.GetBool());
                break;
            default:
                OperationObject->SetField("AttributeValue", MakeShared<FJsonValueNull>());
                break;
            }
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("Unable to update add attribute '%s' to lobby."), *Key),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderUpdateOperation::RemoveAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbyModification.IsValid())
    {
        EOS_EResult ResultCode =
            FRemoveAttribute::Execute(this->LobbyModification.ToSharedRef(), FRemoveAttribute::Options{Key});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddUpdateToDebugJson("RemoveAttribute", ResultCode);
            OperationObject->SetStringField("AttributeKey", Key);
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("Unable to update remove attribute '%s' from lobby."), *Key),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderUpdateOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetMembers();
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderUpdateOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetMemberAttributes(MemberUserId);
}

FError FLobbyRoomProviderUpdateOperation::AddMemberAttribute(const FString &Key, const FRoomAttribute &Value)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbyModification.IsValid())
    {
        EOS_EResult ResultCode = FAddMemberAttribute::Execute(
            this->LobbyModification.ToSharedRef(),
            FAddMemberAttribute::Options{Value.ToAttributeData(Key), Value.GetLobbyAttributeVisibility()});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddUpdateToDebugJson("AddMemberAttribute", ResultCode);
            OperationObject->SetStringField("AttributeKey", Key);
            OperationObject->SetStringField(
                "AttributeVisibility",
                Value.GetLobbyAttributeVisibility() == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC ? "Public"
                                                                                                     : "Private");
            switch (Value.Type())
            {
            case FRoomAttribute::EType::String:
                OperationObject->SetStringField("AttributeValue", Value.GetString());
                break;
            case FRoomAttribute::EType::Int64:
                OperationObject->SetStringField(
                    "AttributeValue",
                    FString::Printf(TEXT("(int64) %lld"), Value.GetInt64()));
                break;
            case FRoomAttribute::EType::Double:
                OperationObject->SetNumberField("AttributeValue", Value.GetDouble());
                break;
            case FRoomAttribute::EType::Bool:
                OperationObject->SetBoolField("AttributeValue", Value.GetBool());
                break;
            default:
                OperationObject->SetField("AttributeValue", MakeShared<FJsonValueNull>());
                break;
            }
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("Unable to update add member attribute '%s' to lobby."), *Key),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderUpdateOperation::RemoveMemberAttribute(const FString &Key)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbyModification.IsValid())
    {
        EOS_EResult ResultCode = FRemoveMemberAttribute::Execute(
            this->LobbyModification.ToSharedRef(),
            FRemoveMemberAttribute::Options{Key});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddUpdateToDebugJson("RemoveMemberAttribute", ResultCode);
            OperationObject->SetStringField("AttributeKey", Key);
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("Unable to update remove member attribute '%s' from lobby."), *Key),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderUpdateOperation::
    GetMemberInfo(const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return FLobbyRoomProviderReadOperation::GetMemberInfo(MemberUserId);
}

const FString &FLobbyRoomProviderUpdateOperation::GetLobbyId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyId;
}

const TSharedPtr<TReleasable<EOS_HLobbyModification>> &FLobbyRoomProviderUpdateOperation::GetLobbyModification() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyModification;
}

const EOS_EResult &FLobbyRoomProviderUpdateOperation::GetUpdateLobbyModificationResultCode() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->UpdateLobbyModificationResultCode;
}

const FName &FLobbyRoomProviderUpdateOperation::GetRoomNamespace()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->RoomNamespace;
}

}

REDPOINT_EOS_CODE_GUARD_END()