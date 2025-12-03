// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3872218600, Redpoint::EOS::Networking)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSNETWORKING_API FNetDriverResolution
{
private:
    /**
     * Attempts to resolve the world context for the specified net driver, returning nullptr on failure.
     */
    static const FWorldContext *TryGetWorldContext(const UNetDriver *NetDriver);

public:
    /**
     * Attempts to resolve the platform handle for the specified net driver, returning nullptr on failure.
     */
    static TSharedPtr<FPlatformInstance> TryGetPlatformHandle(const UNetDriver *InNetDriver);

    /**
     * Attempts to resolve the platform handle for the specified net driver, asserting on failure.
     */
    static FPlatformHandle GetPlatformHandle(const UNetDriver *InNetDriver);
};

}

namespace Redpoint::EOS::Networking
{
REDPOINT_EOS_FILE_NS_EXPORT(3872218600, Redpoint::EOS::Networking, FNetDriverResolution)
}

REDPOINT_EOS_CODE_GUARD_END()