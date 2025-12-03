// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/Templates/Decl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::UserInfo
{

class REDPOINTEOSAPI_API FGetLocalPlatformType
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_BEGIN(
        UserInfo,
        GetLocalPlatformType,
        EOS_USERINFO_GETLOCALPLATFORMTYPE_API_LATEST)

    class Options
    {
    };

    typedef Redpoint::EOS::API::Common::EOnlinePlatformType Result;

    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()