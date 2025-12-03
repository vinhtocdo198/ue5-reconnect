// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncListCopy.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_COPY_IMPL(CallName)                                                         \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void F##CallName::Execute(                                                                                         \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode,                                                                                    \
        TArray<ResultEntry> &OutResultEntries)                                                                         \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                              \
        OutResultEntries = {};                                                                                         \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCountName(), STAT_Call_Count, STATGROUP_RedpointEOS);                               \
        DECLARE_CYCLE_STAT(StatCallCopyByIndexName(), STAT_Call_CopyByIndex, STATGROUP_RedpointEOS);                   \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallCopyByIndexName());                       \
                                                                                                                       \
        if (InHandle->Get<NativeHandle>() == nullptr)                                                                  \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallCopyByIndexName(),                                                                         \
                     TEXT("The platform instance has been shutdown, so SDK calls can not be made."),                   \
                     EOS_EResult::EOS_NoConnection)                                                                    \
                     .ToLogString());                                                                                  \
            OutResultCode = EOS_EResult::EOS_NoConnection;                                                             \
            OutResultEntries = TArray<ResultEntry>();                                                                  \
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
            Count = NativeCountFunction(InHandle->Get<NativeHandle>(), &_NativeCountOptions);                          \
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
            NativeCopyByIndexResult *_NativeResultEntry;                                                               \
            NativeCopyByIndexOptions _NativeCopyByIndexOptions = {};                                                   \
            _NativeCopyByIndexOptions.ApiVersion = NativeCopyByIndexOptionsVersion();                                  \
            MapCopyByIndexOptions(_NativeCopyByIndexOptions, InOptions, Index, *_Allocator);                           \
            EOS_EResult CopyResult = NativeCopyByIndexFunction(                                                        \
                InHandle->Get<NativeHandle>(),                                                                         \
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
            ResultEntry _ResultEntry = {};                                                                             \
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
                 LogCallCopyByIndexName(),                                                                             \
                 TEXT("ObjectSyncListCopy call completed."),                                                           \
                 EOS_EResult::EOS_Success)                                                                             \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_Success;                                                                      \
        OutResultEntries = ResultEntries;                                                                              \
    }

REDPOINT_EOS_CODE_GUARD_END()