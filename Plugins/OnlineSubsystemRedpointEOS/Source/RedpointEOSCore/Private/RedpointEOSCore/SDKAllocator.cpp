// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/SDKAllocator.h"

#include "RedpointEOSAPI/Memory.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1412197862, Redpoint::EOS::Core)
{

void *FSDKAllocator::Allocate(size_t SizeInBytes, size_t Alignment)
{
    REDPOINT_EOS_USE_LLM_TAG(SDK);

    return FMemory::Malloc(SizeInBytes, static_cast<uint32>(Alignment));
}

void *FSDKAllocator::Reallocate(void *Pointer, size_t SizeInBytes, size_t Alignment)
{
    REDPOINT_EOS_USE_LLM_TAG(SDK);

    return FMemory::Realloc(Pointer, SizeInBytes, static_cast<uint32>(Alignment));
}

void FSDKAllocator::Release(void *Pointer)
{
    REDPOINT_EOS_USE_LLM_TAG(SDK);

    FMemory::Free(Pointer);
}

}

REDPOINT_EOS_CODE_GUARD_END()