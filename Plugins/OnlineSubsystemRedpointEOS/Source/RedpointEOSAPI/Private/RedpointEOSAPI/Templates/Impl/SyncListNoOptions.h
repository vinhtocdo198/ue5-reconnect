// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncListNoOptions.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_LIST_NO_OPTIONS_IMPL(CallName)                                                   \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void F##CallName::Execute(                                                                                         \
        const FPlatformHandle &InHandle,                                                                               \
        EOS_EResult &OutResultCode,                                                                                    \
        TArray<ResultEntry> &OutResultEntries)                                                                         \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                              \
        OutResultEntries = {};                                                                                         \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallCountName(), STAT_Call_Count, STATGROUP_RedpointEOS);                               \
        DECLARE_CYCLE_STAT(StatCallGetByIndexName(), STAT_Call_GetByIndex, STATGROUP_RedpointEOS);                     \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallGetByIndexName());                        \
                                                                                                                       \
        if (InHandle->Get<NativeHandle>() == nullptr)                                                                  \
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
            OutResultCode = EOS_EResult::EOS_NoConnection;                                                             \
            OutResultEntries = TArray<ResultEntry>();                                                                  \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> _Allocator = MakeShared<NativeAllocator>();                                        \
                                                                                                                       \
        TArray<ResultEntry> ResultEntries;                                                                             \
                                                                                                                       \
        NativeCount Count = 0;                                                                                         \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_Count);                                                                      \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallCountName());                         \
            Count = NativeCountFunction(InHandle->Get<NativeHandle>());                                                \
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
        for (NativeCount Index = 0; Index < Count; Index++)                                                            \
        {                                                                                                              \
            SCOPE_CYCLE_COUNTER(STAT_Call_GetByIndex);                                                                 \
                                                                                                                       \
            UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s (index: %d)"), LogCallGetByIndexName(), Index); \
            NativeGetByIndexResult _NativeResultEntry;                                                                 \
            _NativeResultEntry = NativeGetByIndexFunction(InHandle->Get<NativeHandle>(), Index);                       \
                                                                                                                       \
            ResultEntry _ResultEntry = MapGetByIndexResult(_NativeResultEntry);                                        \
            ResultEntries.Add(_ResultEntry);                                                                           \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallGetByIndexName(), TEXT("SyncList call completed."), EOS_EResult::EOS_Success)         \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_Success;                                                                      \
        OutResultEntries = ResultEntries;                                                                              \
    }

REDPOINT_EOS_CODE_GUARD_END()