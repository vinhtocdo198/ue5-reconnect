// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util
{

FApiCallNativeAllocator::~FApiCallNativeAllocator()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    for (const auto &Buffer : Buffers)
    {
        FMemory::Free((void *)Buffer);
    }
}

const char *FApiCallNativeAllocator::AsAnsi(const FString &InString, EAllocationFlags InFlags)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    if (InFlags == EAllocationFlags::ReturnNullptrIfEmpty && InString.IsEmpty())
    {
        return nullptr;
    }
    auto StrBuf = StringCast<ANSICHAR>(*InString);
    char *CharPtr = (char *)FMemory::MallocZeroed((SIZE_T)StrBuf.Length() + 1);
    FMemory::Memcpy((void *)CharPtr, (const void *)StrBuf.Get(), (SIZE_T)StrBuf.Length());
    Buffers.Add(CharPtr);
    return CharPtr;
}

const char *FApiCallNativeAllocator::AsUtf8(const FString &InString, EAllocationFlags InFlags)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    if (InFlags == EAllocationFlags::ReturnNullptrIfEmpty && InString.IsEmpty())
    {
        return nullptr;
    }
    auto StrBuf = FTCHARToUTF8(*InString);
    char *CharPtr = (char *)FMemory::MallocZeroed((SIZE_T)StrBuf.Length() + 1);
    FMemory::Memcpy((void *)CharPtr, (const void *)StrBuf.Get(), (SIZE_T)StrBuf.Length());
    Buffers.Add(CharPtr);
    return CharPtr;
}

}

REDPOINT_EOS_CODE_GUARD_END()