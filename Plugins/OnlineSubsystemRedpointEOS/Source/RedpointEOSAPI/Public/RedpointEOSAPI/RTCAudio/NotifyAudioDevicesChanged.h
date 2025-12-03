// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FNotifyAudioDevicesChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        RTCAudio,
        AudioDevicesChanged,
        EOS_RTCAUDIO_ADDNOTIFYAUDIODEVICESCHANGED_API_LATEST)

    class Options
    {
    public:
    };

    class Result
    {
    public:
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()