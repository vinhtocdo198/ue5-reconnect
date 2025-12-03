// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSStorage/StorageSystem.h"

// #include "RedpointEOSCore/Utils/MakeSharedWithEvents.h"
#include "RedpointEOSStorage/StorageSystemImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(290799007, Redpoint::EOS::Storage)
{

FName IStorageSystem::GetSystemName()
{
    return FName(TEXT("IStorageSystem"));
}

TSharedRef<IStorageSystem> IStorageSystem::ConstructSystem(
    FPlatformHandle InPlatformHandle,
    FPlatformInstanceEnvironmentRef InPlatformInstanceEnvironment)
{
    return MakeShared<FStorageSystemImpl>(InPlatformHandle);
}

}

REDPOINT_EOS_CODE_GUARD_END()