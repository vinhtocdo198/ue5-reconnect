// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/World.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2601969302, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSCORE_API FWorldTravel
{
public:
    typedef TDelegate<void(const FError &ErrorCode)> FOnTravelToMapComplete;

    /**
     * Attempts to travel to the target map, invoking the callback when the new map has started or travel has failed.
     */
    static void TravelToMap(const TSoftObjectPtr<UWorld> &World, FURL URL, const FOnTravelToMapComplete &OnComplete);

    /**
     * Attempts to travel to the target map, invoking the callback when the new map has started or travel has failed.
     */
    static void TravelToMap(FWorldContext &WorldContext, FURL URL, const FOnTravelToMapComplete &OnComplete);
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(2601969302, Redpoint::EOS::Core::Utils, FWorldTravel)
}

REDPOINT_EOS_CODE_GUARD_END()