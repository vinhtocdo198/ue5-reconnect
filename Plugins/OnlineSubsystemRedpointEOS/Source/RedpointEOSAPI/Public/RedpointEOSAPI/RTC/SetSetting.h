// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTC
{

class REDPOINTEOSAPI_API FSetSetting
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(RTC, SetSetting, EOS_RTC_SETSETTING_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> SettingName;
        ParamHelpers::TRequired<FString> SettingValue;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()