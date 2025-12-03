// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Engine/World.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4062683160, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSCORE_API FWorldResolution
{
public:
    /**
     * Attempts to resolve a pointer to the current world based on an online subsystem instance name. These differ
     * slightly from world context names.
     */
    static TSoftObjectPtr<UWorld> GetWorld(const FName &InInstanceName, bool bAllowFailure = false);

    /**
     * Attempts to resolve the platform handle for the specified world from the instance pool, returning nullptr on
     * failure.
     */
    static TSharedPtr<FPlatformInstance> TryGetPlatformHandle(const UWorld *InWorld);

    /**
     * Attempts to resolve the platform handle for the specified world from the instance pool, asserting on failure.
     */
    static FPlatformHandle GetPlatformHandle(const UWorld *InWorld);
};

}

namespace Redpoint::EOS::Core::Utils
{
REDPOINT_EOS_FILE_NS_EXPORT(4062683160, Redpoint::EOS::Core::Utils, FWorldResolution)
}

REDPOINT_EOS_CODE_GUARD_END()