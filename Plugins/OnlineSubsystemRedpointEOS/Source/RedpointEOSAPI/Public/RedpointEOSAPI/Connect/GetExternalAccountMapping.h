// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncResult.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FGetExternalAccountMapping
{
    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_BEGIN(
        Connect,
        GetExternalAccountMapping,
        EOS_CONNECT_GETEXTERNALACCOUNTMAPPING_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<EOS_EExternalAccountType> AccountIdType;
        ParamHelpers::TRequired<FString> TargetExternalAccountId;
    };

    typedef EOS_ProductUserId Result;

    REDPOINT_EOSSDK_API_CALL_SYNC_RESULT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()