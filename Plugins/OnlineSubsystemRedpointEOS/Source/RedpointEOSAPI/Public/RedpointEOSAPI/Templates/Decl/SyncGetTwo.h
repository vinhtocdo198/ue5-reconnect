// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/SyncGetTwoInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_BEGIN(CallNamespace, CallName, CallVersion)                              \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncGetTwoInfo<                                              \
        decltype(EOS_##CallNamespace##_##CallName)>::OptionsType NativeOptions;                                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncGetTwoInfo<                                              \
        decltype(EOS_##CallNamespace##_##CallName)>::Result1Type NativeResult1;                                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TSyncGetTwoInfo<                                              \
        decltype(EOS_##CallNamespace##_##CallName)>::Result2Type NativeResult2;                                        \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE EOS_EResult                                                                                     \
    NativeFunction(NativeHandle Handle, const NativeOptions *Options, NativeResult1 *Result1, NativeResult2 *Result2)  \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CallName(Handle, Options, Result1, Result2);                                    \
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

#define REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_END()                                                                    \
public:                                                                                                                \
    static EOS_EResult Execute(                                                                                        \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        Result1 &OutResult1,                                                                                           \
        Result2 &OutResult2);                                                                                          \
    static EOS_EResult Execute(                                                                                        \
        NativeHandle InHandle,                                                                                         \
        const Options &InOptions,                                                                                      \
        Result1 &OutResult1,                                                                                           \
        Result2 &OutResult2);                                                                                          \
                                                                                                                       \
private:                                                                                                               \
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);          \
    static void MapResult1(Result1 &Result1, const NativeResult1 &NativeResult1, NativeConverter &Converter);          \
    static void MapResult2(Result2 &Result2, const NativeResult2 &NativeResult2, NativeConverter &Converter);

REDPOINT_EOS_CODE_GUARD_END()