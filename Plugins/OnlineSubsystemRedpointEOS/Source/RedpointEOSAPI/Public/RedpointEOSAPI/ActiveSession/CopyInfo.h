// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Common/OnlinePlatformType.h"
#include "RedpointEOSAPI/SessionDetails/CopyInfo.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::ActiveSession
{

class REDPOINTEOSAPI_API FCopyInfo
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_BEGIN(
        ActiveSession,
        CopyInfo,
        EOS_ACTIVESESSION_COPYINFO_API_LATEST,
        Info)

    class Options
    {
    };

    class Result
    {
    public:
        FString SessionName;
        EOS_ProductUserId LocalUserId;
        EOS_EOnlineSessionState State;

        Redpoint::EOS::API::SessionDetails::FCopyInfo::Result SessionDetails;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()