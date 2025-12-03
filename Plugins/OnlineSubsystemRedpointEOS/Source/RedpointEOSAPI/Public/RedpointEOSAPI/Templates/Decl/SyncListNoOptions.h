// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/ListCountNoOptionsInfo.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncListGetNoOptionsByIndexInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_BEGIN(CallNamespace, CountCallName, GetByIndexCallName)          \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TListCountNoOptionsInfo<                                      \
        decltype(EOS_##CallNamespace##_##CountCallName)>::CountType NativeCount;                                       \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncListGetNoOptionsByIndexInfo<                             \
        decltype(EOS_##CallNamespace##_##GetByIndexCallName)>::ResultEntryType NativeGetByIndexResult;                 \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    static FORCEINLINE NativeCount NativeCountFunction(NativeHandle Handle)                                            \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CountCallName(Handle);                                                          \
    }                                                                                                                  \
    static FORCEINLINE NativeGetByIndexResult NativeGetByIndexFunction(NativeHandle Handle, NativeCount Index)         \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##GetByIndexCallName(Handle, Index);                                              \
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

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_END()                                                            \
private:                                                                                                               \
public:                                                                                                                \
    static void Execute(                                                                                               \
        const FPlatformHandle &InHandle,                                                                               \
        EOS_EResult &OutResultCode,                                                                                    \
        TArray<ResultEntry> &OutResultEntries);                                                                        \
                                                                                                                       \
private:                                                                                                               \
    static ResultEntry MapGetByIndexResult(const NativeGetByIndexResult &NativeResult);

REDPOINT_EOS_CODE_GUARD_END()