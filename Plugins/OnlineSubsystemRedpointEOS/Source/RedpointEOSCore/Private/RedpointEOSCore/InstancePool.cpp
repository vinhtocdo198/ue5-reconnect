// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/InstancePool.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSCore/InstancePoolImpl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(401947299, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS::Core;

static TSharedPtr<IInstancePool, ESPMode::ThreadSafe> GRedpointEOSPlatformInstance;
static FCriticalSection GRedpointEOSPlatformInstancePoolLock;
IInstancePool &IInstancePool::Pool()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    // Optimistic check outside the lock.
    if (GRedpointEOSPlatformInstance.IsValid())
    {
        return *GRedpointEOSPlatformInstance.Get();
    }

    {
        // @note: Ensures we don't race on the initialisation of the platform pool.
        FScopeLock Lock(&GRedpointEOSPlatformInstancePoolLock);
        if (GRedpointEOSPlatformInstance.IsValid())
        {
            return *GRedpointEOSPlatformInstance.Get();
        }
        GRedpointEOSPlatformInstance = MakeShared<FInstancePoolImpl>();
        return *GRedpointEOSPlatformInstance.Get();
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()