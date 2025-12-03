// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/ObjectSyncCopy.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Logging.h"
#include "RedpointEOSAPI/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_IMPL(CallName)                                                       \
    REDPOINT_EOS_DEFINE_API_CALL_LLM_TAG(CallName);                                                     \
                                                                                                                       \
    void F##CallName::Execute(                                                                                         \
        TReleasableRef<NativeHandle> InHandle,                                                                         \
        const Options &InOptions,                                                                                      \
        EOS_EResult &OutResultCode,                                                                                    \
        Result &OutResult)                                                                                             \
    {                                                                                                                  \
        checkf(IsInGameThread(), TEXT("Calls to the EOS SDK can only be made on the game thread!"));                   \
                                                                                                                       \
        OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                              \
        OutResult = {};                                                                                                \
                                                                                                                       \
        DECLARE_CYCLE_STAT(StatCallName(), STAT_Call, STATGROUP_RedpointEOS);                                          \
        SCOPE_CYCLE_COUNTER(STAT_Call);                                                                                \
                                                                                                                       \
        UE_LOG(LogRedpointEOSAPI, VeryVerbose, TEXT("[starting] %s"), LogCallName());                                  \
                                                                                                                       \
        if (!InHandle->IsValid())                                                                                      \
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
            OutResult = {};                                                                                            \
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
        NativeResult *_NativeResult = nullptr;                                                                         \
        OutResultCode = NativeFunction(InHandle->Value(), &_NativeOptions, &_NativeResult);                            \
        if (_NativeResult != nullptr)                                                                                  \
        {                                                                                                              \
            MapResult(OutResult, *_NativeResult, *Converter);                                                          \
        }                                                                                                              \
        if (_NativeResult == nullptr && OutResultCode == EOS_EResult::EOS_Success)                                     \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                Warning,                                                                                               \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(                                                                                         \
                     LogCallName(),                                                                                    \
                     TEXT("Sync object call reported success but returned object was nullptr. Forcing result code to " \
                          "EOS_UnexpectedError."),                                                                     \
                     OutResultCode)                                                                                    \
                     .ToLogString());                                                                                  \
            OutResultCode = EOS_EResult::EOS_UnexpectedError;                                                          \
        }                                                                                                              \
        else                                                                                                           \
        {                                                                                                              \
            UE_LOG(                                                                                                    \
                LogRedpointEOSAPI,                                                                                     \
                VeryVerbose,                                                                                           \
                TEXT("[complete] %s"),                                                                                 \
                *ConvertError(LogCallName(), TEXT("Sync object copy call completed."), OutResultCode).ToLogString());  \
        }                                                                                                              \
        if (_NativeResult != nullptr)                                                                                  \
        {                                                                                                              \
            NativeReleaseFunction(_NativeResult);                                                                      \
        }                                                                                                              \
        return;                                                                                                        \
    }

REDPOINT_EOS_CODE_GUARD_END()