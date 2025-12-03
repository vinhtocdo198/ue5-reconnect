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

#define REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_IMPL(CallName)                                                     \
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
    F##CallName::NativeWriteResultEnum __REDPOINT_EOSSDK_CDECL_ATTR F##CallName::HandleNativeWriteCallback(            \
        const NativeWriteResult *Data,                                                                                 \
        void *OutDataBuffer,                                                                                           \
        uint32_t *OutDataWritten)                                                                                      \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallWriteName(), STAT_Call_Write, STATGROUP_RedpointEOS);                               \
        SCOPE_CYCLE_COUNTER(STAT_Call_Write);                                                                          \
                                                                                                                       \
        if (!FCallbackValidator::VerifyCallbackWithoutRelease(Data->ClientData, TOptional<EOS_EResult>()))             \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, Error, TEXT("[double call! ignoring] %s"), LogCallName());                       \
            return MapWriteResultEnum(EWriteResult::FailRequest);                                                      \
        }                                                                                                              \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)Data->ClientData;                                                        \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[write] %s"), LogCallName());                                     \
                                                                                                                       \
        WriteResult _WriteResult;                                                                                      \
        MapWriteResult(_WriteResult, *Data, *MakeShared<NativeConverter>());                                           \
        TArray<uint8> _WriteBuffer;                                                                                    \
        EWriteResult _WriteResultEnum = EWriteResult::FailRequest;                                                     \
        if (HeapState->_WriteDelegate.IsBound())                                                                       \
        {                                                                                                              \
            _WriteResultEnum = HeapState->_WriteDelegate.Execute(_WriteResult, _WriteBuffer);                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                Warning,                                                                                               \
                TEXT("[write] %s: Write will be marked as failure as the write delegate is no longer bound."),         \
                LogCallName());                                                                                        \
        }                                                                                                              \
        if (_WriteResultEnum == EWriteResult::ContinueWriting || _WriteResultEnum == EWriteResult::CompleteRequest)    \
        {                                                                                                              \
            if (static_cast<uint32_t>(_WriteBuffer.Num()) > Data->DataBufferLengthBytes)                               \
            {                                                                                                          \
                UE_LOG(                                                                                                \
                    LogRedpointEOSAPI,                                                                                 \
                    Warning,                                                                                           \
                    TEXT(                                                                                              \
                        "[write] %s: Write will be truncated as chunk buffer exceeds maximum permitted data length."), \
                    LogCallName());                                                                                    \
            }                                                                                                          \
            *OutDataWritten = FMath::Min(static_cast<uint32_t>(_WriteBuffer.Num()), Data->DataBufferLengthBytes);      \
            FMemory::Memcpy(OutDataBuffer, _WriteBuffer.GetData(), *OutDataWritten);                                   \
        };                                                                                                             \
        return MapWriteResultEnum(_WriteResultEnum);                                                                   \
    }                                                                                                                  \
    void __REDPOINT_EOSSDK_CDECL_ATTR F##CallName::HandleNativeProgressCallback(const NativeProgressResult *Data)      \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallProgressName(), STAT_Call_Progress, STATGROUP_RedpointEOS);                         \
        SCOPE_CYCLE_COUNTER(STAT_Call_Progress);                                                                       \
                                                                                                                       \
        if (!FCallbackValidator::VerifyCallbackWithoutRelease(Data->ClientData, TOptional<EOS_EResult>()))             \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, Error, TEXT("[double call! ignoring] %s"), LogCallName());                       \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)Data->ClientData;                                                        \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[progress] %s"), LogCallName());                                  \
                                                                                                                       \
        ProgressResult _ProgressResult;                                                                                \
        MapProgressResult(_ProgressResult, *Data, *MakeShared<NativeConverter>());                                     \
        HeapState->_ProgressDelegate.ExecuteIfBound(_ProgressResult);                                                  \
    }                                                                                                                  \
    F##CallName::NativeReturn F##CallName::Execute(                                                                    \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const WriteDelegate &InWriteDelegate,                                                                          \
        const ProgressDelegate &InProgressDelegate,                                                                    \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        return F##CallName::ExecuteWithCurrentAttemptCount(                                                            \
            InHandle,                                                                                                  \
            InOptions,                                                                                                 \
            InWriteDelegate,                                                                                           \
            InProgressDelegate,                                                                                        \
            InCompletionDelegate,                                                                                      \
            1);                                                                                                        \
    }                                                                                                                  \
    F##CallName::NativeReturn F##CallName::ExecuteWithCurrentAttemptCount(                                             \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const WriteDelegate &InWriteDelegate,                                                                          \
        const ProgressDelegate &InProgressDelegate,                                                                    \
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
            return NativeReturn();                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator, &HandleNativeWriteCallback, &HandleNativeProgressCallback);  \
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
        HeapState->_WriteDelegate = InWriteDelegate;                                                                   \
        HeapState->_ProgressDelegate = InProgressDelegate;                                                             \
        HeapState->_Allocator = Allocator;                                                                             \
        HeapState->_bSupportsAutomaticRetry = bSupportsAutomaticRetry;                                                 \
        HeapState->_ExecuteRetry =                                                                                     \
            [InHandle, InOptions, InWriteDelegate, InProgressDelegate, InCompletionDelegate, CurrentAttemptCount](     \
                bool bTooManyRequests) {                                                                               \
                FTSTicker::GetCoreTicker().AddTicker(                                                                  \
                    FTickerDelegate::CreateLambda([InHandle,                                                           \
                                                   InOptions,                                                          \
                                                   InWriteDelegate,                                                    \
                                                   InProgressDelegate,                                                 \
                                                   InCompletionDelegate,                                               \
                                                   CurrentAttemptCount](float) {                                       \
                        F##CallName::ExecuteWithCurrentAttemptCount(                                                   \
                            InHandle,                                                                                  \
                            InOptions,                                                                                 \
                            InWriteDelegate,                                                                           \
                            InProgressDelegate,                                                                        \
                            InCompletionDelegate,                                                                      \
                            CurrentAttemptCount + 1);                                                                  \
                        return false;                                                                                  \
                    }),                                                                                                \
                    bTooManyRequests                                                                                   \
                        ? (60.0f + FMath::RandRange(0.0f, 15.0f))                                                      \
                        : (static_cast<float>(CurrentAttemptCount) * 5.0f + FMath::RandRange(0.0f, 2.5f)));            \
            };                                                                                                         \
                                                                                                                       \
        return NativeFunction(                                                                                         \
            InHandle->Get<NativeHandle>(),                                                                             \
            &HeapState->_NativeOptions,                                                                                \
            HeapState,                                                                                                 \
            &HandleNativeCallback);                                                                                    \
    }

REDPOINT_EOS_CODE_GUARD_END()