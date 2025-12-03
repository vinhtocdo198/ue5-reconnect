// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/SyncCopyNoAlloc.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Platform
{

class REDPOINTEOSAPI_API FGetDesktopCrossplayStatus
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_NOALLOC_BEGIN(
        Platform,
        GetDesktopCrossplayStatus,
        EOS_PLATFORM_GETDESKTOPCROSSPLAYSTATUS_API_LATEST,
        DesktopCrossplayStatusInfo)

    class Options
    {
    };

    class Result
    {
    public:
        EOS_EDesktopCrossplayStatus Status;
        int32 ServiceInitResult;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_NOALLOC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()