// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProviderReadOperation.h"

#include "RedpointEOSAPI/LobbyDetails/CopyAttributes.h"
#include "RedpointEOSAPI/LobbyDetails/CopyInfo.h"
#include "RedpointEOSAPI/LobbyDetails/CopyMemberAttributes.h"
#include "RedpointEOSAPI/LobbyDetails/GetMembers.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Logging.h"
#include "RedpointEOSRooms/Providers/Lobby/LobbyRoomProvider.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(8442807, Redpoint::EOS::Rooms::Providers::Lobby)
{
using namespace ::Redpoint::EOS::Rooms::Providers::Lobby;
using namespace ::Redpoint::EOS::API::LobbyDetails;

#if !NO_LOGGING

TSharedRef<FJsonObject> &FLobbyRoomProviderReadOperation::GetMutableDebugJson()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->DebugJson;
}

#endif

FLobbyRoomProviderReadOperation::FLobbyRoomInfo::FLobbyRoomInfo(const FCopyInfo::Result &InResult)
    : Result(InResult)
{
}

FRoomIdRef FLobbyRoomProviderReadOperation::FLobbyRoomInfo::GetRoomId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return MakeShared<FRoomId>(Redpoint::EOS::Rooms::Providers::Lobby::LobbyRoomProviderName, this->Result.LobbyId);
}

bool FLobbyRoomProviderReadOperation::FLobbyRoomInfo::IsOwnerDedicatedServer() const
{
    return false;
}

FRoomUserId FLobbyRoomProviderReadOperation::FLobbyRoomInfo::GetOwnerUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;
    checkf(this->Result.LobbyOwnerUserId != nullptr, TEXT("Expected LobbyOwnerUserId to not be null!"));
    return GetAccountId(this->Result.LobbyOwnerUserId);
}

FString FLobbyRoomProviderReadOperation::FLobbyRoomInfo::GetOwnerDedicatedServerClientId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TEXT("");
}

uint32 FLobbyRoomProviderReadOperation::FLobbyRoomInfo::GetMaxMembers() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->Result.MaxMembers;
}

#if !NO_LOGGING

void FLobbyRoomProviderReadOperation::FLobbyRoomInfo::InitializeDebugJson(
    const TSharedRef<FJsonObject> &InfoObject) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // @note: We do not do checkf() here on the log verbosity level, as at least one customer reported a scenario where
    // the UE_LOG_ACTIVE expression returns a different value between InitializeOperation() and InitializeDebugJson().

    InfoObject->SetStringField("LobbyId", this->Result.LobbyId);
    if (this->Result.LobbyOwnerUserId == nullptr)
    {
        InfoObject->SetField("LobbyOwnerUserId", MakeShared<FJsonValueNull>());
    }
    else
    {
        InfoObject->SetStringField("LobbyOwnerUserId", GetUserIdString(GetAccountId(this->Result.LobbyOwnerUserId)));
    }
    switch (this->Result.PermissionLevel)
    {
    case EOS_ELobbyPermissionLevel::EOS_LPL_INVITEONLY:
        InfoObject->SetStringField("PermissionLevel", "InviteOnly");
        break;
    case EOS_ELobbyPermissionLevel::EOS_LPL_JOINVIAPRESENCE:
        InfoObject->SetStringField("PermissionLevel", "JoinViaPresence");
        break;
    case EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED:
        InfoObject->SetStringField("PermissionLevel", "PublicAdvertised");
        break;
    default:
        InfoObject->SetStringField("PermissionLevel", "Unknown");
        break;
    }
    InfoObject->SetNumberField("AvailableSlots", static_cast<double>(this->Result.AvailableSlots));
    InfoObject->SetNumberField("MaxMembers", static_cast<double>(this->Result.MaxMembers));
    InfoObject->SetBoolField("AllowInvites", this->Result.bAllowInvites);
    InfoObject->SetStringField("BucketId", this->Result.BucketId);
    InfoObject->SetBoolField("AllowHostMigration", this->Result.bAllowHostMigration);
    InfoObject->SetBoolField("RTCRoomEnabled", this->Result.bRTCRoomEnabled);
    InfoObject->SetBoolField("AllowJoinById", this->Result.bAllowJoinById);
    InfoObject->SetBoolField("RejoinAfterKickRequiresInvite", this->Result.bRejoinAfterKickRequiresInvite);
    InfoObject->SetBoolField("PresenceEnabled", this->Result.bPresenceEnabled);
    // @todo: AllowedPlatformIds
}

#endif

FLobbyRoomProviderReadOperation::FLobbyRoomProviderReadOperation(
    const FRoomUserId &InActorUserId,
    TReleasableRef<EOS_HLobbyDetails> InLobbyDetails,
    bool bInIsSearchResult)
    :
#if !NO_LOGGING
    DebugJson(MakeShared<FJsonObject>())
    ,
#endif
    ActorUserId(InActorUserId)
    , LobbyDetails(InLobbyDetails)
    , bIsSearchResult(bInIsSearchResult)
{
}

FRoomUserId FLobbyRoomProviderReadOperation::GetActorUserId() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->ActorUserId;
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError> FLobbyRoomProviderReadOperation::GetInfo()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    EOS_EResult CopyInfoResultCode;
    FCopyInfo::Result Result = {};
    FCopyInfo::Execute(this->LobbyDetails, FCopyInfo::Options{}, CopyInfoResultCode, Result);
    if (CopyInfoResultCode != EOS_EResult::EOS_Success)
    {
        auto Error = ConvertError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to get copy information from the lobby handle."),
            CopyInfoResultCode);
        UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
        return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError>(nullptr, Error);
    }
    if (Result.LobbyOwnerUserId == nullptr)
    {
        auto Error = Redpoint::EOS::API::Errors::Lobby::InvalidSession(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("The copied information from the lobby handle had no lobby owner specified, which is not permitted."));
        UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
        return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError>(nullptr, Error);
    }
    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomInfo>, FError>(
        MakeShared<FLobbyRoomInfo>(Result),
        Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderReadOperation::GetAttributes()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    // Attempt to copy the attributes from the room details handle.
    TArray<FCopyAttributes::ResultEntry> CopyAttributesResult;
    {
        EOS_EResult CopyAttributesResultCode;
        FCopyAttributes::Execute(
            this->LobbyDetails,
            FCopyAttributes::Options{},
            CopyAttributesResultCode,
            CopyAttributesResult);
        if (CopyAttributesResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to copy attributes from lobby handle."),
                CopyAttributesResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return TTuple<TMap<FString, FRoomAttribute>, FError>(TMap<FString, FRoomAttribute>(), Error);
        }
    }

    // Convert the attribute array into a map.
    TMap<FString, FRoomAttribute> AttributesByKey;
    for (const auto &Attr : CopyAttributesResult)
    {
        AttributesByKey.Add(
            Attr.Data.Key(),
            FRoomAttribute(
                Attr.Data,
                Attr.bPublic ? FRoomAttribute::EVisibility::Public : FRoomAttribute::EVisibility::Private));
    }
    return TTuple<TMap<FString, FRoomAttribute>, FError>(AttributesByKey, Redpoint::EOS::API::Errors::Success());
}

TTuple<TArray<FRoomUserId>, FError> FLobbyRoomProviderReadOperation::GetMembers()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // If this is a search result, return no information because the EOS SDK no longer provides it.
    if (bIsSearchResult)
    {
        return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Redpoint::EOS::API::Errors::Success());
    }

    // Attempt to copy the member list from the room details handle.
    TArray<FGetMembers::ResultEntry> GetMembersResult;
    {
        EOS_EResult GetMembersResultCode;
        FGetMembers::Execute(this->LobbyDetails, FGetMembers::Options{}, GetMembersResultCode, GetMembersResult);
        if (GetMembersResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to get member list from lobby handle."),
                GetMembersResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return TTuple<TArray<FRoomUserId>, FError>(TArray<FRoomUserId>(), Error);
        }
    }

    // Convert the members into the array.
    TArray<FRoomUserId> Result;
    for (const auto &Entry : GetMembersResult)
    {
        Result.Add(GetAccountId(Entry));
    }
    return TTuple<TArray<FRoomUserId>, FError>(Result, Redpoint::EOS::API::Errors::Success());
}

TTuple<TMap<FString, FRoomAttribute>, FError> FLobbyRoomProviderReadOperation::GetMemberAttributes(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    using namespace ::Redpoint::EOS::Core::Id;

    // If this is a search result, return no information because the EOS SDK no longer provides it.
    if (bIsSearchResult)
    {
        return TTuple<TMap<FString, FRoomAttribute>, FError>(
            TMap<FString, FRoomAttribute>(),
            Redpoint::EOS::API::Errors::NotFound(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Member attributes can not be read from search results.")));
    }

    // Attempt to copy the attributes from the room details handle.
    TArray<FCopyMemberAttributes::ResultEntry> CopyMemberAttributesResult;
    {
        EOS_EResult CopyMemberAttributesResultCode;
        FCopyMemberAttributes::Execute(
            this->LobbyDetails,
            FCopyMemberAttributes::Options{GetProductUserId(MemberUserId)},
            CopyMemberAttributesResultCode,
            CopyMemberAttributesResult);
        if (CopyMemberAttributesResultCode != EOS_EResult::EOS_Success)
        {
            auto Error = ConvertError(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("Unable to copy member attributes from lobby handle."),
                CopyMemberAttributesResultCode);
            UE_LOG(LogRedpointEOSRooms, Error, TEXT("%s"), *Error.ToLogString());
            return TTuple<TMap<FString, FRoomAttribute>, FError>(TMap<FString, FRoomAttribute>(), Error);
        }
    }

    // Convert the attribute array into a map.
    TMap<FString, FRoomAttribute> AttributesByKey;
    for (const auto &Attr : CopyMemberAttributesResult)
    {
        AttributesByKey.Add(
            Attr.Data.Key(),
            FRoomAttribute(
                Attr.Data,
                Attr.bPublic ? FRoomAttribute::EVisibility::Public : FRoomAttribute::EVisibility::Private));
    }
    return TTuple<TMap<FString, FRoomAttribute>, FError>(AttributesByKey, Redpoint::EOS::API::Errors::Success());
}

TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError> FLobbyRoomProviderReadOperation::GetMemberInfo(
    const FRoomUserId &MemberUserId)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return TTuple<TSharedPtr<IRoomProviderReadOperation::IRoomMemberInfo>, FError>(
        nullptr,
        Redpoint::EOS::API::Errors::Success());
}

const TReleasableRef<EOS_HLobbyDetails> &FLobbyRoomProviderReadOperation::GetLobbyDetails() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Providers);

    return this->LobbyDetails;
}

}

REDPOINT_EOS_CODE_GUARD_END()