// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentityProcessGlobalUserRegistry.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1455739490, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

int32 FIdentityProcessGlobalUserRegistry::NextHandle = 8000;
TMap<FIdentityProcessGlobalUserHandle, TTuple<TSharedPtr<IIdentitySystem>, TWeakPtr<FIdentityUser>>>
    FIdentityProcessGlobalUserRegistry::Users;

FIdentityProcessGlobalUserHandle FIdentityProcessGlobalUserRegistry::Register(
    TSharedRef<IIdentitySystem> IdentitySystem,
    TSharedRef<FIdentityUser> User)
{
    auto NewId = FIdentityProcessGlobalUserHandle(
        FPlatformUserId::CreateFromInternalId(FIdentityProcessGlobalUserRegistry::NextHandle++));
    FIdentityProcessGlobalUserRegistry::Users.Add(
        NewId,
        TTuple<TSharedPtr<IIdentitySystem>, TWeakPtr<FIdentityUser>>(IdentitySystem, User));
    return NewId;
}

void FIdentityProcessGlobalUserRegistry::Unregister(FIdentityProcessGlobalUserHandle Handle)
{
    FIdentityProcessGlobalUserRegistry::Users.Remove(Handle);
}

TTuple<TSharedPtr<IIdentitySystem>, TWeakPtr<FIdentityUser>> FIdentityProcessGlobalUserRegistry::Get(
    FIdentityProcessGlobalUserHandle Handle)
{
    return FIdentityProcessGlobalUserRegistry::Users.FindRef(Handle);
}

}

REDPOINT_EOS_CODE_GUARD_END()