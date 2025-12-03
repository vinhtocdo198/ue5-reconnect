// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/Utils/CallContextFactory.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(178046447, Redpoint::EOS::Core::Utils)
{
using namespace ::Redpoint::EOS::Core::Utils;

FRuntimePlatformServiceCallContextRef ConstructRuntimePlatformServiceCallContext(
    const FPlatformHandle &PlatformHandle,
    int32 LocalUserNum)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto World = FWorldResolution::GetWorld(PlatformHandle->GetEnvironment()->InstanceName, true);
    return MakeShared<FRuntimePlatformServiceCallContext>(PlatformHandle, LocalUserNum, World);
}

}

REDPOINT_EOS_CODE_GUARD_END()