// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1412197862, Redpoint::EOS::Core)
{

class FSDKAllocator
{
public:
    static void *Allocate(size_t SizeInBytes, size_t Alignment);
    static void *Reallocate(void *Pointer, size_t SizeInBytes, size_t Alignment);
    static void Release(void *Pointer);
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(1412197862, Redpoint::EOS::Core, FSDKAllocator)
}

REDPOINT_EOS_CODE_GUARD_END()