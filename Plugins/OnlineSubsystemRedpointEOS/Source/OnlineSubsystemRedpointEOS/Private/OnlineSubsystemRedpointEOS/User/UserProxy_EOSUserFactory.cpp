// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/User/EOSUserFactory.h"

#include "RedpointEOSUserCache/UserCacheSystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

FEOSUserFactory::FEOSUserFactory(
    FName InInstanceName,
    const Redpoint::EOS::API::FPlatformHandle &InPlatformHandle,
    const TSharedRef<Redpoint::EOS::Platform::FRuntimePlatformRegistry> &InRuntimePlatformRegistry)
    : InstanceName(InInstanceName)
    , PlatformHandle(InPlatformHandle)
{
}

REDPOINT_EOS_CODE_GUARD_END()