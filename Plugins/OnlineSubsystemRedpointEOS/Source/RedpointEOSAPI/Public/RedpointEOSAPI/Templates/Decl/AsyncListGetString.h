// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncListGetStringByIndexInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/ListCountInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_BEGIN(                                                          \
    CallNamespace,                                                                                                     \
    QueryCallName,                                                                                                     \
    QueryCallVersion,                                                                                                  \
    CountCallName,                                                                                                     \
    CountCallVersion,                                                                                                  \
    GetByIndexCallName,                                                                                                \
    GetByIndexCallVersion,                                                                                             \
    StringMaxLength)                                                                                                   \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::OptionsType NativeQueryOptions;                              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::ResultType NativeQueryResult;                                \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountInfo<                                               \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountOptionsType NativeCountOptions;                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListGetStringByIndexInfo<                               \
        decltype(EOS_##CallNamespace##_##GetByIndexCallName)>::GetOptionsType NativeGetByIndexOptions;                 \
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
    static FORCEINLINE int NativeGetByIndexOptionsVersion()                                                            \
    {                                                                                                                  \
        return GetByIndexCallVersion;                                                                                  \
    }                                                                                                                  \
    static FORCEINLINE int32_t NativeGetStringMaxLength()                                                              \
    {                                                                                                                  \
        return StringMaxLength;                                                                                        \
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
    static FORCEINLINE uint32_t NativeCountFunction(NativeHandle Handle, const NativeCountOptions *Options)            \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle, Options);                                                 \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult NativeGetByIndexFunction(                                                           \
        NativeHandle Handle,                                                                                           \
        const NativeGetByIndexOptions *Options,                                                                        \
        char *OutBuffer,                                                                                               \
        int32_t *InOutBufferLength)                                                                                    \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##GetByIndexCallName(Handle, Options, OutBuffer, InOutBufferLength);              \
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
    static FORCEINLINE const TCHAR *LogCallGetByIndexName()                                                            \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##GetByIndexCallName));                               \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_END()                                                           \
    typedef TDelegate<void(EOS_EResult ResultCode, const TArray<FString> &ResultEntries)> CompletionDelegate;          \
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
            , _ExecuteRetry()                                                                                          \
        {                                                                                                              \
        }                                                                                                              \
        NativeHandle _NativeHandle;                                                                                    \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        Options _Options;                                                                                              \
        CompletionDelegate _CompletionDelegate;                                                                        \
        TSharedPtr<NativeAllocator> _Allocator;                                                                        \
        TFunction<void(bool bTooManyRequests)> _ExecuteRetry;                                                          \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeQueryCallback(const NativeQueryResult *QueryData);            \
                                                                                                                       \
public:                                                                                                                \
    static void Execute(                                                                                               \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate);                                                               \
                                                                                                                       \
private:                                                                                                               \
    static void ExecuteWithCurrentAttemptCount(                                                                        \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate,                                                                \
        int CurrentAttemptCount);                                                                                      \
                                                                                                                       \
    static void MapQueryOptions(                                                                                       \
        NativeQueryOptions &NativeOptions,                                                                             \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator);                                                                                   \
    static void MapCountOptions(                                                                                       \
        NativeCountOptions &NativeOptions,                                                                             \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator);                                                                                   \
    static void MapGetByIndexOptions(                                                                                  \
        NativeGetByIndexOptions &NativeOptions,                                                                        \
        const Options &Options,                                                                                        \
        uint32_t Index,                                                                                                \
        NativeAllocator &Allocator);

REDPOINT_EOS_CODE_GUARD_END()