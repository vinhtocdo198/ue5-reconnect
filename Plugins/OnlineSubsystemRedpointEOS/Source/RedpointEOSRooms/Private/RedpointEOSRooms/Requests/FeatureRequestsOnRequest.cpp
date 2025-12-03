// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSRooms/Requests/FeatureRequestsOnRequest.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(212611124, Redpoint::EOS::Rooms::Requests)
{
using namespace ::Redpoint::EOS::Rooms::Requests;

TSharedPtr<Features::IRoomFeatureRequestBase> FFeatureRequestsOnRequest::GetFeatureRequestBase(
    FName InFeatureName) const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    if (this->FeatureRequests.Contains(InFeatureName))
    {
        return this->FeatureRequests[InFeatureName];
    }
    else
    {
        return nullptr;
    }
}

TSet<FName> FFeatureRequestsOnRequest::GetFeatureRequestNames() const
{
    REDPOINT_EOS_USE_LLM_TAG(Rooms_Requests);

    TSet<FName> Names;
    this->FeatureRequests.GetKeys(Names);
    return Names;
}

}

REDPOINT_EOS_CODE_GUARD_END()