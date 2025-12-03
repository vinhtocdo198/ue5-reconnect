// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/RTCAudio/AudioEnabled.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FUpdateReceiving
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(RTCAudio, UpdateReceiving, EOS_RTCAUDIO_UPDATERECEIVING_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> RoomName;
        ParamHelpers::TRequired<EOS_ProductUserId> ParticipantId;
        ParamHelpers::TRequired<EAudioEnabled> bAudioEnabled;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
        FString RoomName;
        EOS_ProductUserId ParticipantId;
        EAudioEnabled bAudioEnabled;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()