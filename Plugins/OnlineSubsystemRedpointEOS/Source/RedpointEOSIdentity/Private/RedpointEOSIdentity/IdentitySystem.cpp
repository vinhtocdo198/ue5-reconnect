// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSCore/Exec/ExecSystem.h"
#include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSIdentity/IdentitySystemDedicatedServerImpl.h"
#include "RedpointEOSIdentity/IdentitySystemGameImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(526824123, Redpoint::EOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Core::Exec;
using namespace ::Redpoint::EOS::Core::Utils;

FName IIdentitySystem::GetSystemName()
{
    return FName(TEXT("IIdentitySystem"));
}

TSharedRef<IIdentitySystem> IIdentitySystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    if (InPlatformInstanceEnvironment->bIsDedicatedServer)
    {
#endif
        auto IdentitySystem =
            MakeSharedWithEvents<FIdentitySystemDedicatedServerImpl>(InPlatformHandle, InPlatformInstanceEnvironment);
        return IdentitySystem;
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    }
    else
    {
        auto IdentitySystem =
            MakeSharedWithEvents<FIdentitySystemGameImpl>(InPlatformHandle, InPlatformInstanceEnvironment);
        InPlatformHandle->GetSystem<IExecSystem>()->Register(TEXT("Identity"), IdentitySystem);
        return IdentitySystem;
    }
#endif
}

}

REDPOINT_EOS_CODE_GUARD_END()