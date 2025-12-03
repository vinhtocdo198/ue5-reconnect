// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/UpdateRoomRequest.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2218672566, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

const TSet<FName> FUpdateRoomRequest::GetDeleteFeatureNames() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    return this->DeleteFeatureNames;
}

void FUpdateRoomRequest::DeleteFeature(FName FeatureName)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    this->DeleteFeatureNames.Add(FeatureName);
    this->FeatureRequests.Remove(FeatureName);
}

void FUpdateRoomRequest::RevertFeature(FName FeatureName)
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    this->FeatureRequests.Remove(FeatureName);
}

}

REDPOINT_EOS_CODE_GUARD_END()