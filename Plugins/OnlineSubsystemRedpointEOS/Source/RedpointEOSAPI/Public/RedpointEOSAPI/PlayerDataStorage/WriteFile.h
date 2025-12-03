// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncWriteStream.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

class REDPOINTEOSAPI_API FWriteFile
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_BEGIN(
        PlayerDataStorage,
        WriteFile,
        EOS_PLAYERDATASTORAGE_WRITEFILE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> Filename;
        ParamHelpers::TRequired<uint32_t> ChunkLengthBytes;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    class WriteResult
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString Filename;
        uint32 DataBufferLengthBytes;
    };

    class ProgressResult
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString Filename;
        uint32 BytesTransferred;
        uint32 TotalFileSizeBytes;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_WRITE_STREAM_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()