// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FCopyIdToken
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_BEGIN(
        Connect,
        CopyIdToken,
        EOS_CONNECT_COPYIDTOKEN_API_LATEST,
        EOS_Connect_IdToken)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    class Result
    {
    public:
        FString JsonWebToken;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()