// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FNotifyParticipantUpdated
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        RTCAudio,
        ParticipantUpdated,
        EOS_RTCAUDIO_ADDNOTIFYPARTICIPANTUPDATED_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> RoomName;
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        FString RoomName;
        EOS_ProductUserId ParticipantId;
        bool bSpeaking;
        EOS_ERTCAudioStatus AudioStatus;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()