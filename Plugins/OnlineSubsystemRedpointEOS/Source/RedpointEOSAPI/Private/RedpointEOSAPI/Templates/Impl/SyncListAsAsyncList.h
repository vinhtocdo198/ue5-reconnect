// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncListAsAsyncList.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_AS_ASYNC_LIST_IMPL(CallName)                                                \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void F##CallName::Execute(                                                                                         \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        return Execute(InHandle->Get<NativeHandle>(), InOptions, InCompletionDelegate);                                \
    }                                                                                                                  \
    void F##CallName::Execute(                                                                                         \
        NativeHandle InHandle,                                                                                         \
        const Options &InOptions,                                                                                      \
        const CompletionDelegate &InCompletionDelegate)                                                                \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCountName(), STAT_Call_Count, STATGROUP_RedpointEOS);                               \
        DECLARE_CYCLE_STAT(StatCallGetByIndexName(), STAT_Call_GetByIndex, STATGROUP_RedpointEOS);                     \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallGetByIndexName());                        \
                                                                                                                       \
        if (InHandle == nullptr)                                                                                       \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallGetByIndexName(),                                                                          \
                     TEXT("The platform instance has been shutdown, so SDK calls can not be made."),                   \
                     EOS_EResult::EOS_NoConnection)                                                                    \
                     .ToLogString());                                                                                  \
            InCompletionDelegate.ExecuteIfBound(EOS_EResult::EOS_NoConnection, TArray<ResultEntry>());                 \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> _Allocator = MakeShared<NativeAllocator>();                                        \
        TSharedRef<NativeConverter> _Converter = MakeShared<NativeConverter>();                                        \
                                                                                                                       \
        TArray<ResultEntry> ResultEntries;                                                                             \
                                                                                                                       \
        uint32_t Count = 0;                                                                                            \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_Count);                                                                      \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallCountName());                         \
            NativeCountOptions _NativeCountOptions = {};                                                               \
            _NativeCountOptions.ApiVersion = NativeCountOptionsVersion();                                              \
            MapCountOptions(_NativeCountOptions, InOptions, *_Allocator);                                              \
            Count = NativeCountFunction(InHandle, &_NativeCountOptions);                                               \
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
            SCOPE_CYCLE_COUNTER(STAT_Call_GetByIndex);                                                                 \
                                                                                                                       \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s (index: %d)"), LogCallGetByIndexName(), Index); \
            NativeGetByIndexOptions _NativeGetByIndexOptions = {};                                                     \
            _NativeGetByIndexOptions.ApiVersion = NativeGetByIndexOptionsVersion();                                    \
            MapCopyByIndexOptions(_NativeGetByIndexOptions, InOptions, Index, *_Allocator);                            \
            const NativeGetByIndexResult *_NativeResultEntry =                                                         \
                NativeGetByIndexFunction(InHandle, &_NativeGetByIndexOptions);                                         \
            if (_NativeResultEntry == nullptr)                                                                         \
            {                                                                                                          \
                UE_LOG(                                                                                                \
                    LogRedpointEOSAPI,                                                                                 \
                    Warning,                                                                                           \
                    TEXT("[informat] %s: Unexpected nullptr returned from GetByIndex call."),                          \
                    LogCallGetByIndexName());                                                                          \
                continue;                                                                                              \
            }                                                                                                          \
                                                                                                                       \
            ResultEntry _ResultEntry;                                                                                  \
            MapCopyByIndexResult(_ResultEntry, *_NativeResultEntry, *_Converter);                                      \
            ResultEntries.Add(_ResultEntry);                                                                           \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallGetByIndexName(),                                                                          \
                     *FString::Printf(TEXT("Retrieved result from index %d."), Index),                                 \
                     EOS_EResult::EOS_Success)                                                                         \
                     .ToLogString());                                                                                  \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(                                                                                             \
                 LogCallGetByIndexName(),                                                                              \
                 TEXT("SyncListAsAsyncList call completed."),                                                          \
                 EOS_EResult::EOS_Success)                                                                             \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        InCompletionDelegate.ExecuteIfBound(EOS_EResult::EOS_Success, ResultEntries);                                  \
    }

REDPOINT_EOS_CODE_GUARD_END()