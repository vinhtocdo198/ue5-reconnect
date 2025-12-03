// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Async.h"

#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSAPI/CallbackValidator.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_WITH_IMMEDIATE_ERROR_IMPL(CallName)                                             \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                                    \
                                                                                                                       \
    void __REDPOINT_EOSSDK_CDECL_ATTR F##CallName::HandleNativeCallback(const NativeResult *Data)                      \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCompletionName(), STAT_Call_Completion, STATGROUP_RedpointEOS);                     \
        SCOPE_CYCLE_COUNTER(STAT_Call_Completion);                                                                     \
                                                                                                                       \
        if (!EOS_EResult_IsOperationComplete(Data->ResultCode))                                                        \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[pending ] %s"), LogCallName());                              \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        if (!FCallbackValidator::VerifyCallback(Data->ClientData, Data->ResultCode))                                   \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, Error, TEXT("[double call! ignoring] %s"), LogCallName());                       \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)Data->ClientData;                                                        \
                                                                                                                       \
        if (HeapState->_bSupportsAutomaticRetry && Data->ResultCode == EOS_EResult::EOS_AlreadyPending)                \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(LogCallName(), TEXT("Async call received EOS_AlreadyPending."), Data->ResultCode)        \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(false);                                                                           \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (HeapState->_bSupportsAutomaticRetry && Data->ResultCode == EOS_EResult::EOS_TooManyRequests)               \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(LogCallName(), TEXT("Async call received EOS_TooManyRequests."), Data->ResultCode)       \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(true);                                                                            \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (HeapState->_bSupportsAutomaticRetry && Data->ResultCode == EOS_EResult::EOS_TimedOut)                      \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(LogCallName(), TEXT("Async call received EOS_TimedOut."), Data->ResultCode)              \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(false);                                                                           \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Async call completed."), Data->ResultCode).ToLogString());              \
                                                                                                                       \
        Result _Result;                                                                                                \
        MapResult(_Result, *Data, *MakeShared<NativeConverter>());                                                     \
        HeapState->_CompletionDelegate.ExecuteIfBound(_Result);                                                        \
        delete HeapState;                                                                                              \
    }                                                                                                                  \
    void F##CallName::Execute(                                                                                         \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        F##CallName::ExecuteWithCurrentAttemptCount(InHandle, InOptions, InCompletionDelegate, 1);                     \
    }                                                                                                                  \
    void F##CallName::ExecuteWithCurrentAttemptCount(                                                                  \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate,                                                                \
        int CurrentAttemptCount)                                                                                       \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());                                  \
                                                                                                                       \
        if (InHandle->Get<NativeHandle>() == nullptr)                                                                  \
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
                                                                                                                       \
            Result _DeadResult;                                                                                        \
            _DeadResult.ResultCode = EOS_EResult::EOS_NoConnection;                                                    \
            InCompletionDelegate.ExecuteIfBound(_DeadResult);                                                          \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator);                                                             \
                                                                                                                       \
        bool bSupportsAutomaticRetry = true;                                                                           \
        if (!GConfig->GetBool(                                                                                         \
                TEXT("EpicOnlineServices"),                                                                            \
                TEXT("bEnableAutomaticRetries"),                                                                       \
                bSupportsAutomaticRetry,                                                                               \
                GEngineIni))                                                                                           \
        {                                                                                                              \
            /* If there is no configuration value, default to true. */                                                 \
            bSupportsAutomaticRetry = true;                                                                            \
        }                                                                                                              \
                                                                                                                       \
        FHeapState *HeapState = new FHeapState();                                                                      \
        FCallbackValidator::TrackCallback(HeapState, LogCallName());                                                   \
        HeapState->_NativeOptions = _NativeOptions;                                                                    \
        HeapState->_CompletionDelegate = InCompletionDelegate;                                                         \
        HeapState->_Allocator = Allocator;                                                                             \
        HeapState->_bSupportsAutomaticRetry = bSupportsAutomaticRetry;                                                 \
        HeapState->_ExecuteRetry = [InHandle, InOptions, InCompletionDelegate, CurrentAttemptCount](                   \
                                       bool bTooManyRequests) {                                                        \
            FTSTicker::GetCoreTicker().AddTicker(                                                                      \
                FTickerDelegate::CreateLambda(                                                                         \
                    [InHandle, InOptions, InCompletionDelegate, CurrentAttemptCount](float) {                          \
                        F##CallName::ExecuteWithCurrentAttemptCount(                                                   \
                            InHandle,                                                                                  \
                            InOptions,                                                                                 \
                            InCompletionDelegate,                                                                      \
                            CurrentAttemptCount + 1);                                                                  \
                        return false;                                                                                  \
                    }),                                                                                                \
                bTooManyRequests ? (60.0f + FMath::RandRange(0.0f, 15.0f))                                             \
                                 : (static_cast<float>(CurrentAttemptCount) * 5.0f + FMath::RandRange(0.0f, 2.5f)));   \
        };                                                                                                             \
                                                                                                                       \
        EOS_EResult ImmediateResult = NativeFunction(                                                                  \
            InHandle->Get<NativeHandle>(),                                                                             \
            &HeapState->_NativeOptions,                                                                                \
            HeapState,                                                                                                 \
            &HandleNativeCallback);                                                                                    \
        if (ImmediateResult != EOS_EResult::EOS_Success)                                                               \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(LogCallName(), TEXT("Async call immediately failed."), ImmediateResult).ToLogString());  \
                                                                                                                       \
            Result _ImmediateResult;                                                                                   \
            _ImmediateResult.ResultCode = ImmediateResult;                                                             \
            HeapState->_CompletionDelegate.ExecuteIfBound(_ImmediateResult);                                           \
            delete HeapState;                                                                                          \
        }                                                                                                              \
    }

REDPOINT_EOS_CODE_GUARD_END()