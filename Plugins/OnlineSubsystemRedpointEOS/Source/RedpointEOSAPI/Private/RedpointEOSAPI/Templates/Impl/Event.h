// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

#include "RedpointEOSAPI/CallbackValidator.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_EVENT_IMPL(CallName)                                                                  \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void __REDPOINT_EOSSDK_CDECL_ATTR F##CallName::HandleNativeCallback(const NativeResult *Data)                      \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCallbackName(), STAT_Call_Callback, STATGROUP_RedpointEOS);                         \
        SCOPE_CYCLE_COUNTER(STAT_Call_Callback);                                                                       \
                                                                                                                       \
        if (!FCallbackValidator::VerifyEvent(Data->ClientData))                                                        \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, Error, TEXT("[untracked event] %s"), LogCallName());                             \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[received] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Event received."), EOS_EResult::EOS_Success).ToLogString());            \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)Data->ClientData;                                                        \
        Result _Result;                                                                                                \
        MapResult(_Result, *Data, *MakeShared<NativeConverter>());                                                     \
        HeapState->_EventDelegate.ExecuteIfBound(_Result);                                                             \
    }                                                                                                                  \
    void F##CallName::HandleUnregisterCallback(FPlatformHandle InHandle, EOS_NotificationId InNotificationId)          \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        if (!InHandle->IsShutdown())                                                                                   \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[unregister] %s"),                                                                               \
                *ConvertError(LogCallName(), TEXT("Event will be unregistered."), EOS_EResult::EOS_Success)            \
                     .ToLogString());                                                                                  \
                                                                                                                       \
            NativeUnregisterFunction(InHandle->Get<NativeHandle>(), InNotificationId);                                 \
            FCallbackValidator::UntrackEvent(InNotificationId);                                                        \
                                                                                                                       \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[unregister] %s"),                                                                               \
                *ConvertError(LogCallName(), TEXT("Event has now been unregistered."), EOS_EResult::EOS_Success)       \
                     .ToLogString());                                                                                  \
        }                                                                                                              \
    }                                                                                                                  \
    FEventHandleRef F##CallName::Register(                                                                             \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const EventDelegate &InEventDelegate)                                                                          \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallRegisterName(), STAT_Call, STATGROUP_RedpointEOS);                                  \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());                                  \
                                                                                                                       \
        if (InHandle->IsShutdown())                                                                                    \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallName(),                                                                                    \
                     TEXT("The platform instance has been shutdown, so SDK calls can not be made."),                   \
                     EOS_EResult::EOS_NoConnection)                                                                    \
                     .ToLogString());                                                                                  \
            return MakeShared<FEventHandle>(InHandle);                                                                 \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator);                                                             \
                                                                                                                       \
        FHeapState *HeapState = new FHeapState();                                                                      \
        HeapState->_EventDelegate = InEventDelegate;                                                                   \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[register] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Event will be registered."), EOS_EResult::EOS_Success).ToLogString());  \
                                                                                                                       \
        EOS_NotificationId NotificationId =                                                                            \
            NativeRegisterFunction(InHandle->Get<NativeHandle>(), &_NativeOptions, HeapState, &HandleNativeCallback);  \
        FCallbackValidator::TrackEvent(HeapState, LogCallName(), NotificationId);                                      \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[register] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Event has been registered."), EOS_EResult::EOS_Success).ToLogString()); \
                                                                                                                       \
        return MakeShared<FEventHandle>(                                                                               \
            InHandle,                                                                                                  \
            NotificationId,                                                                                            \
            HeapState,                                                                                                 \
            TDelegate<void(FPlatformHandle, EOS_NotificationId)>::CreateStatic(&HandleUnregisterCallback));            \
    }

REDPOINT_EOS_CODE_GUARD_END()