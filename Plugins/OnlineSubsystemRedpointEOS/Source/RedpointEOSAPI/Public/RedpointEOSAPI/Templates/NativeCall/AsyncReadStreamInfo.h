// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/Templates/Util/Cdecl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncReadStreamReadCallbackInfo;
template <typename Options, typename ResultEnum>
struct TAsyncReadStreamReadCallbackInfo<ResultEnum(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Options *)>
{
    using CallbackOptionsType = Options;
    using CallbackResultEnumType = ResultEnum;
};

template <typename S> struct TAsyncReadStreamProgressCallbackInfo;
template <typename Options>
struct TAsyncReadStreamProgressCallbackInfo<void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Options *)>
{
    using CallbackOptionsType = Options;
};

template <typename S> struct TAsyncReadStreamInfo;
template <typename Handle, typename Options, typename Result, typename ReturnedHandle>
struct TAsyncReadStreamInfo<
    ReturnedHandle(Handle, const Options *, void *, void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Result *))>
{
    using HandleType = Handle;
    using OptionsType = Options;
    using ResultType = Result;
    using ReturnType = ReturnedHandle;
    using ReadResultType =
        typename TAsyncReadStreamReadCallbackInfo<decltype(Options::ReadFileDataCallback)>::CallbackOptionsType;
    using ReadResultEnumType =
        typename TAsyncReadStreamReadCallbackInfo<decltype(Options::ReadFileDataCallback)>::CallbackResultEnumType;
    using ProgressResultType = typename TAsyncReadStreamProgressCallbackInfo<
        decltype(Options::FileTransferProgressCallback)>::CallbackOptionsType;
    using NativeDataCallback = decltype(Options::ReadFileDataCallback);
    using NativeProgressCallback = decltype(Options::FileTransferProgressCallback);
};

}

REDPOINT_EOS_CODE_GUARD_END()