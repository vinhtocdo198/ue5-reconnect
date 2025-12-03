// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncListCopyByIndexInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncListCountErrorableInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_BEGIN(                                                \
    CallNamespace,                                                                                                     \
    QueryCallName,                                                                                                     \
    QueryCallVersion,                                                                                                  \
    CountCallName,                                                                                                     \
    CountCallVersion,                                                                                                  \
    CopyByIndexCallName,                                                                                               \
    CopyByIndexCallVersion,                                                                                            \
    ResultEntryName)                                                                                                   \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::OptionsType NativeQueryOptions;                              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::ResultType NativeQueryResult;                                \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListCountErrorableInfo<                                 \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountOptionsType NativeCountOptions;                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListCopyByIndexInfo<                                    \
        decltype(EOS_##CallNamespace##_##CopyByIndexCallName)>::CopyOptionsType NativeCopyByIndexOptions;              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListCopyByIndexInfo<                                    \
        decltype(EOS_##CallNamespace##_##CopyByIndexCallName)>::ResultEntryType NativeCopyByIndexResult;               \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeQueryOptionsVersion()                                                                 \
    {                                                                                                                  \
        return QueryCallVersion;                                                                                       \
    }                                                                                                                  \
    static FORCEINLINE int NativeCountOptionsVersion()                                                                 \
    {                                                                                                                  \
        return CountCallVersion;                                                                                       \
    }                                                                                                                  \
    static FORCEINLINE int NativeCopyByIndexOptionsVersion()                                                           \
    {                                                                                                                  \
        return CopyByIndexCallVersion;                                                                                 \
    }                                                                                                                  \
    static FORCEINLINE void NativeQueryFunction(                                                                       \
        NativeHandle Handle,                                                                                           \
        const NativeQueryOptions *Options,                                                                             \
        void *ClientData,                                                                                              \
        void(__REDPOINT_EOSSDK_CDECL_ATTR * Callback)(const NativeQueryResult *Data))                                  \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##CallNamespace##_##QueryCallName(Handle, Options, ClientData, Callback);                                  \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult                                                                                     \
    NativeCountFunction(NativeHandle Handle, const NativeCountOptions *Options, int32_t *OutCount)                     \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle, Options, OutCount);                                       \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult NativeCopyByIndexFunction(                                                          \
        NativeHandle Handle,                                                                                           \
        const NativeCopyByIndexOptions *Options,                                                                       \
        NativeCopyByIndexResult **Result)                                                                              \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CopyByIndexCallName(Handle, Options, Result);                                   \
    }                                                                                                                  \
    static FORCEINLINE void NativeReleaseFunction(NativeCopyByIndexResult *Result)                                     \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##CallNamespace##_##ResultEntryName##_Release(Result);                                                     \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallName()                                                                     \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(QueryCallName));                                                               \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCompletionName()                                                           \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(QueryCallName)) TEXT("/Callback");                                             \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCountName()                                                                \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(QueryCallName)) TEXT("/Count");                                                \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCopyByIndexName()                                                          \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(QueryCallName)) TEXT("/CopyByIndex");                                          \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallQueryName()                                                                 \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##QueryCallName));                                    \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallCountName()                                                                 \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##CountCallName));                                    \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallCopyByIndexName()                                                           \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##CopyByIndexCallName));                              \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_END()                                                 \
    typedef TDelegate<void(EOS_EResult ResultCode, const TArray<ResultEntry> &ResultEntries)> CompletionDelegate;      \
                                                                                                                       \
private:                                                                                                               \
    class FHeapState                                                                                                   \
    {                                                                                                                  \
    public:                                                                                                            \
        FHeapState(const Options &InOptions)                                                                           \
            : _NativeHandle()                                                                                          \
            , _NativeQueryOptions()                                                                                    \
            , _Options(InOptions)                                                                                      \
            , _CompletionDelegate()                                                                                    \
            , _Allocator()                                                                                             \
            , _bSupportsAutomaticRetry(false)                                                                          \
            , _ExecuteRetry()                                                                                          \
        {                                                                                                              \
        }                                                                                                              \
        NativeHandle _NativeHandle;                                                                                    \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        Options _Options;                                                                                              \
        CompletionDelegate _CompletionDelegate;                                                                        \
        TSharedPtr<NativeAllocator> _Allocator;                                                                        \
        bool _bSupportsAutomaticRetry;                                                                                 \
        TFunction<void(bool bTooManyRequests)> _ExecuteRetry;                                                          \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeQueryCallback(const NativeQueryResult *QueryData);            \
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
    static void MapQueryOptions(                                                                                       \
        NativeQueryOptions &NativeOptions,                                                                             \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator);                                                                                   \
    static void MapCountOptions(                                                                                       \
        NativeCountOptions &NativeOptions,                                                                             \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator);                                                                                   \
    static void MapCopyByIndexOptions(                                                                                 \
        NativeCopyByIndexOptions &NativeOptions,                                                                       \
        const Options &Options,                                                                                        \
        uint32_t Index,                                                                                                \
        NativeAllocator &Allocator);                                                                                   \
    static void MapCopyByIndexResult(                                                                                  \
        ResultEntry &Result,                                                                                           \
        const NativeCopyByIndexResult &NativeResult,                                                                   \
        NativeConverter &Converter);

REDPOINT_EOS_CODE_GUARD_END()