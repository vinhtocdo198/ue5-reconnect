// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Reports
{

class REDPOINTEOSAPI_API FSendPlayerBehaviorReport
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(
        Reports,
        SendPlayerBehaviorReport,
        EOS_REPORTS_SENDPLAYERBEHAVIORREPORT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> ReporterUserId;
        ParamHelpers::TRequired<EOS_ProductUserId> ReportedUserId;
        ParamHelpers::TRequired<EOS_EPlayerReportsCategory> Category;
        ParamHelpers::TRequired<FString> Message;
        ParamHelpers::TRequired<FString> Context;
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