// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/Templates/Util/AllocationFlags.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util
{

class REDPOINTEOSAPI_API FApiCallNativeAllocator
{
private:
    TArray<void *> Buffers;

public:
    FApiCallNativeAllocator() = default;
    FApiCallNativeAllocator(const FApiCallNativeAllocator &) = delete;
    FApiCallNativeAllocator(FApiCallNativeAllocator &&) = delete;
    ~FApiCallNativeAllocator();
    const char *AsAnsi(const FString &InString, EAllocationFlags InFlags = EAllocationFlags::Default);
    const char *AsUtf8(const FString &InString, EAllocationFlags InFlags = EAllocationFlags::Default);
    template <typename T> T &Allocate()
    {
        REDPOINT_EOS_USE_LLM_TAG(API);

        static_assert(std::is_trivially_copyable<T>::value);
        void *Buffer = FMemory::MallocZeroed(sizeof(T));
        Buffers.Add(Buffer);
        return *((T *)Buffer);
    }
    template <typename T>
    const T *AllocateArray(
        const TArray<T> &InArray,
        uint32_t &OutCount,
        EAllocationFlags InFlags = EAllocationFlags::Default)
    {
        REDPOINT_EOS_USE_LLM_TAG(API);

        OutCount = {};
        static_assert(std::is_trivially_copyable<T>::value);
        if (InFlags == EAllocationFlags::ReturnNullptrIfEmpty && InArray.Num() == 0)
        {
            OutCount = 0;
            return nullptr;
        }
        void *Buffer = FMemory::MallocZeroed(sizeof(T) * (SIZE_T)InArray.Num());
        Buffers.Add(Buffer);
        OutCount = (uint32_t)InArray.Num();
        T *BufferCast = (T *)Buffer;
        for (typename TArray<T>::SizeType i = 0; i < InArray.Num(); i++)
        {
            BufferCast[(uint32_t)i] = InArray[i];
        }
        return BufferCast;
    }
};

}

REDPOINT_EOS_CODE_GUARD_END()