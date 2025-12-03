// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FUpdateSendingVolume
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(RTCAudio, UpdateSendingVolume, EOS_RTCAUDIO_UPDATESENDINGVOLUME_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> RoomName;
        ParamHelpers::TRequired<float> Volume;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
        float Volume;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()