// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncReleasable.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_RELEASABLE_IMPL(CallName)                                                        \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    TSharedRef<TReleasable<F##CallName::NativeResult>> F##CallName::Execute(                                           \
        FPlatformHandle InHandle,                                                                                      \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode)                                                                                    \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                              \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());                                  \
                                                                                                                       \
        if (InHandle->IsShutdown())                                                                                    \
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
            return MakeShareable(new TReleasable<NativeResult>(InHandle));                                             \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator);                                                             \
                                                                                                                       \
        NativeResult _NativeResult = {};                                                                               \
        OutResultCode = NativeFunction(InHandle->Get<NativeHandle>(), &_NativeOptions, &_NativeResult);                \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Sync copy call completed."), OutResultCode).ToLogString());             \
        if (OutResultCode != EOS_EResult::EOS_Success)                                                                 \
        {                                                                                                              \
            return MakeShareable(new TReleasable<NativeResult>(InHandle));                                             \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            return MakeShareable(new TReleasable<NativeResult>(                                                        \
                InHandle,                                                                                              \
                _NativeResult,                                                                                         \
                TDelegate<void(NativeResult Value)>::CreateStatic(&F##CallName::NativeReleaseFunction)));              \
        }                                                                                                              \
    }

REDPOINT_EOS_CODE_GUARD_END()