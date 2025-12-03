// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_EVENT_STUB_BEGIN(CallNamespace, CallName)                                             \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE const TCHAR *StatCallRegisterName()                                                             \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Register");                                                  \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCallbackName()                                                             \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Callback");                                                  \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallUnregisterName()                                                           \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Unregister");                                                \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallName()                                                                      \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_AddNotify##CallName));                                \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_EVENT_STUB_END()                                                                      \
    typedef TDelegate<void(const Result &InResult)> EventDelegate;                                                     \
                                                                                                                       \
public:                                                                                                                \
    [[nodiscard]] static FEventHandleRef Register(                                                                     \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const EventDelegate &InEventDelegate);                                                                         \
                                                                                                                       \
private:

REDPOINT_EOS_CODE_GUARD_END()