// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Platform/Tick.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

void FTick::Execute(const FPlatformHandle &InHandle)
{
    checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));

    DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);
    SCOPE_CYCLE_COUNTER(STAT_Call);

    // @note: We do not emit VeryVerbose logs for the Tick operation because it is
    // called extremely frequently.

    NativeHandle Handle = InHandle->Handle();

    if (Handle == nullptr)
    {
        return;
    }

    NativeFunction(Handle);
}

}

REDPOINT_EOS_CODE_GUARD_END()