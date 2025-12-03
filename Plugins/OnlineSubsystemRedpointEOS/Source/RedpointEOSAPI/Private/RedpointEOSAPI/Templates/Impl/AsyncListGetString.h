// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/AsyncListGetString.h"

#include "Containers/Ticker.h"
#include "RedpointEOSAPI/CallbackValidator.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_GET_STRING_IMPL(CallName)                                                  \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                                    \
                                                                                                                       \
    void __REDPOINT_EOSSDK_CDECL_ATTR F##CallName::HandleNativeQueryCallback(const NativeQueryResult *QueryData)       \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCompletionName(), STAT_Call_Completion, STATGROUP_RedpointEOS);                     \
        DECLARE_CYCLE_STAT(StatCallCountName(), STAT_Call_Count, STATGROUP_RedpointEOS);                               \
        DECLARE_CYCLE_STAT(StatCallCopyByIndexName(), STAT_Call_CopyByIndex, STATGROUP_RedpointEOS);                   \
        SCOPE_CYCLE_COUNTER(STAT_Call_Completion);                                                                     \
                                                                                                                       \
        if (!EOS_EResult_IsOperationComplete(QueryData->ResultCode))                                                   \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[pending ] %s"), LogCallQueryName());                         \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        if (!FCallbackValidator::VerifyCallback(QueryData->ClientData, QueryData->ResultCode))                         \
        {                                                                                                              \
            UE_LOG(LogRedpointEOSAPI, Error, TEXT("[double call! ignoring] %s"), LogCallQueryName());                  \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)QueryData->ClientData;                                                   \
                                                                                                                       \
        if (QueryData->ResultCode == EOS_EResult::EOS_AlreadyPending)                                                  \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncListGetString call received EOS_AlreadyPending."),                                     \
                     QueryData->ResultCode)                                                                            \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(false);                                                                           \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (QueryData->ResultCode == EOS_EResult::EOS_TooManyRequests)                                                 \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncListGetString call received EOS_TooManyRequests."),                                    \
                     QueryData->ResultCode)                                                                            \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(true);                                                                            \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (QueryData->ResultCode == EOS_EResult::EOS_TimedOut)                                                        \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncListGetString call received EOS_TimedOut."),                                           \
                     QueryData->ResultCode)                                                                            \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(false);                                                                           \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[queried ] %s"),                                                                                     \
            *ConvertError(                                                                                             \
                 LogCallQueryName(),                                                                                   \
                 TEXT("AsyncListGetString call received query result."),                                               \
                 QueryData->ResultCode)                                                                                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        TArray<FString> ResultEntries;                                                                                 \
                                                                                                                       \
        if (QueryData->ResultCode != EOS_EResult::EOS_Success)                                                         \
        {                                                                                                              \
            HeapState->_CompletionDelegate.ExecuteIfBound(QueryData->ResultCode, ResultEntries);                       \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeConverter> _Converter = MakeShared<NativeConverter>();                                        \
        uint32_t Count = 0;                                                                                            \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_Count);                                                                      \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallCountName());                         \
            NativeCountOptions _NativeCountOptions = {};                                                               \
            _NativeCountOptions.ApiVersion = NativeCountOptionsVersion();                                              \
            MapCountOptions(_NativeCountOptions, HeapState->_Options, *HeapState->_Allocator);                         \
            Count = NativeCountFunction(HeapState->_NativeHandle, &_NativeCountOptions);                               \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallCountName(),                                                                               \
                     *FString::Printf(TEXT("Count returned %d results."), Count),                                      \
                     EOS_EResult::EOS_Success)                                                                         \
                     .ToLogString());                                                                                  \
        }                                                                                                              \
                                                                                                                       \
        for (uint32_t Index = 0; Index < Count; Index++)                                                               \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_CopyByIndex);                                                                \
                                                                                                                       \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s (index: %d)"), LogCallGetByIndexName(), Index); \
            int32_t NativeResultStackBufferLength = NativeGetStringMaxLength();                                        \
            char *NativeResultStackBuffer =                                                                            \
                (char *)FMemory::MallocZeroed((SIZE_T)(NativeResultStackBufferLength + 1) * sizeof(char));             \
            NativeGetByIndexOptions _NativeGetByIndexOptions = {};                                                     \
            _NativeGetByIndexOptions.ApiVersion = NativeGetByIndexOptionsVersion();                                    \
            MapGetByIndexOptions(_NativeGetByIndexOptions, HeapState->_Options, Index, *HeapState->_Allocator);        \
            EOS_EResult GetResult = NativeGetByIndexFunction(                                                          \
                HeapState->_NativeHandle,                                                                              \
                &_NativeGetByIndexOptions,                                                                             \
                NativeResultStackBuffer,                                                                               \
                &NativeResultStackBufferLength);                                                                       \
            if (GetResult != EOS_EResult::EOS_Success)                                                                 \
            {                                                                                                          \
                FMemory::Free(NativeResultStackBuffer);                                                                \
                NativeResultStackBuffer = nullptr;                                                                     \
                UE_LOG(                                                                                                \
                    LogRedpointEOSAPI,                                                                                 \
                    Warning,                                                                                           \
                    TEXT("[informat] %s"),                                                                             \
                    *ConvertError(                                                                                     \
                         LogCallGetByIndexName(),                                                                      \
                         *FString::Printf(TEXT("GetByIndex returned error."), Count),                                  \
                         GetResult)                                                                                    \
                         .ToLogString());                                                                              \
                continue;                                                                                              \
            }                                                                                                          \
                                                                                                                       \
            FString _ResultEntry = _Converter->FromUtf8(NativeResultStackBuffer);                                      \
            FMemory::Free(NativeResultStackBuffer);                                                                    \
            ResultEntries.Add(_ResultEntry);                                                                           \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallGetByIndexName(),                                                                          \
                     *FString::Printf(TEXT("Copied result from index %d."), Index),                                    \
                     EOS_EResult::EOS_Success)                                                                         \
                     .ToLogString());                                                                                  \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(                                                                                             \
                 LogCallQueryName(),                                                                                   \
                 *FString::Printf(TEXT("AsyncListGetString call completed with %d results."), ResultEntries.Num()),    \
                 QueryData->ResultCode)                                                                                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        HeapState->_CompletionDelegate.ExecuteIfBound(QueryData->ResultCode, ResultEntries);                           \
        delete HeapState;                                                                                              \
    }                                                                                                                  \
    void F##CallName::Execute(                                                                                         \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        F##CallName::ExecuteWithCurrentAttemptCount(InHandle, InOptions, InCompletionDelegate, 1);                     \
    }                                                                                                                  \
    void F##CallName::ExecuteWithCurrentAttemptCount(                                                                  \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate,                                                                \
        int CurrentAttemptCount)                                                                                       \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallQueryName());                             \
                                                                                                                       \
        if (InHandle->Get<NativeHandle>() == nullptr)                                                                  \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("The platform instance has been shutdown, so SDK calls can not be made."),                   \
                     EOS_EResult::EOS_NoConnection)                                                                    \
                     .ToLogString());                                                                                  \
            InCompletionDelegate.ExecuteIfBound(EOS_EResult::EOS_NoConnection, TArray<FString>());                     \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        _NativeQueryOptions.ApiVersion = NativeQueryOptionsVersion();                                                  \
        MapQueryOptions(_NativeQueryOptions, InOptions, *Allocator);                                                   \
                                                                                                                       \
        FHeapState *HeapState = new FHeapState(InOptions);                                                             \
        FCallbackValidator::TrackCallback(HeapState, LogCallQueryName());                                              \
        HeapState->_NativeHandle = InHandle->Get<NativeHandle>();                                                      \
        HeapState->_NativeQueryOptions = _NativeQueryOptions;                                                          \
        HeapState->_CompletionDelegate = InCompletionDelegate;                                                         \
        HeapState->_Allocator = Allocator;                                                                             \
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
        NativeQueryFunction(                                                                                           \
            InHandle->Get<NativeHandle>(),                                                                             \
            &HeapState->_NativeQueryOptions,                                                                           \
            HeapState,                                                                                                 \
            &HandleNativeQueryCallback);                                                                               \
    }

REDPOINT_EOS_CODE_GUARD_END()