// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/PublicMemberListRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2382703701, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FPublicMemberListRoomFeatureImpl::FPublicMemberListRoomFeatureImpl()
    : CurrentMembersValue()
    , MaxMembersValue()
{
}

FPublicMemberListRoomFeatureImpl::FPublicMemberListRoomFeatureImpl(
    const TArray<FRoomUserId> &InCurrentMembers,
    uint32 InMaxMembers)
    : CurrentMembersValue(InCurrentMembers)
    , MaxMembersValue(InMaxMembers)
{
}

TSharedPtr<IRoomFeatureRequestBase> FPublicMemberListRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
    return MakeShared<FPublicMemberListRoomFeatureRequest>();
}

}

REDPOINT_EOS_CODE_GUARD_END()