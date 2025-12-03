// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4224030567, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FInvitesRoomFeatureRequest::FInvitesRoomFeatureRequest(bool bRejoinAfterKickRequiresInvite)
    : bRejoinAfterKickRequiresInviteValue(bRejoinAfterKickRequiresInvite)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FInvitesRoomFeatureRequest::RejoinAfterKickRequiresInvite() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bRejoinAfterKickRequiresInviteValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()