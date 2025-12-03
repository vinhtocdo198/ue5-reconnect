// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAPI/Templates/NativeCall/AsyncWriteStreamInfo.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeAllocator.h"
#include "RedpointEOSAPI/Templates/Util/ApiCallNativeConverter.h"
#include "RedpointEOSAPI/Templates/Util/Cdecl.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#define REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_BEGIN(CallNamespace, CallName, CallVersion)                        \
private:                                                                                                               \
    REDPOINT_EOS_DECLARE_API_CALL_LLM_TAG(CallNamespace, CallName);                                                    \
    typedef EOS_H##CallNamespace NativeHandle;                                                                         \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::OptionsType NativeOptions;                                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::ResultType NativeResult;                                          \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::ReturnType NativeReturn;                                          \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::WriteResultType NativeWriteResult;                                \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::WriteResultEnumType NativeWriteResultEnum;                        \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::ProgressResultType NativeProgressResult;                          \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::NativeDataCallback NativeDataCallback;                            \
    typedef ::Redpoint::EOS::API::Templates::NativeCall::TAsyncWriteStreamInfo<                                        \
        decltype(EOS_##CallNamespace##_##CallName)>::NativeProgressCallback NativeProgressCallback;                    \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeAllocator NativeAllocator;                            \
    typedef ::Redpoint::EOS::API::Templates::Util::FApiCallNativeConverter NativeConverter;                            \
    static FORCEINLINE int NativeOptionsVersion()                                                                      \
    {                                                                                                                  \
        return CallVersion;                                                                                            \
    }                                                                                                                  \
    static FORCEINLINE NativeReturn NativeFunction(                                                                    \
        NativeHandle Handle,                                                                                           \
        const NativeOptions *Options,                                                                                  \
        void *ClientData,                                                                                              \
        void(__REDPOINT_EOSSDK_CDECL_ATTR * Callback)(const NativeResult *Data))                                       \
    {                                                                                                                  \
        REDPOINT_EOS_USE_API_CALL_LLM_TAG();                                                                           \
        return EOS_##CallNamespace##_##CallName(Handle, Options, ClientData, Callback);                                \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallName()                                                                     \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName));                                                                    \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallWriteName()                                                                \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Write");                                                     \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallProgressName()                                                             \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Progress");                                                  \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *StatCallCompletionName()                                                           \
    {                                                                                                                  \
        return TEXT("RedpointEOS/API/EOS_") TEXT(PREPROCESSOR_TO_STRING(CallNamespace)) TEXT("_")                      \
            TEXT(PREPROCESSOR_TO_STRING(CallName)) TEXT("/Callback");                                                  \
    }                                                                                                                  \
    static FORCEINLINE const TCHAR *LogCallName()                                                                      \
    {                                                                                                                  \
        return TEXT(PREPROCESSOR_TO_STRING(EOS_##CallNamespace##_##CallName));                                         \
    }                                                                                                                  \
                                                                                                                       \
public:

#define REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_END()                                                              \
    enum class EWriteResult : uint8                                                                                    \
    {                                                                                                                  \
        ContinueWriting,                                                                                               \
        CompleteRequest,                                                                                               \
        FailRequest,                                                                                                   \
        CancelRequest,                                                                                                 \
    };                                                                                                                 \
    typedef TDelegate<void(const Result &InResult)> CompletionDelegate;                                                \
    typedef TDelegate<EWriteResult(const WriteResult &InWriteResult, TArray<uint8> &OutWrittenData)> WriteDelegate;    \
    typedef TDelegate<void(const ProgressResult &InProgressResult)> ProgressDelegate;                                  \
                                                                                                                       \
private:                                                                                                               \
    class FHeapState                                                                                                   \
    {                                                                                                                  \
    public:                                                                                                            \
        NativeOptions _NativeOptions = {};                                                                             \
        CompletionDelegate _CompletionDelegate;                                                                        \
        WriteDelegate _WriteDelegate;                                                                                  \
        ProgressDelegate _ProgressDelegate;                                                                            \
        TSharedPtr<NativeAllocator> _Allocator;                                                                        \
        bool _bSupportsAutomaticRetry;                                                                                 \
        TFunction<void(bool bTooManyRequests)> _ExecuteRetry;                                                          \
    };                                                                                                                 \
                                                                                                                       \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeCallback(const NativeResult *Data);                           \
    static NativeWriteResultEnum __REDPOINT_EOSSDK_CDECL_ATTR                                                          \
    HandleNativeWriteCallback(const NativeWriteResult *Data, void *OutDataBuffer, uint32_t *OutDataWritten);           \
    static void __REDPOINT_EOSSDK_CDECL_ATTR HandleNativeProgressCallback(const NativeProgressResult *Data);           \
                                                                                                                       \
public:                                                                                                                \
    static NativeReturn Execute(                                                                                       \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const WriteDelegate &InWriteDelegate,                                                                          \
        const ProgressDelegate &InProgressDelegate,                                                                    \
        const CompletionDelegate &InCompletionDelegate);                                                               \
                                                                                                                       \
private:                                                                                                               \
    static NativeReturn ExecuteWithCurrentAttemptCount(                                                                \
        const FPlatformHandle &InHandle,                                                                               \
        const Options &InOptions,                                                                                      \
        const WriteDelegate &InWriteDelegate,                                                                          \
        const ProgressDelegate &InProgressDelegate,                                                                    \
        const CompletionDelegate &InCompletionDelegate,                                                                \
        int CurrentAttemptCount);                                                                                      \
                                                                                                                       \
    static void MapOptions(                                                                                            \
        NativeOptions &NativeOptions,                                                                                  \
        const Options &Options,                                                                                        \
        NativeAllocator &Allocator,                                                                                    \
        const NativeDataCallback &NativeDataCallback,                                                                  \
        const NativeProgressCallback &NativeProgressCallback);                                                         \
    static void MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter);               \
    static void MapWriteResult(                                                                                        \
        WriteResult &WriteResult,                                                                                      \
        const NativeWriteResult &NativeWriteResult,                                                                    \
        NativeConverter &Converter);                                                                                   \
    static NativeWriteResultEnum MapWriteResultEnum(EWriteResult WriteResultEnum);                                     \
    static void MapProgressResult(                                                                                     \
        ProgressResult &ProgressResult,                                                                                \
        const NativeProgressResult &NativeProgressResult,                                                              \
        NativeConverter &Converter);

REDPOINT_EOS_CODE_GUARD_END()