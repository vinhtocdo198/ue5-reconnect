// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_BEGIN(CallNamespace, CallName, CallVersion)                               \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncInfo<                                                    \
        decltype(EOS_##CallNamespace##_##CallName)>::OptionsType NativeOptions;                                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncInfo<                                                    \
        decltype(EOS_##CallNamespace##_##CallName)>::ResultType NativeResult;                                          \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE NativeResult NativeFunction(NativeHandle Handle, const NativeOptions *Options)                  \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CallName(Handle, Options);                                                      \
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

#define REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_END()                                                                     \
public:                                                                                                                \
    static Result Execute(const FPlatformHandle &InHandle, const Options &InOptions);                                  \
    static Result Execute(NativeHandle InHandle, const Options &InOptions);                                            \
                                                                                                                       \
private:                                                                                                               \
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);          \
    static Result MapResult(const NativeResult &NativeResult);

REDPOINT_EOS_CODE_GUARD_END()