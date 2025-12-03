// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/MatchStateRoomFeatureImpl.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/MatchStateRoomFeatureRequest.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3686280070, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

void FMatchStateRoomFeatureImpl::StartMatchInRoom(
    const FStartMatchInRoomRequest &Request,
    const FOnStartMatchInRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::NotImplemented());
}

void FMatchStateRoomFeatureImpl::EndMatchInRoom(
    const FEndMatchInRoomRequest &Request,
    const FOnEndMatchInRoomComplete &OnComplete)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    OnComplete.ExecuteIfBound(Redpoint::EOS::API::Errors::NotImplemented());
}

TSharedPtr<IRoomFeatureRequestBase> FMatchStateRoomFeatureImpl::GetFeatureRequestForUpdate() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    // @todo: Room provider needs to set "allow join in progress value".
    return MakeShared<FMatchStateRoomFeatureRequest>(false);
}

}

REDPOINT_EOS_CODE_GUARD_END()