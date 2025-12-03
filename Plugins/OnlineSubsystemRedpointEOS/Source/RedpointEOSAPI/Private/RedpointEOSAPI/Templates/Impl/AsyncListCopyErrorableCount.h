// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/AsyncListCopyErrorableCount.h"

#include "Containers/Ticker.h"
#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSAPI/CallbackValidator.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_IMPL(CallName)                                        \
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
        if (HeapState->_bSupportsAutomaticRetry && QueryData->ResultCode == EOS_EResult::EOS_AlreadyPending)           \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncList call received EOS_AlreadyPending."),                                              \
                     QueryData->ResultCode)                                                                            \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(false);                                                                           \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (HeapState->_bSupportsAutomaticRetry && QueryData->ResultCode == EOS_EResult::EOS_TooManyRequests)          \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncList call received EOS_TooManyRequests."),                                             \
                     QueryData->ResultCode)                                                                            \
                     .ToLogString());                                                                                  \
            HeapState->_ExecuteRetry(true);                                                                            \
            delete HeapState;                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
        if (HeapState->_bSupportsAutomaticRetry && QueryData->ResultCode == EOS_EResult::EOS_TimedOut)                 \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[retry   ] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallQueryName(),                                                                               \
                     TEXT("AsyncList call received EOS_TimedOut."),                                                    \
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
            *ConvertError(LogCallQueryName(), TEXT("AsyncList call received query result."), QueryData->ResultCode)    \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        TArray<ResultEntry> ResultEntries;                                                                             \
                                                                                                                       \
        if (QueryData->ResultCode != EOS_EResult::EOS_Success)                                                         \
        {                                                                                                              \
            HeapState->_CompletionDelegate.ExecuteIfBound(QueryData->ResultCode, ResultEntries);                       \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeConverter> _Converter = MakeShared<NativeConverter>();                                        \
        int32_t Count = 0;                                                                                             \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_Count);                                                                      \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallCountName());                         \
            NativeCountOptions _NativeCountOptions = {};                                                               \
            _NativeCountOptions.ApiVersion = NativeCountOptionsVersion();                                              \
            MapCountOptions(_NativeCountOptions, HeapState->_Options, *HeapState->_Allocator);                         \
            EOS_EResult CountResult = NativeCountFunction(HeapState->_NativeHandle, &_NativeCountOptions, &Count);     \
            if (CountResult != EOS_EResult::EOS_Success)                                                               \
            {                                                                                                          \
                UE_LOG(                                                                                                \
                    LogRedpointEOSAPI,                                                                                 \
                    Warning,                                                                                           \
                    TEXT("[informat] %s"),                                                                             \
                    *ConvertError(                                                                                     \
                         LogCallCountName(),                                                                           \
                         *FString::Printf(TEXT("Count returned error or nullptr."), Count),                            \
                         CountResult)                                                                                  \
                         .ToLogString());                                                                              \
                HeapState->_CompletionDelegate.ExecuteIfBound(CountResult, ResultEntries);                             \
                return;                                                                                                \
            }                                                                                                          \
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
        for (uint32_t Index = 0; Index < static_cast<uint32_t>(Count); Index++)                                        \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_CopyByIndex);                                                                \
                                                                                                                       \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[starting] %s (index: %d)"),                                                                     \
                LogCallCopyByIndexName(),                                                                              \
                Index);                                                                                                \
            NativeCopyByIndexResult *_NativeResultEntry;                                                               \
            NativeCopyByIndexOptions _NativeCopyByIndexOptions = {};                                                   \
            _NativeCopyByIndexOptions.ApiVersion = NativeCopyByIndexOptionsVersion();                                  \
            MapCopyByIndexOptions(_NativeCopyByIndexOptions, HeapState->_Options, Index, *HeapState->_Allocator);      \
            EOS_EResult CopyResult =                                                                                   \
                NativeCopyByIndexFunction(HeapState->_NativeHandle, &_NativeCopyByIndexOptions, &_NativeResultEntry);  \
            if (CopyResult != EOS_EResult::EOS_Success || _NativeResultEntry == nullptr)                               \
            {                                                                                                          \
                UE_LOG(                                                                                                \
                    LogRedpointEOSAPI,                                                                                 \
                    Warning,                                                                                           \
                    TEXT("[informat] %s"),                                                                             \
                    *ConvertError(                                                                                     \
                         LogCallCopyByIndexName(),                                                                     \
                         *FString::Printf(TEXT("CopyByIndex returned error or nullptr."), Count),                      \
                         CopyResult)                                                                                   \
                         .ToLogString());                                                                              \
                continue;                                                                                              \
            }                                                                                                          \
                                                                                                                       \
            ResultEntry _ResultEntry;                                                                                  \
            MapCopyByIndexResult(_ResultEntry, *_NativeResultEntry, *_Converter);                                      \
            NativeReleaseFunction(_NativeResultEntry);                                                                 \
            ResultEntries.Add(_ResultEntry);                                                                           \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallCopyByIndexName(),                                                                         \
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
                 *FString::Printf(TEXT("AsyncList call completed with %d results."), ResultEntries.Num()),             \
                 QueryData->ResultCode)                                                                                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        HeapState->_CompletionDelegate.ExecuteIfBound(QueryData->ResultCode, ResultEntries);                           \
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
            InCompletionDelegate.ExecuteIfBound(EOS_EResult::EOS_NoConnection, TArray<ResultEntry>());                 \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        _NativeQueryOptions.ApiVersion = NativeQueryOptionsVersion();                                                  \
        MapQueryOptions(_NativeQueryOptions, InOptions, *Allocator);                                                   \
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
        FHeapState *HeapState = new FHeapState(InOptions);                                                             \
        FCallbackValidator::TrackCallback(HeapState, LogCallQueryName());                                              \
        HeapState->_NativeHandle = InHandle->Get<NativeHandle>();                                                      \
        HeapState->_NativeQueryOptions = _NativeQueryOptions;                                                          \
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
        NativeQueryFunction(                                                                                           \
            InHandle->Get<NativeHandle>(),                                                                             \
            &HeapState->_NativeQueryOptions,                                                                           \
            HeapState,                                                                                                 \
            &HandleNativeQueryCallback);                                                                               \
    }                                                                                                                  \
    void F##CallName::Execute(                                                                                         \
        NativeHandle InHandle,                                                                                         \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallQueryName());                             \
                                                                                                                       \
        if (InHandle == nullptr)                                                                                       \
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
            InCompletionDelegate.ExecuteIfBound(EOS_EResult::EOS_NoConnection, TArray<ResultEntry>());                 \
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
        HeapState->_NativeHandle = InHandle;                                                                           \
        HeapState->_NativeQueryOptions = _NativeQueryOptions;                                                          \
        HeapState->_CompletionDelegate = InCompletionDelegate;                                                         \
        HeapState->_Allocator = Allocator;                                                                             \
        HeapState->_bSupportsAutomaticRetry = false;                                                                   \
        HeapState->_ExecuteRetry = [](bool) {                                                                          \
        };                                                                                                             \
                                                                                                                       \
        NativeQueryFunction(InHandle, &HeapState->_NativeQueryOptions, HeapState, &HandleNativeQueryCallback);         \
    }

REDPOINT_EOS_CODE_GUARD_END()