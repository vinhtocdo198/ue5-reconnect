// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/ListCountInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncListGetByIndexInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_BEGIN(                                                                      \
    CallNamespace,                                                                                                     \
    CountCallName,                                                                                                     \
    CountCallVersion,                                                                                                  \
    GetByIndexCallName,                                                                                                \
    GetByIndexCallVersion)                                                                                             \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountInfo<                                               \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountType NativeCount;                                       \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountInfo<                                               \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountOptionsType NativeCountOptions;                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncListGetByIndexInfo<                                      \
        decltype(EOS_##CallNamespace##_##GetByIndexCallName)>::GetOptionsType NativeGetByIndexOptions;                 \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncListGetByIndexInfo<                                      \
        decltype(EOS_##CallNamespace##_##GetByIndexCallName)>::ResultEntryType NativeGetByIndexResult;                 \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    static FORCEINLINE int NativeCountOptionsVersion()                                                                 \
    {                                                                                                                  \
        return CountCallVersion;                                                                                       \
    }                                                                                                                  \
    static FORCEINLINE int NativeGetByIndexOptionsVersion()                                                            \
    {                                                                                                                  \
        return GetByIndexCallVersion;                                                                                  \
    }                                                                                                                  \
    static FORCEINLINE NativeCount NativeCountFunction(NativeHandle Handle, const NativeCountOptions *Options)         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle, Options);                                                 \
    }                                                                                                                  \
    static FORCEINLINE NativeGetByIndexResult NativeGetByIndexFunction(                                                \
        NativeHandle Handle,                                                                                           \
        const NativeGetByIndexOptions *Options)                                                                        \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##GetByIndexCallName(Handle, Options);                                            \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCountName()                                                                \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(GetByIndexCallVersion)) TEXT("/Count");                                        \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallGetByIndexName()                                                           \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(GetByIndexCallVersion)) TEXT("/GetByIndex");                                   \
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

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_END()                                                                       \
private:                                                                                                               \
public:                                                                                                                \
    static void Execute(                                                                                               \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode,                                                                                    \
        TArray<ResultEntry> &OutResultEntries);                                                                        \
                                                                                                                       \
private:                                                                                                               \
    static void MapCountOptions(                                                                                       \
        NativeCountOptions &NativeOptions,                                                                             \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator);                                                                                   \
    static void MapGetByIndexOptions(                                                                                  \
        NativeGetByIndexOptions &NativeOptions,                                                                        \
        const Options &Options,                                                                                        \
        NativeCount Index,                                                                                             \
        NativeAllocator &Allocator);                                                                                   \
    static ResultEntry MapGetByIndexResult(const NativeGetByIndexResult &NativeResult);

REDPOINT_EOS_CODE_GUARD_END()