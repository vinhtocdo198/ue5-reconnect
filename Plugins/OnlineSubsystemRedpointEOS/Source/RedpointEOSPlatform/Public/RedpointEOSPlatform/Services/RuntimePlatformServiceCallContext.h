// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Online/CoreOnline.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2720465923, Redpoint::EOS::Platform::Services)
{
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSPLATFORM_API FRuntimePlatformServiceCallContext
{
public:
    FRuntimePlatformServiceCallContext(
        const FPlatformHandle &InPlatformHandle,
        int32 InLocalUserNum,
        const TSoftObjectPtr<UWorld> &InWorld);
    UE_NONCOPYABLE(FRuntimePlatformServiceCallContext);
    ~FRuntimePlatformServiceCallContext() = default;

    /**
     * The current EOS SDK platform handle.
     */
    FPlatformHandle PlatformHandle;

    /**
     * The local user that this call is being executed in the context of.
     */
    int32 LocalUserNum;

    /**
     * The current world, if known. This value may be nullptr.
     */
    TSoftObjectPtr<UWorld> World;

    typedef FPlatformHandle FPlatformKey;
    typedef TTuple<FPlatformHandle, int32> FUserKey;

    /**
     * Return the key that runtime platform integrations should use to store data per platform instance.
     */
    FORCEINLINE FPlatformKey GetPlatformKey() const
    {
        return this->PlatformHandle;
    }

    /**
     * Return the key that runtime platform integrations should use to store data per user, per platform instance.
     */
    FORCEINLINE FUserKey GetUserKey() const
    {
        return FUserKey(this->PlatformHandle, this->LocalUserNum);
    }
};

typedef TSharedRef<const FRuntimePlatformServiceCallContext> FRuntimePlatformServiceCallContextRef;
typedef TSharedPtr<const FRuntimePlatformServiceCallContext> FRuntimePlatformServiceCallContextPtr;

}

namespace Redpoint::EOS::Platform::Services
{
REDPOINT_EOS_FILE_NS_EXPORT(2720465923, Redpoint::EOS::Platform::Services, FRuntimePlatformServiceCallContext)
REDPOINT_EOS_FILE_NS_EXPORT(2720465923, Redpoint::EOS::Platform::Services, FRuntimePlatformServiceCallContextRef)
REDPOINT_EOS_FILE_NS_EXPORT(2720465923, Redpoint::EOS::Platform::Services, FRuntimePlatformServiceCallContextPtr)
}

REDPOINT_EOS_CODE_GUARD_END()