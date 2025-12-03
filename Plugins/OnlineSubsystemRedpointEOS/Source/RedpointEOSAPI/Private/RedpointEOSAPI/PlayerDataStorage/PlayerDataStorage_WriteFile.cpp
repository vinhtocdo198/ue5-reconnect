// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/PlayerDataStorage/WriteFile.h"

#include "RedpointEOSAPI/Templates/Impl/AsyncWriteStream.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_IMPL(WriteFile);

void FWriteFile::MapOptions(
    NativeOptions &NativeOptions,
    const Options &Options,
    NativeAllocator &Allocator,
    const NativeDataCallback &NativeDataCallback,
    const NativeProgressCallback &NativeProgressCallback)
{
    NativeOptions.LocalUserId = *Options.LocalUserId;
    NativeOptions.Filename = Allocator.AsUtf8(*Options.Filename);
    NativeOptions.ChunkLengthBytes = *Options.ChunkLengthBytes;
    NativeOptions.WriteFileDataCallback = NativeDataCallback;
    NativeOptions.FileTransferProgressCallback = NativeProgressCallback;
}

void FWriteFile::MapResult(Result &Result, const NativeResult &NativeResult, NativeConverter &Converter)
{
    Result.ResultCode = NativeResult.ResultCode;
}

void FWriteFile::MapWriteResult(
    WriteResult &WriteResult,
    const NativeWriteResult &NativeWriteResult,
    NativeConverter &Converter)
{
    WriteResult.LocalUserId = NativeWriteResult.LocalUserId;
    WriteResult.Filename = Converter.FromUtf8(NativeWriteResult.Filename);
    WriteResult.DataBufferLengthBytes = NativeWriteResult.DataBufferLengthBytes;
}

void FWriteFile::MapProgressResult(
    ProgressResult &ProgressResult,
    const NativeProgressResult &NativeProgressResult,
    NativeConverter &Converter)
{
    ProgressResult.LocalUserId = NativeProgressResult.LocalUserId;
    ProgressResult.Filename = Converter.FromUtf8(NativeProgressResult.Filename);
    ProgressResult.BytesTransferred = NativeProgressResult.BytesTransferred;
    ProgressResult.TotalFileSizeBytes = NativeProgressResult.TotalFileSizeBytes;
}

FWriteFile::NativeWriteResultEnum FWriteFile::MapWriteResultEnum(EWriteResult WriteResultEnum)
{
    switch (WriteResultEnum)
    {
    case EWriteResult::ContinueWriting:
        return EOS_PlayerDataStorage_EWriteResult::EOS_WR_ContinueWriting;
    case EWriteResult::CompleteRequest:
        return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CompleteRequest;
    case EWriteResult::FailRequest:
        return EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest;
    case EWriteResult::CancelRequest:
        return EOS_PlayerDataStorage_EWriteResult::EOS_WR_CancelRequest;
    default:
        return EOS_PlayerDataStorage_EWriteResult::EOS_WR_FailRequest;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()