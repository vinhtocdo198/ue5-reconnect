// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncAsAsync.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_AS_ASYNC_IMPL(CallName)                                                          \
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
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());                                  \
                                                                                                                       \
        if (InHandle == nullptr)                                                                                       \
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
            NativeResult _DeadNativeResult;                                                                            \
            Result _DeadResult;                                                                                        \
            _DeadNativeResult.ResultCode = EOS_EResult::EOS_NoConnection;                                              \
            MapResult(_DeadResult, _DeadNativeResult, *MakeShared<NativeConverter>());                                 \
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
        NativeResult _NativeResult;                                                                                    \
        _NativeResult.ResultCode = NativeFunction(InHandle, &_NativeOptions);                                          \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("SyncAsAsync call completed."), _NativeResult.ResultCode)                \
                 .ToLogString());                                                                                      \
                                                                                                                       \
        Result _Result;                                                                                                \
        MapResult(_Result, _NativeResult, *MakeShared<NativeConverter>());                                             \
        InCompletionDelegate.ExecuteIfBound(_Result);                                                                  \
    }

REDPOINT_EOS_CODE_GUARD_END()