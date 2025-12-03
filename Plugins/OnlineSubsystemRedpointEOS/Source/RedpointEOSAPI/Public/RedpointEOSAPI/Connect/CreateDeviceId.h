// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FCreateDeviceId
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Connect, CreateDeviceId, EOS_CONNECT_CREATEDEVICEID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> DeviceModel;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()