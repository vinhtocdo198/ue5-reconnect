// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncGetTwo.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_SYNC_GET_TWO_IMPL(CallName)                                                           \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    EOS_EResult F##CallName::Execute(                                                                                  \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        Result1 &OutResult1,                                                                                           \
        Result2 &OutResult2)                                                                                           \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        return Execute(InHandle->Get<NativeHandle>(), InOptions, OutResult1, OutResult2);                              \
    }                                                                                                                  \
    EOS_EResult F##CallName::Execute(                                                                                  \
        NativeHandle InHandle,                                                                                         \
        const Options &InOptions,                                                                                      \
        Result1 &OutResult1,                                                                                           \
        Result2 &OutResult2)                                                                                           \
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
            return EOS_EResult::EOS_NoConnection;                                                                      \
        }                                                                                                              \
                                                                                                                       \
        TSharedRef<NativeAllocator> Allocator = MakeShared<NativeAllocator>();                                         \
                                                                                                                       \
        NativeOptions _NativeOptions = {};                                                                             \
        _NativeOptions.ApiVersion = NativeOptionsVersion();                                                            \
        MapOptions(_NativeOptions, InOptions, *Allocator);                                                             \
                                                                                                                       \
        NativeResult1 _NativeResult1 = {};                                                                             \
        NativeResult2 _NativeResult2 = {};                                                                             \
                                                                                                                       \
        EOS_EResult ResultCode = NativeFunction(InHandle, &_NativeOptions, &_NativeResult1, &_NativeResult2);          \
                                                                                                                       \
        OutResult1 = {};                                                                                               \
        OutResult2 = {};                                                                                               \
        if (ResultCode == EOS_EResult::EOS_Success)                                                                    \
        {                                                                                                              \
            auto Converter = MakeShared<NativeConverter>();                                                            \
            MapResult1(OutResult1, _NativeResult1, *Converter);                                                        \
            MapResult2(OutResult2, _NativeResult2, *Converter);                                                        \
        }                                                                                                              \
                                                                                                                       \
        UE_LOG(                                                                                                        \
            LogRedpointEOSAPI,                                                                                         \
            VeryVerbose,                                                                                               \
            TEXT("[complete] %s"),                                                                                     \
            *ConvertError(LogCallName(), TEXT("Sync call completed."), ResultCode).ToLogString());                     \
        return ResultCode;                                                                                             \
    }

REDPOINT_EOS_CODE_GUARD_END()