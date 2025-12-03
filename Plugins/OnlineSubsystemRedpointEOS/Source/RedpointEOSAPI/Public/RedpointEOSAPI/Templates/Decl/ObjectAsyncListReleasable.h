// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncListReleasableByIndexInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/ListCountInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSAPI/Templates/Util/Cdecl.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_BEGIN(                                                   \
    CallNamespace,                                                                                                     \
    QueryCallName,                                                                                                     \
    QueryCallVersion,                                                                                                  \
    CountCallName,                                                                                                     \
    CountCallVersion,                                                                                                  \
    CopyByIndexCallName,                                                                                               \
    CopyByIndexCallVersion,                                                                                            \
    ReleaseCallName)                                                                                                   \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::OptionsType NativeQueryOptions;                              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncInfo<                                                   \
        decltype(EOS_##CallNamespace##_##QueryCallName)>::ResultType NativeQueryResult;                                \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountInfo<                                               \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountOptionsType NativeCountOptions;                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListReleasableByIndexInfo<                              \
        decltype(EOS_##CallNamespace##_##CopyByIndexCallName)>::CopyOptionsType NativeCopyByIndexOptions;              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncListReleasableByIndexInfo<                              \
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
    static FORCEINLINE uint32_t NativeCountFunction(NativeHandle Handle, const NativeCountOptions *Options)            \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle, Options);                                                 \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult NativeCopyByIndexFunction(                                                          \
        NativeHandle Handle,                                                                                           \
        const NativeCopyByIndexOptions *Options,                                                                       \
        NativeCopyByIndexResult *Result)                                                                               \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CopyByIndexCallName(Handle, Options, Result);                                   \
    }                                                                                                                  \
    static FORCEINLINE void NativeReleaseFunction(NativeCopyByIndexResult Result)                                      \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##ReleaseCallName##_Release(Result);                                                                       \
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

#define REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_END()                                                    \
    typedef TDelegate<                                                                                                 \
        void(EOS_EResult ResultCode, const TArray<TReleasableRef<NativeCopyByIndexResult>> &ResultEntries)>            \
        CompletionDelegate;                                                                                            \
                                                                                                                       \
private:                                                                                                               \
    class FHeapState                                                                                                   \
    {                                                                                                                  \
    public:                                                                                                            \
        FHeapState(const TReleasableRef<NativeHandle> &InNativeHandle, const Options &InOptions)                       \
            : _NativeHandle(InNativeHandle)                                                                            \
            , _NativeQueryOptions()                                                                                    \
            , _Options(InOptions)                                                                                      \
            , _CompletionDelegate()                                                                                    \
            , _Allocator()                                                                                             \
        {                                                                                                              \
        }                                                                                                              \
        TReleasableRef<NativeHandle> _NativeHandle;                                                                    \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        Options _Options;                                                                                              \
        CompletionDelegate _CompletionDelegate;                                                                        \
        TSharedPtr<NativeAllocator> _Allocator;                                                                        \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeQueryCallback(const NativeQueryResult *QueryData);            \
                                                                                                                       \
public:                                                                                                                \
    static void Execute(                                                                                               \
        TReleasableRef<NativeHandle> InHandle,                                                                         \
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
        NativeAllocator &Allocator);

REDPOINT_EOS_CODE_GUARD_END()