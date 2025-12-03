// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Utils/GenericRegistry.h"
#include "RedpointEOSIdentity/IdentityHook.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2880485021, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Core::Utils;

class REDPOINTEOSIDENTITY_API FIdentityHookRegistry : public TGenericRegistry<IIdentityHook, FIdentityHookRegistry>
{
    REDPOINT_EOS_DECLARE_GENERIC_REGISTRY_BODY()

protected:
    virtual TSharedRef<IIdentityHook> MakeEntry(const TSharedRef<IIdentityHook> &InEntry) override;
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(2880485021, Redpoint::EOS::Identity, FIdentityHookRegistry)
}

REDPOINT_EOS_CODE_GUARD_END()