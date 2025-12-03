// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSAPI/Templates/Util/Cdecl.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::NativeCall
{

template <typename S> struct TAsyncWriteStreamWriteCallbackInfo;
template <typename Options, typename ResultEnum>
struct TAsyncWriteStreamWriteCallbackInfo<ResultEnum(
    __REDPOINT_EOSSDK_CDECL_ATTR *)(const Options *, void *, uint32_t *)>
{
    using CallbackOptionsType = Options;
    using CallbackResultEnumType = ResultEnum;
};

template <typename S> struct TAsyncWriteStreamProgressCallbackInfo;
template <typename Options>
struct TAsyncWriteStreamProgressCallbackInfo<void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Options *)>
{
    using CallbackOptionsType = Options;
};

template <typename S> struct TAsyncWriteStreamInfo;
template <typename Handle, typename Options, typename Result, typename ReturnedHandle>
struct TAsyncWriteStreamInfo<
    ReturnedHandle(Handle, const Options *, void *, void(__REDPOINT_EOSSDK_CDECL_ATTR *)(const Result *))>
{
    using HandleType = Handle;
    using OptionsType = Options;
    using ResultType = Result;
    using ReturnType = ReturnedHandle;
    using WriteResultType =
        typename TAsyncWriteStreamWriteCallbackInfo<decltype(Options::WriteFileDataCallback)>::CallbackOptionsType;
    using WriteResultEnumType =
        typename TAsyncWriteStreamWriteCallbackInfo<decltype(Options::WriteFileDataCallback)>::CallbackResultEnumType;
    using ProgressResultType = typename TAsyncWriteStreamProgressCallbackInfo<
        decltype(Options::FileTransferProgressCallback)>::CallbackOptionsType;
    using NativeDataCallback = decltype(Options::WriteFileDataCallback);
    using NativeProgressCallback = decltype(Options::FileTransferProgressCallback);
};

}

REDPOINT_EOS_CODE_GUARD_END()