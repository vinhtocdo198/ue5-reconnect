// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderSearchOperation.h"

#include "RedpointEOSAPI/LobbySearch/SetLobbyId.h"
#include "RedpointEOSAPI/LobbySearch/SetMaxResults.h"
#include "RedpointEOSAPI/LobbySearch/SetParameter.h"
#include "RedpointEOSAPI/LobbySearch/SetTargetUserId.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Utils/JsonGetOrAdd.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2332866522, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API::LobbySearch;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderSearchOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderSearchOperation::FLobbyRoomProviderSearchOperation(
    const FRoomUserId &InActorUserId,
    TReleasableRef<EOS_HLobbySearch> InLobbySearch)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , LobbySearch(InLobbySearch)
    , CreateSearchResultCode(EOS_EResult::EOS_Success)
{
}

FLobbyRoomProviderSearchOperation::FLobbyRoomProviderSearchOperation(
    const FRoomUserId &InActorUserId,
    EOS_EResult InCreateSearchResultCode)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , LobbySearch()
    , CreateSearchResultCode(InCreateSearchResultCode)
{
}

#if !NO_LOGGING

TSharedRef<FJsonObject> FLobbyRoomProviderSearchOperation::AddSearchToDebugJson(
    const FString &OpName,
    const EOS_EResult &ResultCode)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->AddSearchToDebugJson(OpName, ConvertError(ResultCode));
}

TSharedRef<FJsonObject> FLobbyRoomProviderSearchOperation::AddSearchToDebugJson(
    const FString &OpName,
    const FError &ErrorCode)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Rooms::Utils;

    auto OperationObject = MakeShared<FJsonObject>();
    OperationObject->SetStringField("Name", OpName);
    OperationObject->SetStringField("Error", ErrorCode.ToLogString());

    auto UpdateObject = FJsonGetOrAdd::GetOrAddObjectField(this->DebugJson, "SearchOp");
    FJsonGetOrAdd::AddToArrayField(UpdateObject, "Operations", MakeShared<FJsonValueObject>(OperationObject));

    return OperationObject;
}

#endif

FRoomUserId FLobbyRoomProviderSearchOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

FError FLobbyRoomProviderSearchOperation::WithRoomId(const FRoomIdRef &RoomId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbySearch.IsValid())
    {
        EOS_EResult ResultCode =
            FSetLobbyId::Execute(this->LobbySearch.ToSharedRef(), FSetLobbyId::Options{RoomId->GetRoomId()});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddSearchToDebugJson("SetLobbyId", ResultCode);
            OperationObject->SetStringField("LobbyId", RoomId->GetRoomId());
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to set lobby ID as a filter on a lobby search."),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderSearchOperation::WithRoomMemberId(const FRoomUserId &RoomMemberId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    if (this->LobbySearch.IsValid())
    {
        EOS_EResult ResultCode = FSetTargetUserId::Execute(
            this->LobbySearch.ToSharedRef(),
            FSetTargetUserId::Options{GetProductUserId(RoomMemberId)});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddSearchToDebugJson("SetTargetUserId", ResultCode);
            OperationObject->SetStringField("RoomMemberId", GetUserIdString(RoomMemberId));
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to set room member ID as a filter on a lobby search."),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

FError FLobbyRoomProviderSearchOperation::WithAttributeConstraint(
    const FString &Key,
    const FRoomAttributeConstraint &Constraint)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    if (this->LobbySearch.IsValid())
    {
        EOS_EResult ResultCode = FSetParameter::Execute(
            this->LobbySearch.ToSharedRef(),
            FSetParameter::Options{Constraint.Attribute().ToAttributeData(Key), Constraint.GetComparisonOp()});
#if !NO_LOGGING
        if (UE_LOG_ACTIVE(LogRedpointEOSRooms, VeryVerbose))
        {
            auto OperationObject = this->AddSearchToDebugJson("AddAttribute", ResultCode);
            OperationObject->SetStringField("AttributeKey", Key);
            OperationObject->SetStringField(
                "AttributeVisibility",
                Constraint.Attribute().GetLobbyAttributeVisibility() == EOS_ELobbyAttributeVisibility::EOS_LAT_PUBLIC
                    ? "Public"
                    : "Private");
            switch (Constraint.Attribute().Type())
            {
            case FRoomAttribute::EType::String:
                OperationObject->SetStringField("AttributeValue", Constraint.Attribute().GetString());
                break;
            case FRoomAttribute::EType::Int64:
                OperationObject->SetStringField(
                    "AttributeValue",
                    FString::Printf(TEXT("(int64) %lld"), Constraint.Attribute().GetInt64()));
                break;
            case FRoomAttribute::EType::Double:
                OperationObject->SetNumberField("AttributeValue", Constraint.Attribute().GetDouble());
                break;
            case FRoomAttribute::EType::Bool:
                OperationObject->SetBoolField("AttributeValue", Constraint.Attribute().GetBool());
                break;
            default:
                OperationObject->SetField("AttributeValue", MakeShared<FJsonValueNull>());
                break;
            }
            switch (Constraint.ConstraintType())
            {
            case FRoomAttributeConstraint::EConstraintType::Equal:
                OperationObject->SetStringField("ConstraintType", "Equal");
                break;
            case FRoomAttributeConstraint::EConstraintType::NotEqual:
                OperationObject->SetStringField("ConstraintType", "NotEqual");
                break;
            case FRoomAttributeConstraint::EConstraintType::GreaterThan:
                OperationObject->SetStringField("ConstraintType", "GreaterThan");
                break;
            case FRoomAttributeConstraint::EConstraintType::GreaterThanOrEqual:
                OperationObject->SetStringField("ConstraintType", "GreaterThanOrEqual");
                break;
            case FRoomAttributeConstraint::EConstraintType::LessThan:
                OperationObject->SetStringField("ConstraintType", "LessThan");
                break;
            case FRoomAttributeConstraint::EConstraintType::LessThanOrEqual:
                OperationObject->SetStringField("ConstraintType", "LessThanOrEqual");
                break;
            case FRoomAttributeConstraint::EConstraintType::Distance:
                OperationObject->SetStringField("ConstraintType", "Distance");
                break;
            case FRoomAttributeConstraint::EConstraintType::AnyOf:
                OperationObject->SetStringField("ConstraintType", "AnyOf");
                break;
            case FRoomAttributeConstraint::EConstraintType::NotAnyOf:
                OperationObject->SetStringField("ConstraintType", "NotAnyOf");
                break;
            case FRoomAttributeConstraint::EConstraintType::OneOf:
                OperationObject->SetStringField("ConstraintType", "OneOf");
                break;
            case FRoomAttributeConstraint::EConstraintType::NotOneOf:
                OperationObject->SetStringField("ConstraintType", "NotOneOf");
                break;
            case FRoomAttributeConstraint::EConstraintType::Contains:
                OperationObject->SetStringField("ConstraintType", "Contains");
                break;
            default:
                OperationObject->SetStringField("ConstraintType", "Unknown");
                break;
            }
        }
#endif
        if (ResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(TEXT("Unable to set parameter '%s' as a filter on a lobby search."), *Key),
                ResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return Error;
        }
    }
    return Redpoint::EOS::API::Errors::Success();
}

const TSharedPtr<TReleasable<EOS_HLobbySearch>> &FLobbyRoomProviderSearchOperation::GetLobbySearch() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbySearch;
}

const EOS_EResult &FLobbyRoomProviderSearchOperation::GetCreateSearchResultCode() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->CreateSearchResultCode;
}

#if !NO_LOGGING

void FLobbyRoomProviderSearchOperation::InitializeDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // @note: We do not do checkf() here on the log verbosity level, as at least one customer reported a scenario where
    // the UE_LOG_ACTIVE expression returns a different value between InitializeOperation() and InitializeDebugJson().
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()