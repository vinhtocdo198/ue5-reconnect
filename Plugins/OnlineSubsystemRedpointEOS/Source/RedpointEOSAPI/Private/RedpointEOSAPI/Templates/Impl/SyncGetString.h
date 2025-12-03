// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncGetString.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_GET_STRING_IMPL(CallName)                                                        \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void F##CallName::Execute(                                                                                         \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode,                                                                                    \
        FString &OutResult)                                                                                            \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                              \
        OutResult = TEXT("");                                                                                          \
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
            OutResultCode = EOS_EResult::EOS_NoConnection;                                                             \
            OutResult = TEXT("");                                                                                      \
            return;                                                                                                    \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
        TSharedRef<NativeConverter> Converter = MakeShared<NativeConverter>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator);                                                             \
                                                                                                                       \
        uint32_t NativeResultStackBufferLength = 1;                                                                    \
        char *NativeResultStackBuffer =                                                                                \
            (char *)FMemory::MallocZeroed((NativeResultStackBufferLength + 1) * sizeof(char));                         \
        OutResultCode = NativeFunction(                                                                                \
            InHandle->Get<NativeHandle>(),                                                                             \
            &_NativeOptions,                                                                                           \
            NativeResultStackBuffer,                                                                                   \
            &NativeResultStackBufferLength);                                                                           \
        if (OutResultCode == EOS_EResult::EOS_LimitExceeded)                                                           \
        {                                                                                                              \
            FMemory::Free(NativeResultStackBuffer);                                                                    \
            NativeResultStackBuffer =                                                                                  \
                (char *)FMemory::MallocZeroed((NativeResultStackBufferLength + 1) * sizeof(char));                     \
            OutResultCode = NativeFunction(                                                                            \
                InHandle->Get<NativeHandle>(),                                                                         \
                &_NativeOptions,                                                                                       \
                NativeResultStackBuffer,                                                                               \
                &NativeResultStackBufferLength);                                                                       \
        }                                                                                                              \
        if (OutResultCode == EOS_EResult::EOS_Success)                                                                 \
        {                                                                                                              \
            OutResult = Converter->FromUtf8(NativeResultStackBuffer);                                                  \
        }                                                                                                              \
        FMemory::Free(NativeResultStackBuffer);                                                                        \
        NativeResultStackBuffer = nullptr;                                                                             \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Sync get string call completed."), OutResultCode).ToLogString());       \
        return;                                                                                                        \
    }

REDPOINT_EOS_CODE_GUARD_END()