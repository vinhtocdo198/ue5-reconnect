// Copyright June Rhodes. All Rights Reserved.

#include "RedpointPartyMember.h"

#include "LogRedpointEOSFramework.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/RoomSystem.h"
#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

void ARedpointPartyMember::PartyMemberChanged_Implementation(const FRedpointPartyMemberInfo &PartyMember)
{
    this->NativePartyMemberChanged(PartyMember);
}

void ARedpointPartyMember::NativePartyMemberChanged(const FRedpointPartyMemberInfo &PartyMember)
{
}

void ARedpointPartyMember::BeginPlay()
{
    using namespace ::Redpoint::EOS::Identity;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Rooms::Requests;
    using namespace ::Redpoint::EOS::Rooms::Features;
    using namespace ::Redpoint::EOS::Core::Utils;

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        UE_LOG(
            LogRedpointEOSFramework,
            Error,
            TEXT("Redpoint EOS platform handle is not available for the current world."));
        this->FirePartyMemberChanged();
        return;
    }

    auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
    auto RoomSystem = PlatformHandle->GetSystem<IRoomSystem>();

    this->OnRoomJoinedHandle =
        RoomSystem->OnRoomJoined(NAME_Game).AddUObject(this, &ARedpointPartyMember::OnRoomJoined);
    this->OnRoomLeftHandle = RoomSystem->OnRoomLeft(NAME_Game).AddUObject(this, &ARedpointPartyMember::OnRoomLeft);

    for (const auto &User : IdentitySystem->GetUsers())
    {
        auto JoinedRooms =
            RoomSystem->GetCurrentRooms(FGetCurrentRoomsRequest(ERoomScope::User, User->GetUserId(), NAME_Game));
        for (const auto &Room : JoinedRooms)
        {
            auto PartyTypeFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
            if (PartyTypeFeature.IsValid())
            {
                this->CurrentPartyRoom = Room;
                break;
            }
        }
        if (this->CurrentPartyRoom.IsValid())
        {
            break;
        }
    }

    if (this->CurrentPartyRoom.IsValid())
    {
        this->CurrentPartyRoom->OnRoomMemberJoined().AddUObject(this, &ARedpointPartyMember::OnRoomMemberJoined);
        this->CurrentPartyRoom->OnRoomMemberUpdated().AddUObject(this, &ARedpointPartyMember::OnRoomMemberUpdated);
        this->CurrentPartyRoom->OnRoomMemberPromoted().AddUObject(this, &ARedpointPartyMember::OnRoomMemberPromoted);
        this->CurrentPartyRoom->OnRoomMemberLeft().AddUObject(this, &ARedpointPartyMember::OnRoomMemberLeft);
    }

    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::BeginDestroy()
{
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::Core::Utils;

    Super::BeginDestroy();

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        return;
    }

    auto RoomSystem = PlatformHandle->GetSystem<IRoomSystem>();

    if (this->CurrentPartyRoom.IsValid())
    {
        this->CurrentPartyRoom->OnRoomMemberJoined().Remove(this->OnRoomMemberJoinedHandle);
        this->CurrentPartyRoom->OnRoomMemberUpdated().Remove(this->OnRoomMemberUpdatedHandle);
        this->CurrentPartyRoom->OnRoomMemberPromoted().Remove(this->OnRoomMemberPromotedHandle);
        this->CurrentPartyRoom->OnRoomMemberLeft().Remove(this->OnRoomMemberLeftHandle);
    }

    RoomSystem->OnRoomJoined(NAME_Game).Remove(this->OnRoomJoinedHandle);
    RoomSystem->OnRoomLeft(NAME_Game).Remove(this->OnRoomLeftHandle);
}

void ARedpointPartyMember::OnRoomJoined(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const ::Redpoint::EOS::Rooms::FRoomRef &Room,
    ::Redpoint::EOS::Rooms::ERoomJoinReason Reason)
{
    using namespace ::Redpoint::EOS::Rooms::Features;

    if (this->CurrentPartyRoom.IsValid())
    {
        return;
    }

    auto PartyTypeFeature = Room->GetRoomFeature<IPartyTypeRoomFeature>();
    if (!PartyTypeFeature.IsValid())
    {
        return;
    }

    this->CurrentPartyRoom = Room;

    this->CurrentPartyRoom->OnRoomMemberJoined().AddUObject(this, &ARedpointPartyMember::OnRoomMemberJoined);
    this->CurrentPartyRoom->OnRoomMemberUpdated().AddUObject(this, &ARedpointPartyMember::OnRoomMemberUpdated);
    this->CurrentPartyRoom->OnRoomMemberPromoted().AddUObject(this, &ARedpointPartyMember::OnRoomMemberPromoted);
    this->CurrentPartyRoom->OnRoomMemberLeft().AddUObject(this, &ARedpointPartyMember::OnRoomMemberLeft);

    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::OnRoomLeft(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const ::Redpoint::EOS::Rooms::FRoomRef &Room,
    ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason)
{
    if (Room == this->CurrentPartyRoom)
    {
        this->CurrentPartyRoom->OnRoomMemberJoined().Remove(this->OnRoomMemberJoinedHandle);
        this->CurrentPartyRoom->OnRoomMemberUpdated().Remove(this->OnRoomMemberUpdatedHandle);
        this->CurrentPartyRoom->OnRoomMemberPromoted().Remove(this->OnRoomMemberPromotedHandle);
        this->CurrentPartyRoom->OnRoomMemberLeft().Remove(this->OnRoomMemberLeftHandle);

        this->CurrentPartyRoom.Reset();

        this->FirePartyMemberChanged();
    }
}

void ARedpointPartyMember::OnRoomMemberJoined(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser)
{
    // @todo: Fire only if the party member is for this slot.
    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::OnRoomMemberUpdated(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
    const ::Redpoint::EOS::Rooms::FRoomMemberUpdateInfo &RoomMemberUpdateInfo)
{
    // @todo: Fire only if the party member is for this slot.
    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::OnRoomMemberPromoted(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser)
{
    // @todo: Fire only if the party member is for this slot.
    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::OnRoomMemberLeft(
    const ::Redpoint::EOS::Rooms::FRoomEventContext &EventContext,
    const TSharedRef<::Redpoint::EOS::Rooms::IRoom> &Room,
    ::Redpoint::EOS::Rooms::FRoomUserId TargetUser,
    ::Redpoint::EOS::Rooms::ERoomLeaveReason Reason)
{
    // @todo: Fire only if the party member is for this slot.
    this->FirePartyMemberChanged();
}

void ARedpointPartyMember::FirePartyMemberChanged()
{
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Identity;
    using namespace ::Redpoint::EOS::Rooms;
    using namespace ::Redpoint::EOS::UserCache;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::Core::Utils;

    auto PlatformHandle = FWorldResolution::TryGetPlatformHandle(this->GetWorld());
    if (!PlatformHandle.IsValid())
    {
        FRedpointPartyMemberInfo MemberInfo;
        MemberInfo.bHasPartyMember = false;
        this->PartyMemberChanged(MemberInfo);
        return;
    }

    auto IdentitySystem = PlatformHandle->GetSystem<IIdentitySystem>();
    auto UserCacheSystem = PlatformHandle->GetSystem<IUserCacheSystem>();

    if (!this->CurrentPartyRoom.IsValid())
    {
        FRedpointPartyMemberInfo MemberInfo;
        MemberInfo.bHasPartyMember = false;
        this->PartyMemberChanged(MemberInfo);
        return;
    }

    auto RoomMembers = this->CurrentPartyRoom->GetMembers();
    if (RoomMembers.Num() <= this->PartyMemberSlot)
    {
        FRedpointPartyMemberInfo MemberInfo;
        MemberInfo.bHasPartyMember = false;
        this->PartyMemberChanged(MemberInfo);
        return;
    }

    auto &RoomMember = RoomMembers[this->PartyMemberSlot];

    auto LocalUsers = IdentitySystem->GetUsers();
    if (LocalUsers.Num() == 0)
    {
        FRedpointPartyMemberInfo MemberInfo;
        MemberInfo.bHasPartyMember = false;
        this->PartyMemberChanged(MemberInfo);
        return;
    }

    bool bIsPartyLeader = this->CurrentPartyRoom->GetRoomOwner()->GetUserId() == RoomMember->GetUserId();

    auto LocalUser = LocalUsers[0];

    UserCacheSystem->GetUser(
        LocalUser->GetUserId(),
        RoomMember->GetUserId(),
        IUserCacheSystem::FOnGetUserComplete::CreateWeakLambda(
            this,
            [this, RoomMember, bIsPartyLeader](const FError &ErrorCode, const FUserInfoPtr &UserInfo) {
                FRedpointPartyMemberInfo MemberInfo;
                MemberInfo.bHasPartyMember = true;
                MemberInfo.UserId.SetUniqueNetId(GetUniqueNetIdRef(RoomMember->GetUserId()));
                if (UserInfo.IsValid())
                {
                    MemberInfo.PlayerName = UserInfo->GetAttributes().FindRef(TEXT("displayName"));
                    MemberInfo.PlayerUserAttributes = UserInfo->GetAttributes();
                }
                for (const auto &Attribute : RoomMember->GetAttributes())
                {
                    FRedpointPartyAttributeValue AttributeValue;
                    switch (Attribute.Value.Type())
                    {
                    case FRoomAttribute::EType::String:
                        AttributeValue.Type = ERedpointPartyAttributeType::String;
                        AttributeValue.String = Attribute.Value.GetString();
                        break;
                    case FRoomAttribute::EType::Int64:
                        AttributeValue.Type = ERedpointPartyAttributeType::Int64;
                        AttributeValue.Int64 = Attribute.Value.GetInt64();
                        break;
                    case FRoomAttribute::EType::Double:
                        AttributeValue.Type = ERedpointPartyAttributeType::Double;
                        AttributeValue.Double = Attribute.Value.GetDouble();
                        break;
                    case FRoomAttribute::EType::Bool:
                        AttributeValue.Type = ERedpointPartyAttributeType::Bool;
                        AttributeValue.Bool = Attribute.Value.GetBool();
                        break;
                    default:
                        // @note: Unknown type?
                        AttributeValue.Type = ERedpointPartyAttributeType::Bool;
                        AttributeValue.Bool = false;
                        break;
                    }
                    MemberInfo.PartyMemberAttributes.Add(Attribute.Key, AttributeValue);
                }
                MemberInfo.bIsPartyLeader = bIsPartyLeader;
                this->PartyMemberChanged(MemberInfo);
            }));
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()