// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

class REDPOINTEOSAPI_API FJoinRoom
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(RTC, JoinRoom, EOS_RTC_JOINROOM_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<FString> RoomName;
        ParamHelpers::TRequired<FString> ClientBaseUrl;
        ParamHelpers::TRequired<FString> ParticipantToken;
        ParamHelpers::TRequired<EOS_ProductUserId> ParticipantId;
        ParamHelpers::TRequired<uint32> Flags;
        ParamHelpers::TRequired<bool> bManualAudioInputEnabled;
        ParamHelpers::TRequired<bool> bManualAudioOutputEnabled;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
        FString RoomName;
        TMap<FString, FString> RoomOptions;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()