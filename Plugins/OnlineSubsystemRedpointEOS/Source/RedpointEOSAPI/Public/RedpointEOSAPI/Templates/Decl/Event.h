// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/EventHandle.h"
#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/EventRegisterInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSAPI/Templates/Util/Cdecl.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(CallNamespace, CallName, CallVersion)                                     \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TEventRegisterInfo<                                           \
        decltype(EOS_##CallNamespace##_AddNotify##CallName)>::RegisterOptionsType NativeOptions;                       \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TEventRegisterInfo<                                           \
        decltype(EOS_##CallNamespace##_AddNotify##CallName)>::ResultType NativeResult;                                 \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE EOS_NotificationId NativeRegisterFunction(                                                      \
        NativeHandle Handle,                                                                                           \
        const NativeOptions *Options,                                                                                  \
        void *ClientData,                                                                                              \
        void(__REDPOINT_EOSSDK_CDECL_ATTR * Callback)(const NativeResult *Data))                                       \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_AddNotify##CallName(Handle, Options, ClientData, Callback);                       \
    }                                                                                                                  \
    static FORCEINLINE void NativeUnregisterFunction(NativeHandle Handle, EOS_NotificationId Id)                       \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        EOS_##CallNamespace##_RemoveNotify##CallName(Handle, Id);                                                      \
    }                                                                                                                  \
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

#define REDPOINT_EOSSDK_API_CALL_EVENT_END()                                                                           \
    typedef TDelegate<void(const Result &InResult)> EventDelegate;                                                     \
                                                                                                                       \
private:                                                                                                               \
    class FHeapState : public INativeHeapStateCleanup                                                                  \
    {                                                                                                                  \
    public:                                                                                                            \
        EventDelegate _EventDelegate;                                                                                  \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeCallback(const NativeResult *Data);                           \
    static void HandleUnregisterCallback(FPlatformHandle InHandle, EOS_NotificationId InNotificationId);               \
                                                                                                                       \
public:                                                                                                                \
    [[nodiscard]] static FEventHandleRef Register(                                                                     \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const EventDelegate &InEventDelegate);                                                                         \
                                                                                                                       \
private:                                                                                                               \
    static void MapOptions(NativeOptions &NativeOptions, const Options &Options, NativeAllocator &Allocator);          \
    static void MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter);

REDPOINT_EOS_CODE_GUARD_END()