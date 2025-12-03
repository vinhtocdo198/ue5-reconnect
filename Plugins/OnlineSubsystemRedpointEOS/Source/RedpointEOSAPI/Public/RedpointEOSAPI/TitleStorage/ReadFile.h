// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncReadStream.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::TitleStorage
{

class REDPOINTEOSAPI_API FReadFile
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_READ_STREAM_BEGIN(TitleStorage, ReadFile, EOS_TITLESTORAGE_READFILE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> Filename;
        ParamHelpers::TRequired<uint32_t> ReadChunkLengthBytes;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    class ReadResult
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString Filename;
        uint32 TotalFileSizeBytes;
        bool bIsLastChunk;
        TArray<uint8> DataChunk;
    };

    class ProgressResult
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString Filename;
        uint32 BytesTransferred;
        uint32 TotalFileSizeBytes;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_READ_STREAM_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()