// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Features/AliasRoomFeatureRequest.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSRooms/Features/AliasRoomFeature.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(668254285, Redpoint::EOS::Rooms::Features)
{
using namespace ::Redpoint::EOS::Rooms::Features;

FAliasRoomFeatureRequest::FAliasRoomFeatureRequest(const FName &InAlias)
    : AliasValue(InAlias)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);
}

bool FAliasRoomFeatureRequest::IsValid() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return !this->AliasValue.ToString().IsEmpty() && !this->AliasValue.IsNone();
}

const FName &FAliasRoomFeatureRequest::Alias() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Features);

    return this->AliasValue;
}

}

REDPOINT_EOS_CODE_GUARD_END()