// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/ListCountInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncListCopyByIndexInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_BEGIN(                                                          \
    CallNamespace,                                                                                                     \
    CountCallName,                                                                                                     \
    CountCallVersion,                                                                                                  \
    CopyByIndexCallName,                                                                                               \
    CopyByIndexCallVersion,                                                                                            \
    ResultEntryNamespace,                                                                                              \
    ResultEntryName)                                                                                                   \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountInfo<                                               \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountOptionsType NativeCountOptions;                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncListCopyByIndexInfo<                                     \
        decltype(EOS_##CallNamespace##_##CopyByIndexCallName)>::CopyOptionsType NativeCopyByIndexOptions;              \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncListCopyByIndexInfo<                                     \
        decltype(EOS_##CallNamespace##_##CopyByIndexCallName)>::ResultEntryType NativeCopyByIndexResult;               \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeCountOptionsVersion()                                                                 \
    {                                                                                                                  \
        return CountCallVersion;                                                                                       \
    }                                                                                                                  \
    static FORCEINLINE int NativeCopyByIndexOptionsVersion()                                                           \
    {                                                                                                                  \
        return CopyByIndexCallVersion;                                                                                 \
    }                                                                                                                  \
    static FORCEINLINE uint32_t NativeCountFunction(NativeHandle Handle, const NativeCountOptions *Options)            \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle, Options);                                                 \
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
        EOS_##ResultEntryNamespace##_##ResultEntryName##_Release(Result);                                              \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCountName()                                                                \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CopyByIndexCallVersion)) TEXT("/Count");                                       \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCopyByIndexName()                                                          \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CopyByIndexCallVersion)) TEXT("/CopyByIndex");                                 \
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

#define REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_LIST_COPY_END()                                                           \
private:                                                                                                               \
public:                                                                                                                \
    static void Execute(                                                                                               \
        TReleasableRef<NativeHandle> InHandle,                                                                         \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode,                                                                                    \
        TArray<ResultEntry> &OutResultEntries);                                                                        \
                                                                                                                       \
private:                                                                                                               \
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