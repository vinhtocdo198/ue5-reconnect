// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyRedpointEOS.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Party/OnlinePartyIdRedpointEOS.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"
#include "RedpointEOSRooms/Features/OwnerMigrationRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"
#include "RedpointEOSRooms/Features/PartyTypeRoomFeature.h"
#include "RedpointEOSRooms/Features/PresenceRoomFeature.h"
#include "RedpointEOSRooms/Features/PublicAdvertisementRoomFeature.h"
#include "RedpointEOSRooms/Features/VoiceChatRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3473859598, Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party)
{
using namespace ::Redpoint::EOS::Rooms::Features;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Party;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS;

void FOnlinePartyRedpointEOS::SyncRoomState()
{
    this->LeaderId = GetUniqueNetId(this->Room->GetRoomOwner()->GetUserId());
    auto VoiceChatRoomFeature = this->Room->GetRoomFeature<IVoiceChatRoomFeature>();
    if (VoiceChatRoomFeature.IsValid())
    {
        this->RoomId = VoiceChatRoomFeature->GetRTCRoomName();
    }
    else
    {
        this->RoomId = TEXT("");
    }
}

FOnlinePartyRedpointEOS::FOnlinePartyRedpointEOS(const TSharedRef<IRoomSystem> &InRoomSystem, const FRoomRef &InRoom)
    : FOnlineParty(
          MakeShared<FOnlinePartyIdRedpointEOS>(InRoom->GetRoomId()),
          FOnlinePartyTypeId(InRoom->GetRoomFeatureChecked<IPartyTypeRoomFeature>()->GetPartyTypeId()))
    , RoomSystem(InRoomSystem)
    , Room(InRoom)
    , RoomMemberPromotedHandle()
    , RoomUpdatedHandle()
{
    this->State = EPartyState::Active;
    this->PreviousState = EPartyState::Active;
    this->SyncRoomState();
}

void FOnlinePartyRedpointEOS::RegisterEvents()
{
    this->RoomMemberPromotedHandle =
        this->Room->OnRoomMemberPromoted().Add(IRoom::FOnRoomMemberPromoted::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomRef RoomParam, FRoomUserId MemberId) {
                this->SyncRoomState();
            }));
    this->RoomUpdatedHandle = this->RoomSystem->OnRoomUpdated(NAME_Game).Add(
        IRoomSystem::FOnRoomUpdated::FDelegate::CreateSPLambda(
            this,
            [this](FRoomEventContext RoomEventContext, FRoomRef RoomParam, const FRoomUpdateInfo &RoomUpdateInfo) {
                if (RoomParam->GetRoomId()->IsEqual(this->Room->GetRoomId()))
                {
                    this->SyncRoomState();
                }
            }));
}

FOnlinePartyRedpointEOS::~FOnlinePartyRedpointEOS()
{
    this->Room->OnRoomMemberPromoted().Remove(this->RoomMemberPromotedHandle);
    this->RoomSystem->OnRoomUpdated(NAME_Game).Remove(this->RoomUpdatedHandle);
}

bool FOnlinePartyRedpointEOS::CanLocalUserInvite(const FUniqueNetId &LocalUserId) const
{
    return this->Room->HasRoomFeature<IInvitesRoomFeature>();
}

bool FOnlinePartyRedpointEOS::IsJoinable() const
{
    if (this->Room->HasRoomFeature<IInvitesRoomFeature>() ||
        this->Room->HasRoomFeature<IPublicAdvertisementRoomFeature>())
    {
        return true;
    }

    auto PresenceRoomFeature = this->Room->GetRoomFeature<IPresenceRoomFeature>();
    if (PresenceRoomFeature.IsValid() && PresenceRoomFeature->AllowJoinFromPresence())
    {
        return true;
    }

    return false;
}

void FOnlinePartyRedpointEOS::SetState(EPartyState InState)
{
    UE_LOG(LogOnlineSubsystemRedpointEOS, Warning, TEXT("%s is not supported."), ANSI_TO_TCHAR(__FUNCTION__));
}

TSharedRef<const FPartyConfiguration> FOnlinePartyRedpointEOS::GetConfiguration() const
{
    auto PartyConfiguration = MakeShared<FPartyConfiguration>();
    auto PartyConfigurationFeature = this->Room->GetRoomFeature<Features::IPartyConfigurationRoomFeature>();
    if (PartyConfigurationFeature.IsValid())
    {
        switch (PartyConfigurationFeature->GetJoinRequestAction())
        {
        case EPartyConfigurationRoomFeatureJoinRequestAction::Manual:
            PartyConfiguration->JoinRequestAction = EJoinRequestAction::Manual;
            break;
        case EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove:
            PartyConfiguration->JoinRequestAction = EJoinRequestAction::AutoApprove;
            break;
        case EPartyConfigurationRoomFeatureJoinRequestAction::AutoReject:
            PartyConfiguration->JoinRequestAction = EJoinRequestAction::AutoReject;
            break;
        default:
            checkf(false, TEXT("Value of GetJoinRequestAction is not supported."));
            break;
        }
        switch (PartyConfigurationFeature->GetJoinByPresencePermissions())
        {
        case EPartyConfigurationRoomFeaturePermissionType::Noone:
            PartyConfiguration->PresencePermissions = PartySystemPermissions::EPermissionType::Noone;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Leader:
            PartyConfiguration->PresencePermissions = PartySystemPermissions::EPermissionType::Leader;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Friends:
            PartyConfiguration->PresencePermissions = PartySystemPermissions::EPermissionType::Friends;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Anyone:
            PartyConfiguration->PresencePermissions = PartySystemPermissions::EPermissionType::Anyone;
            break;
        default:
            checkf(false, TEXT("Value of GetJoinByPresencePermissions is not supported."));
            break;
        }
        switch (PartyConfigurationFeature->GetSendInvitePermissions())
        {
        case EPartyConfigurationRoomFeaturePermissionType::Noone:
            PartyConfiguration->InvitePermissions = PartySystemPermissions::EPermissionType::Noone;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Leader:
            PartyConfiguration->InvitePermissions = PartySystemPermissions::EPermissionType::Leader;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Friends:
            PartyConfiguration->InvitePermissions = PartySystemPermissions::EPermissionType::Friends;
            break;
        case EPartyConfigurationRoomFeaturePermissionType::Anyone:
            PartyConfiguration->InvitePermissions = PartySystemPermissions::EPermissionType::Anyone;
            break;
        default:
            checkf(false, TEXT("Value of GetSendInvitePermissions is not supported."));
            break;
        }
        PartyConfiguration->Nickname = PartyConfigurationFeature->GetNickname();
        PartyConfiguration->Description = PartyConfigurationFeature->GetDescription();
        PartyConfiguration->Password = PartyConfigurationFeature->GetPassword();
    }
    else
    {
        UE_LOG(
            LogOnlineSubsystemRedpointEOS,
            Error,
            TEXT("%s: Room is missing PartyConfiguration room feature!"),
            ANSI_TO_TCHAR(__FUNCTION__));
    }
    PartyConfiguration->bChatEnabled = this->Room->HasRoomFeature<IVoiceChatRoomFeature>();
    PartyConfiguration->bShouldRemoveOnDisconnection = !this->Room->HasRoomFeature<IOwnerMigrationRoomFeature>();
    PartyConfiguration->bIsAcceptingMembers = this->IsJoinable();
    PartyConfiguration->NotAcceptingMembersReason = 0;
    PartyConfiguration->MaxMembers = static_cast<int32>(this->Room->GetRoomMaxMembers());
    return PartyConfiguration;
}

FOnlinePartyDataConstPtr FOnlinePartyRedpointEOS::GetPartyData(const FName &InNamespace) const
{
    auto Data = MakeShared<FOnlinePartyData>();
    for (const auto &AttributeKV : this->Room->GetRoomAttributes())
    {
        if (AttributeKV.Key == TEXT("RedpointOSSv1EventForcingAttribute"))
        {
            continue;
        }

        FVariantData AttributeData;
        switch (AttributeKV.Value.Type())
        {
        case FRoomAttribute::EType::String:
            AttributeData = FVariantData(AttributeKV.Value.GetString());
            break;
        case FRoomAttribute::EType::Int64:
            AttributeData = FVariantData(AttributeKV.Value.GetInt64());
            break;
        case FRoomAttribute::EType::Double:
            AttributeData = FVariantData(AttributeKV.Value.GetDouble());
            break;
        case FRoomAttribute::EType::Bool:
            AttributeData = FVariantData(AttributeKV.Value.GetBool());
            break;
        default:
            checkf(false, TEXT("Unhandled FRoomAttribute::EType value!"));
            break;
        }
        Data->SetAttribute(AttributeKV.Key, AttributeData);
    }
    Data->ClearDirty();
    return Data;
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()