// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSIdentity/IdentityProcessGlobalUserHandle.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSIdentity/IdentityUser.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

REDPOINT_EOS_FILE_NS_FORWARD_DECLARE_CLASS(2503918595, Redpoint::EOS::Identity, FIdentitySystemImpl)

namespace REDPOINT_EOS_FILE_NS_ID(1455739490, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

class REDPOINTEOSIDENTITY_API FIdentityProcessGlobalUserRegistry
{
    friend FIdentityUser;
    friend FIdentitySystemImpl;

private:
    static int32 NextHandle;
    static TMap<FIdentityProcessGlobalUserHandle, TTuple<TSharedPtr<IIdentitySystem>, TWeakPtr<FIdentityUser>>> Users;

    FIdentityProcessGlobalUserRegistry() = delete;

    static FIdentityProcessGlobalUserHandle Register(
        TSharedRef<IIdentitySystem> IdentitySystem,
        TSharedRef<FIdentityUser> User);
    static void Unregister(FIdentityProcessGlobalUserHandle Handle);

public:
    static TTuple<TSharedPtr<IIdentitySystem>, TWeakPtr<FIdentityUser>> Get(FIdentityProcessGlobalUserHandle Handle);
};

}

namespace Redpoint::EOS::Identity
{
REDPOINT_EOS_FILE_NS_EXPORT(1455739490, Redpoint::EOS::Identity, FIdentityProcessGlobalUserRegistry)
}

REDPOINT_EOS_CODE_GUARD_END()