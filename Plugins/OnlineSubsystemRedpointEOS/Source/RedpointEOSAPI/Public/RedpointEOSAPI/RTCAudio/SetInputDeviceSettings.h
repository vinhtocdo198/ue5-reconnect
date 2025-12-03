// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FSetInputDeviceSettings
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(
        RTCAudio,
        SetInputDeviceSettings,
        EOS_RTCAUDIO_SETINPUTDEVICESETTINGS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> DeviceId;
        ParamHelpers::TRequired<bool> bEnablePlatformAEC;
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