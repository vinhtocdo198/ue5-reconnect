// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PartyConfigurationRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3356681519, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPartyConfigurationRoomFeatureSearchStateImpl::FPartyConfigurationRoomFeatureSearchStateImpl()
    : JoinRequestAction()
    , JoinByPresencePermissions()
    , SendInvitePermissions()
    , Nickname()
    , Description()
    , bIsPasswordRequired()
{
}

EPartyConfigurationRoomFeatureJoinRequestAction FPartyConfigurationRoomFeatureSearchStateImpl::GetJoinRequestAction()
    const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinRequestAction;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureSearchStateImpl::
    GetJoinByPresencePermissions() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->JoinByPresencePermissions;
}

EPartyConfigurationRoomFeaturePermissionType FPartyConfigurationRoomFeatureSearchStateImpl::GetSendInvitePermissions()
    const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->SendInvitePermissions;
}

FString FPartyConfigurationRoomFeatureSearchStateImpl::GetNickname() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Nickname;
}

FString FPartyConfigurationRoomFeatureSearchStateImpl::GetDescription() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->Description;
}

bool FPartyConfigurationRoomFeatureSearchStateImpl::IsPasswordRequired() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bIsPasswordRequired;
}

}

REDPOINT_EOS_CODE_GUARD_END()