// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/ReadFile.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncReadStream.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_READ_STREAM_IMPL(ReadFile);

void FReadFile::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator,
    const NativeDataCallback &NativeDataCallback,
    const NativeProgressCallback &NativeProgressCallback)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Filename = Allocator.AsUtf8(*Options.Filename);
    NativeOptions.ReadChunkLengthBytes = *Options.ReadChunkLengthBytes;
    NativeOptions.ReadFileDataCallback = NativeDataCallback;
    NativeOptions.FileTransferProgressCallback = NativeProgressCallback;
}

void FReadFile::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

void FReadFile::MapReadResult(
    ReadResult &ReadResult,
    const NativeReadResult &NativeReadResult,
    NativeConverter &Converter)
{
    ReadResult.LocalUserId = NativeReadResult.LocalUserId;
    ReadResult.Filename = Converter.FromUtf8(NativeReadResult.Filename);
    ReadResult.bIsLastChunk = NativeReadResult.bIsLastChunk == EOS_TRUE;
    ReadResult.TotalFileSizeBytes = NativeReadResult.TotalFileSizeBytes;
    ReadResult.DataChunk = TArray<uint8>(
        reinterpret_cast<const uint8 *>(NativeReadResult.DataChunk),
        static_cast<int32>(NativeReadResult.DataChunkLengthBytes));
}

void FReadFile::MapProgressResult(
    ProgressResult &ProgressResult,
    const NativeProgressResult &NativeProgressResult,
    NativeConverter &Converter)
{
    ProgressResult.LocalUserId = NativeProgressResult.LocalUserId;
    ProgressResult.Filename = Converter.FromUtf8(NativeProgressResult.Filename);
    ProgressResult.BytesTransferred = NativeProgressResult.BytesTransferred;
    ProgressResult.TotalFileSizeBytes = NativeProgressResult.TotalFileSizeBytes;
}

FReadFile::NativeReadResultEnum FReadFile::MapReadResultEnum(EReadResult ReadResultEnum)
{
    switch (ReadResultEnum)
    {
    case EReadResult::ContinueReading:
        return EOS_PlayerDataStorage_EReadResult::EOS_RR_ContinueReading;
    case EReadResult::FailRequest:
        return EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest;
    case EReadResult::CancelRequest:
        return EOS_PlayerDataStorage_EReadResult::EOS_RR_CancelRequest;
    default:
        return EOS_PlayerDataStorage_EReadResult::EOS_RR_FailRequest;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()