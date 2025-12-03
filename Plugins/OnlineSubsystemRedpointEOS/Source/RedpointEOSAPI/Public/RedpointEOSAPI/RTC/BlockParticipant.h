// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

class REDPOINTEOSAPI_API FBlockParticipant
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(RTC, BlockParticipant, EOS_RTC_BLOCKPARTICIPANT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> RoomName;
        ParamHelpers::TRequired<EOS_ProductUserId> ParticipantId;
        ParamHelpers::TRequired<bool> bBlocked;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
        FString RoomName;
        EOS_ProductUserId ParticipantId;
        bool bBlocked;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()