// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/ObjectAsyncListReleasable.h"

#include "RedpointEOSAPI/CallbackValidator.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_IMPL(CallName)                                           \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
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
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[queried ] %s"),                                                                                     \
            *ConvertError(                                                                                             \
                 LogCallQueryName(),                                                                                   \
                 TEXT("ObjectAsyncList call received query result."),                                                  \
                 QueryData->ResultCode)                                                                                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        FHeapState *HeapState = (FHeapState *)QueryData->ClientData;                                                   \
                                                                                                                       \
        TArray<TReleasableRef<NativeCopyByIndexResult>> ResultEntries;                                                 \
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
            Count = NativeCountFunction(HeapState->_NativeHandle->Value(), &_NativeCountOptions);                      \
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
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[starting] %s (index: %d)"),                                                                     \
                LogCallCopyByIndexName(),                                                                              \
                Index);                                                                                                \
            NativeCopyByIndexResult _NativeResultEntry = {};                                                           \
            NativeCopyByIndexOptions _NativeCopyByIndexOptions = {};                                                   \
            _NativeCopyByIndexOptions.ApiVersion = NativeCopyByIndexOptionsVersion();                                  \
            MapCopyByIndexOptions(_NativeCopyByIndexOptions, HeapState->_Options, Index, *HeapState->_Allocator);      \
            EOS_EResult CopyResult = NativeCopyByIndexFunction(                                                        \
                HeapState->_NativeHandle->Value(),                                                                     \
                &_NativeCopyByIndexOptions,                                                                            \
                &_NativeResultEntry);                                                                                  \
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
            ResultEntries.Add(MakeShareable(new TReleasable<NativeCopyByIndexResult>(                                  \
                *HeapState->_NativeHandle,                                                                             \
                _NativeResultEntry,                                                                                    \
                TDelegate<void(NativeCopyByIndexResult Value)>::CreateStatic(&F##CallName::NativeReleaseFunction))));  \
                                                                                                                       \
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
                 *FString::Printf(TEXT("ObjectAsyncList call completed with %d results."), ResultEntries.Num()),       \
                 QueryData->ResultCode)                                                                                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        HeapState->_CompletionDelegate.ExecuteIfBound(QueryData->ResultCode, ResultEntries);                           \
        delete HeapState;                                                                                              \
    }                                                                                                                  \
    void F##CallName::Execute(                                                                                         \
        TReleasableRef<NativeHandle> InHandle,                                                                         \
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
        if (!InHandle->IsValid())                                                                                      \
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
            InCompletionDelegate.ExecuteIfBound(                                                                       \
                EOS_EResult::EOS_NoConnection,                                                                         \
                TArray<TReleasableRef<NativeCopyByIndexResult>>());                                                    \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeQueryOptions _NativeQueryOptions;                                                                        \
        _NativeQueryOptions.ApiVersion = NativeQueryOptionsVersion();                                                  \
        MapQueryOptions(_NativeQueryOptions, InOptions, *Allocator);                                                   \
                                                                                                                       \
        FHeapState *HeapState = new FHeapState(InHandle, InOptions);                                                   \
        FCallbackValidator::TrackCallback(HeapState, LogCallQueryName());                                              \
        HeapState->_NativeQueryOptions = _NativeQueryOptions;                                                          \
        HeapState->_CompletionDelegate = InCompletionDelegate;                                                         \
        HeapState->_Allocator = Allocator;                                                                             \
                                                                                                                       \
        NativeQueryFunction(                                                                                           \
            InHandle->Value(),                                                                                         \
            &HeapState->_NativeQueryOptions,                                                                           \
            HeapState,                                                                                                 \
            &HandleNativeQueryCallback);                                                                               \
    }

REDPOINT_EOS_CODE_GUARD_END()