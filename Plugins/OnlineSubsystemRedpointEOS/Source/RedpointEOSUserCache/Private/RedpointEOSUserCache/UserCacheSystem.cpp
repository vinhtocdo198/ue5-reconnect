// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSUserCache/UserCacheSystem.h"
#include "RedpointEOSUserCache/UserCacheSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1660508860, Redpoint::EOS::UserCache)
{

FName IUserCacheSystem::GetSystemName()
{
    return FName(TEXT("IUserCacheSystem"));
}

TSharedRef<IUserCacheSystem> IUserCacheSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FUserCacheSystemImpl>(InPlatformHandle, InPlatformInstanceEnvironment);
}

}

REDPOINT_EOS_CODE_GUARD_END()