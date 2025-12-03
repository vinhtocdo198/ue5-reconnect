// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Event.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

class REDPOINTEOSAPI_API FNotifyParticipantStatusChanged
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(
        RTC,
        ParticipantStatusChanged,
        EOS_RTC_ADDNOTIFYPARTICIPANTSTATUSCHANGED_API_LATEST)

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
        EOS_ERTCParticipantStatus ParticipantStatus;
        TMap<FString, FString> ParticipantMetadata;
        bool bParticipantInBlocklist;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()