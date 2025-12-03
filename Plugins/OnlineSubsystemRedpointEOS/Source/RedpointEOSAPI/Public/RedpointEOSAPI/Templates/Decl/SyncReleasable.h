// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncReleasableAllocateInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_BEGIN(CallNamespace, CallName, CallVersion, ReleaseCallName)          \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncReleasableAllocateInfo<                                  \
        decltype(EOS_##CallNamespace##_##CallName)>::CopyOptionsType NativeOptions;                                    \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncReleasableAllocateInfo<                                  \
        decltype(EOS_##CallNamespace##_##CallName)>::ResultEntryType NativeResult;                                     \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult                                                                                     \
    NativeFunction(NativeHandle Handle, const NativeOptions *Options, NativeResult *Result)                            \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CallName(Handle, Options, Result);                                              \
    }                                                                                                                  \
    static FORCEINLINE void NativeReleaseFunction(NativeResult Result)                                                 \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##ReleaseCallName##_Release(Result);                                                                       \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallName()                                                                     \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName));                                                                    \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallName()                                                                      \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##CallName));                                         \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_END()                                                                 \
public:                                                                                                                \
    static TSharedRef<TReleasable<NativeResult>> Execute(                                                              \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode);                                                                                   \
                                                                                                                       \
private:                                                                                                               \
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);

REDPOINT_EOS_CODE_GUARD_END()