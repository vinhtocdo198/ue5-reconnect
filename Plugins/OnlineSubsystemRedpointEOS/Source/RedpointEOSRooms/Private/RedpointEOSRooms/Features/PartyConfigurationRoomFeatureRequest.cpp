// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1899928368, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPartyConfigurationRoomFeatureRequest::FPartyConfigurationRoomFeatureRequest(
    EPartyConfigurationRoomFeatureJoinRequestAction InJoinRequestAction,
    EPartyConfigurationRoomFeaturePermissionType InJoinByPresencePermissions,
    EPartyConfigurationRoomFeaturePermissionType InSendInvitePermissions,
    const FString &InNickname,
    const FString &InDescription,
    const FString &InPassword)
    : JoinRequestAction(InJoinRequestAction)
    , JoinByPresencePermissions(InJoinByPresencePermissions)
    , SendInvitePermissions(InSendInvitePermissions)
    , Nickname(InNickname)
    , Description(InDescription)
    , Password(InPassword)

{
}

FPartyConfigurationRoomFeatureRequest::FPartyConfigurationRoomFeatureRequest(const FString &InClientPassword)
    : JoinRequestAction(EPartyConfigurationRoomFeatureJoinRequestAction::AutoApprove)
    , JoinByPresencePermissions(EPartyConfigurationRoomFeaturePermissionType::Anyone)
    , SendInvitePermissions(EPartyConfigurationRoomFeaturePermissionType::Anyone)
    , Nickname()
    , Description()
    , Password(InClientPassword)

{
}

bool FPartyConfigurationRoomFeatureRequest::IsValid() const
{
    return true;
}

EPartyConfigurationRoomFeatureJoinRequestAction FPartyConfigurationRoomFeatureRequest::GetJoinRequestAction() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinRequestAction;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureRequest::GetJoinByPresencePermissions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinByPresencePermissions;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureRequest::GetSendInvitePermissions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->SendInvitePermissions;
}

const FString &FPartyConfigurationRoomFeatureRequest::GetNickname() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Nickname;
}

const FString &FPartyConfigurationRoomFeatureRequest::GetDescription() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Description;
}

const FString &FPartyConfigurationRoomFeatureRequest::GetPassword() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Password;
}

}

REDPOINT_EOS_CODE_GUARD_END()