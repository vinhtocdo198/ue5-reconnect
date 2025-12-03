// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

class REDPOINTEOSAPI_API FDeleteFile
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(PlayerDataStorage, DeleteFile, EOS_PLAYERDATASTORAGE_DELETEFILE_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> Filename;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()