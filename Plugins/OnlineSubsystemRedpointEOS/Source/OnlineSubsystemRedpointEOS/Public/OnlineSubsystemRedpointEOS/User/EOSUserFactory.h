// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemRedpointEOS/User/EOSUser.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Id/UserIdMap.h"
#include "RedpointEOSPlatform/RuntimePlatformRegistry.h"
#include "RedpointEOSUserCache/UserInfo.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

class ONLINESUBSYSTEMREDPOINTEOS_API FEOSUserFactory : public TSharedFromThis<FEOSUserFactory>
{
public:
    FEOSUserFactory(
        FName InInstanceName,
        const Redpoint::EOS::API::FPlatformHandle &InPlatformHandle,
        const TSharedRef<Redpoint::EOS::Platform::FRuntimePlatformRegistry> &InRuntimePlatformRegistry);
    UE_NONCOPYABLE(FEOSUserFactory);
    virtual ~FEOSUserFactory() = default;

    const FName InstanceName;
    const Redpoint::EOS::API::FPlatformHandle PlatformHandle;
};

REDPOINT_EOS_CODE_GUARD_END()