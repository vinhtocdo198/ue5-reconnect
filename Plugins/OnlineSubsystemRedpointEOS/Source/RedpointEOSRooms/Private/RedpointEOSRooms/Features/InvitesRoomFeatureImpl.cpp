// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/InvitesRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/InvitesRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1089720277, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

bool FInvitesRoomFeatureImpl::RejoinAfterKickRequiresInviteValue() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bRejoinAfterKickRequiresInviteValue;
}

TSharedPtr<IRoomFeatureRequestBase> FInvitesRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return MakeShared<FInvitesRoomFeatureRequest>(this->bRejoinAfterKickRequiresInviteValue);
}

}

REDPOINT_EOS_CODE_GUARD_END()