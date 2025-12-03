// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityHookRegistry.h"

#include "RedpointEOSIdentity/IdentityHookTracer.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2880485021, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

REDPOINT_EOS_DEFINE_GENERIC_REGISTRY(FIdentityHookRegistry);

TSharedRef<IIdentityHook> FIdentityHookRegistry::MakeEntry(const TSharedRef<IIdentityHook> &InEntry)
{
#if !NO_LOGGING
    return MakeShared<FIdentityHookTracer>(InEntry);
#else
    return InEntry;
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()