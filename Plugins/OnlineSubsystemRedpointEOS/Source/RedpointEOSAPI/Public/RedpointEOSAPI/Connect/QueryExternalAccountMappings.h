// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FQueryExternalAccountMappings
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(
        Connect,
        QueryExternalAccountMappings,
        EOS_CONNECT_QUERYEXTERNALACCOUNTMAPPINGS_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<EOS_EExternalAccountType> AccountIdType;
        ParamHelpers::TRequired<TArray<FString>> ExternalAccountIds;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_ProductUserId LocalUserId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()