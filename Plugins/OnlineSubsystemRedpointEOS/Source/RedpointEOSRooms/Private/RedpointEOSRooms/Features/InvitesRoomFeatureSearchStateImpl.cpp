// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/InvitesRoomFeatureSearchStateImpl.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3535887801, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FInvitesRoomFeatureSearchStateImpl::FInvitesRoomFeatureSearchStateImpl()
    : bRejoinAfterKickRequiresInvite()
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FInvitesRoomFeatureSearchStateImpl::RejoinAfterKickRequiresInvite() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->bRejoinAfterKickRequiresInvite;
}

}

REDPOINT_EOS_CODE_GUARD_END()