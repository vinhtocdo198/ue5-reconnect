// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentitySystem.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3314893565, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::API;

class REDPOINTEOSIDENTITY_API FIdentityHookContext
{
public:
    FIdentityHookContext(const TSharedRef<IIdentitySystem> &InIdentitySystem, const FPlatformHandle &InPlatformHandle);
    FIdentityHookContext(const FIdentityHookContext &) = default;
    FIdentityHookContext(FIdentityHookContext &&) = default;
    virtual ~FIdentityHookContext() = default;

    const TSharedRef<IIdentitySystem> IdentitySystem;
    const FPlatformHandle PlatformHandle;
    const FPlatformInstanceEnvironmentRef Environment;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(3314893565, Redpoint::EOS::Identity, FIdentityHookContext)
}

REDPOINT_EOS_CODE_GUARD_END()