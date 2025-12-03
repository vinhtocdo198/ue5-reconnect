// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSAPI/Templates/Util/Cdecl.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(CallNamespace, CallName, CallVersion)                                     \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##CallName)>::OptionsType NativeOptions;                                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##CallName)>::ResultType NativeResult;                                          \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE void NativeFunction(                                                                            \
        NativeHandle Handle,                                                                                           \
        const NativeOptions *Options,                                                                                  \
        void *ClientData,                                                                                              \
        void(__REDPOINT_EOSSDK_CDECL_ATTR * Callback)(const NativeResult *Data))                                       \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##CallNamespace##_##CallName(Handle, Options, ClientData, Callback);                                       \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallName()                                                                     \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName));                                                                    \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCompletionName()                                                           \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Callback");                                                  \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallName()                                                                      \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##CallName));                                         \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_ASYNC_END()                                                                           \
    typedef TDelegate<void(const Result &InResult)> CompletionDelegate;                                                \
                                                                                                                       \
private:                                                                                                               \
    class FHeapState                                                                                                   \
    {                                                                                                                  \
    public:                                                                                                            \
        NativeOptions _NativeOptions = {};                                                                             \
        CompletionDelegate _CompletionDelegate;                                                                        \
        TSharedPtr<NativeAllocator> _Allocator;                                                                        \
        bool _bSupportsAutomaticRetry;                                                                                 \
        TFunction<void(bool bTooManyRequests)> _ExecuteRetry;                                                          \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeCallback(const NativeResult *Data);                           \
                                                                                                                       \
public:                                                                                                                \
    static void Execute(                                                                                               \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate);                                                               \
                                                                                                                       \
private:                                                                                                               \
    static void ExecuteWithCurrentAttemptCount(                                                                        \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate,                                                                \
        int CurrentAttemptCount);                                                                                      \
                                                                                                                       \
public:                                                                                                                \
    static void Execute(                                                                                               \
        NativeHandle InHandle,                                                                                         \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate);                                                               \
                                                                                                                       \
private:                                                                                                               \
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);          \
    static void MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter);

REDPOINT_EOS_CODE_GUARD_END()