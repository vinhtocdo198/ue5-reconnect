// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureImpl.h"

#include "Misc/SecureHash.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(249413209, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

EPartyConfigurationRoomFeatureJoinRequestAction FPartyConfigurationRoomFeatureImpl::GetJoinRequestAction() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinRequestAction;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureImpl::GetJoinByPresencePermissions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinByPresencePermissions;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureImpl::GetSendInvitePermissions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->SendInvitePermissions;
}

FString FPartyConfigurationRoomFeatureImpl::GetNickname() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Nickname;
}

FString FPartyConfigurationRoomFeatureImpl::GetDescription() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Description;
}

FString FPartyConfigurationRoomFeatureImpl::GetPassword() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Password;
}

TSharedPtr<IRoomFeatureRequestBase> FPartyConfigurationRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPartyConfigurationRoomFeatureRequest>(
        this->JoinRequestAction,
        this->JoinByPresencePermissions,
        this->SendInvitePermissions,
        this->Nickname,
        this->Description,
        this->Password);
}

TSharedRef<FPartyConfigurationRoomFeatureSearchStateImpl> FPartyConfigurationRoomFeatureImpl::
    ConstructFeatureSearchState(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const TMap<FString, FRoomAttribute> &FeatureAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    auto SearchState = MakeShared<FPartyConfigurationRoomFeatureSearchStateImpl>();
    {
        auto JoinRequestActionAttribute = FeatureAttributes.Find(TEXT("JoinRequestAction"));
        if (JoinRequestActionAttribute != nullptr && JoinRequestActionAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            SearchState->JoinRequestAction =
                (EPartyConfigurationRoomFeatureJoinRequestAction)JoinRequestActionAttribute->GetInt64();
        }
    }
    {
        auto JoinByPresencePermissionsAttribute = FeatureAttributes.Find(TEXT("JoinByPresencePermissions"));
        if (JoinByPresencePermissionsAttribute != nullptr &&
            JoinByPresencePermissionsAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            SearchState->JoinByPresencePermissions =
                (EPartyConfigurationRoomFeaturePermissionType)JoinByPresencePermissionsAttribute->GetInt64();
        }
    }
    {
        auto SendInvitePermissionsAttribute = FeatureAttributes.Find(TEXT("SendInvitePermissions"));
        if (SendInvitePermissionsAttribute != nullptr &&
            SendInvitePermissionsAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            SearchState->SendInvitePermissions =
                (EPartyConfigurationRoomFeaturePermissionType)SendInvitePermissionsAttribute->GetInt64();
        }
    }
    {
        auto NicknameAttribute = FeatureAttributes.Find(TEXT("Nickname"));
        if (NicknameAttribute != nullptr && NicknameAttribute->Type() == FRoomAttribute::EType::String)
        {
            SearchState->Nickname = NicknameAttribute->GetString();
        }
    }
    {
        auto DescriptionAttribute = FeatureAttributes.Find(TEXT("Description"));
        if (DescriptionAttribute != nullptr && DescriptionAttribute->Type() == FRoomAttribute::EType::String)
        {
            SearchState->Description = DescriptionAttribute->GetString();
        }
    }
    {
        auto bIsPasswordRequiredAttribute = FeatureAttributes.Find(TEXT("bIsPasswordRequired"));
        if (bIsPasswordRequiredAttribute != nullptr &&
            bIsPasswordRequiredAttribute->Type() == FRoomAttribute::EType::String)
        {
            SearchState->bIsPasswordRequired = bIsPasswordRequiredAttribute->GetBool();
        }
    }
    return SearchState;
}

TSharedRef<FPartyConfigurationRoomFeatureImpl> FPartyConfigurationRoomFeatureImpl::ConstructFeatureImplementation(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FPartyConfigurationRoomFeatureImpl>();
}

FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult FPartyConfigurationRoomFeatureImpl::
    UpdateFeatureImplementationDuringPostSynchronisation(
        const FRoomProviderReadOperationRef &ReadOp,
        const TSharedRef<FRoomProvider> &RoomProvider,
        const FRoomRef &Room,
        const TSharedRef<FPartyConfigurationRoomFeatureImpl> &Feature,
        const TMap<FString, FRoomAttribute> &FeatureAttributes,
        const TMap<FRoomUserId, TMap<FString, FRoomAttribute>> &FeatureMemberAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    bool bUpdated = false;

    {
        auto JoinRequestActionAttribute = FeatureAttributes.Find(TEXT("JoinRequestAction"));
        if (JoinRequestActionAttribute != nullptr && JoinRequestActionAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            bUpdated =
                bUpdated || (Feature->JoinRequestAction !=
                             (EPartyConfigurationRoomFeatureJoinRequestAction)JoinRequestActionAttribute->GetInt64());
            Feature->JoinRequestAction =
                (EPartyConfigurationRoomFeatureJoinRequestAction)JoinRequestActionAttribute->GetInt64();
        }
    }
    {
        auto JoinByPresencePermissionsAttribute = FeatureAttributes.Find(TEXT("JoinByPresencePermissions"));
        if (JoinByPresencePermissionsAttribute != nullptr &&
            JoinByPresencePermissionsAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            bUpdated = bUpdated ||
                       (Feature->JoinByPresencePermissions !=
                        (EPartyConfigurationRoomFeaturePermissionType)JoinByPresencePermissionsAttribute->GetInt64());
            Feature->JoinByPresencePermissions =
                (EPartyConfigurationRoomFeaturePermissionType)JoinByPresencePermissionsAttribute->GetInt64();
        }
    }
    {
        auto SendInvitePermissionsAttribute = FeatureAttributes.Find(TEXT("SendInvitePermissions"));
        if (SendInvitePermissionsAttribute != nullptr &&
            SendInvitePermissionsAttribute->Type() == FRoomAttribute::EType::Int64)
        {
            bUpdated =
                bUpdated || (Feature->SendInvitePermissions !=
                             (EPartyConfigurationRoomFeaturePermissionType)SendInvitePermissionsAttribute->GetInt64());
            Feature->SendInvitePermissions =
                (EPartyConfigurationRoomFeaturePermissionType)SendInvitePermissionsAttribute->GetInt64();
        }
    }
    {
        auto NicknameAttribute = FeatureAttributes.Find(TEXT("Nickname"));
        if (NicknameAttribute != nullptr && NicknameAttribute->Type() == FRoomAttribute::EType::String)
        {
            bUpdated = bUpdated || (Feature->Nickname != NicknameAttribute->GetString());
            Feature->Nickname = NicknameAttribute->GetString();
        }
    }
    {
        auto DescriptionAttribute = FeatureAttributes.Find(TEXT("Description"));
        if (DescriptionAttribute != nullptr && DescriptionAttribute->Type() == FRoomAttribute::EType::String)
        {
            bUpdated = bUpdated || (Feature->Description != DescriptionAttribute->GetString());
            Feature->Description = DescriptionAttribute->GetString();
        }
    }
    {
        // Try to get the password from the room attributes; this will only work for the host.
        auto HostPasswordAttribute = FeatureAttributes.Find(TEXT("Password"));
        if (HostPasswordAttribute != nullptr && HostPasswordAttribute->Type() == FRoomAttribute::EType::String)
        {
            bUpdated = bUpdated || (Feature->Password != HostPasswordAttribute->GetString());
            Feature->Password = HostPasswordAttribute->GetString();
        }
        else
        {
            // If we can't get the password from the room attributes, try to get the password from the member
            // attributes, which will be set if we are a client joining to a room.
            auto UserAttributeMap = FeatureMemberAttributes.Find(ReadOp->GetActorUserId());
            if (UserAttributeMap != nullptr)
            {
                auto ClientPasswordAttribute = UserAttributeMap->Find(TEXT("Password"));
                if (ClientPasswordAttribute != nullptr &&
                    ClientPasswordAttribute->Type() == FRoomAttribute::EType::String)
                {
                    bUpdated = bUpdated || (Feature->Password != ClientPasswordAttribute->GetString());
                    Feature->Password = ClientPasswordAttribute->GetString();
                }
            }
        }
    }

    return FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult(
        FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::ERequireRoomUpdate::No,
        bUpdated ? FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::Yes
                 : FRoomProviderFeatureHandlerBase::FFeaturePostSynchronisationResult::EStateUpdated::No);
}

TMap<FString, FRoomAttribute> FPartyConfigurationRoomFeatureImpl::SerializeFeatureRequestToRoomAttributes(
    const FRoomProviderWriteOperationRef &WriteOp,
    const TSharedRef<FPartyConfigurationRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return TMap<FString, FRoomAttribute>{
        {TEXT("JoinRequestAction"), (int64)FeatureRequest->GetJoinRequestAction()},
        {TEXT("JoinByPresencePermissions"), (int64)FeatureRequest->GetJoinByPresencePermissions()},
        {TEXT("SendInvitePermissions"), (int64)FeatureRequest->GetSendInvitePermissions()},
        {TEXT("Nickname"), FeatureRequest->GetNickname()},
        {TEXT("Description"), FeatureRequest->GetDescription()},
        // @note: FRoomAttribute::EVisibility::Private prevents anyone from reading the password except for the user
        // setting this attribute.
        {TEXT("Password"), FRoomAttribute(FeatureRequest->GetPassword(), FRoomAttribute::EVisibility::Private)},
        {TEXT("bIsPasswordRequired"), !FeatureRequest->GetPassword().IsEmpty()},
    };
}

TMap<FString, FRoomAttribute> FPartyConfigurationRoomFeatureImpl::SerializeFeatureRequestToRoomMemberAttributes(
    const FRoomProviderWriteOperationRef &WriteOp,
    const TSharedRef<FPartyConfigurationRoomFeatureRequest> &FeatureRequest)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    using namespace ::Redpoint::EOS::Core::Id;

    FString SaltedPassword =
        FString::Printf(TEXT("%s|%s"), *GetUserIdString(WriteOp->GetActorUserId()), *FeatureRequest->GetPassword());
    FSHAHash SaltedPasswordHash;
    FSHA1::HashBuffer(
        *SaltedPassword,
        static_cast<uint64>(SaltedPassword.Len()) * sizeof(FString::ElementType),
        SaltedPasswordHash.Hash);

    return TMap<FString, FRoomAttribute>{
        // @note: FRoomAttribute::EVisibility::Private prevents anyone from reading the password except for the user
        // setting this attribute.
        {TEXT("Password"), FRoomAttribute(FeatureRequest->GetPassword(), FRoomAttribute::EVisibility::Private)},
        // This is visible to everyone, but it's salted with the user's ID so another user can't just copy the value and
        // reuse it themselves. The host of the room verifies this attribute when the room member becomes ready, kicking
        // them if they didn't set the correct value.
        //
        // During host migration, the room member that gets promoted will already have a copy of the (correct) password
        // since they weren't kicked by the previous owner.
        {TEXT("HashedSaltedPassword"), SaltedPasswordHash.ToString()},
    };
}

bool FPartyConfigurationRoomFeatureImpl::ShouldKickReadyRoomMemberAfterJoin(
    const FRoomProviderReadOperationRef &ReadOp,
    const TSharedRef<FRoomProvider> &RoomProvider,
    const FRoomRef &Room,
    const TSharedRef<FPartyConfigurationRoomFeatureImpl> &Feature,
    FRoomUserId RoomMemberId,
    const TMap<FString, FRoomAttribute> &FeatureMemberAttributes)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    using namespace ::Redpoint::EOS::Core::Id;

    // If the room doesn't have a password, don't kick room members.
    if (Feature->GetPassword().IsEmpty())
    {
        return false;
    }

    // Calculate the expected hash.
    FString SaltedPassword = FString::Printf(TEXT("%s|%s"), *GetUserIdString(RoomMemberId), *Feature->GetPassword());
    FSHAHash SaltedPasswordHash;
    FSHA1::HashBuffer(
        *SaltedPassword,
        static_cast<uint64>(SaltedPassword.Len()) * sizeof(FString::ElementType),
        SaltedPasswordHash.Hash);

    // Compare it with the provided hash.
    auto HashedSaltedPasswordAttribute = FeatureMemberAttributes.Find(TEXT("HashedSaltedPassword"));
    if (HashedSaltedPasswordAttribute == nullptr ||
        HashedSaltedPasswordAttribute->Type() != FRoomAttribute::EType::String)
    {
        // User didn't provide password at all.
        UE_LOG(
            LogRedpointEOSRooms,
            Warning,
            TEXT("Kicking room member '%s' because they did not provide a 'HashedSaltedPassword' attribute, which is "
                 "required when joining a room with a password-enabled PartyConfigurationRoomFeature."),
            *GetUserIdString(RoomMemberId));
        return true;
    }
    if (HashedSaltedPasswordAttribute->GetString() != SaltedPasswordHash.ToString())
    {
        // Password is incorrect.
        UE_LOG(
            LogRedpointEOSRooms,
            Warning,
            TEXT("Kicking room member '%s' because the value of the 'HashedSaltedPassword' attribute they provided "
                 "does not match the expected password hash. Joining members must provide the correct password when "
                 "joining a room with a password-enabled PartyConfigurationRoomFeature."),
            *GetUserIdString(RoomMemberId));
        return true;
    }

    // The password they provided was correct.
    return false;
}

}

REDPOINT_EOS_CODE_GUARD_END()