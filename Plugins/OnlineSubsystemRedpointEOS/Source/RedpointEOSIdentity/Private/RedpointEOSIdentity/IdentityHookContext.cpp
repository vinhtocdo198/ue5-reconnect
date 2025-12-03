// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityHookContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3314893565, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

FIdentityHookContext::FIdentityHookContext(
    const TSharedRef<IIdentitySystem> &InIdentitySystem,
    const FPlatformHandle &InPlatformHandle)
    : IdentitySystem(InIdentitySystem)
    , PlatformHandle(InPlatformHandle)
    , Environment(InPlatformHandle->GetEnvironment())
{
}

}

REDPOINT_EOS_CODE_GUARD_END()