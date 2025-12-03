// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Platform/Release.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Stats.h"
#if !(defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING)
#include "RedpointEOSAPI/Logging.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

void FRelease::Execute(const FPlatformHandle &InHandle)
{
    checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));
    
    DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);
    SCOPE_CYCLE_COUNTER(STAT_Call);

    NativeHandle Handle = InHandle->Handle();

    if (Handle == nullptr)
    {
        return;
    }

    NativeFunction(Handle);
}

}

REDPOINT_EOS_CODE_GUARD_END()