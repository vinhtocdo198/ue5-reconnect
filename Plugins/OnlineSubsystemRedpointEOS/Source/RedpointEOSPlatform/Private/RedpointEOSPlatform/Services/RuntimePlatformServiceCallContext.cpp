// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/Services/RuntimePlatformServiceCallContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2720465923, Redpoint::EOS::Platform::Services)
{

FRuntimePlatformServiceCallContext::FRuntimePlatformServiceCallContext(
    const FPlatformHandle &InPlatformHandle,
    int32 InLocalUserNum,
    const TSoftObjectPtr<UWorld> &InWorld)
    : PlatformHandle(InPlatformHandle)
    , LocalUserNum(InLocalUserNum)
    , World(InWorld)
{
}

}

REDPOINT_EOS_CODE_GUARD_END()