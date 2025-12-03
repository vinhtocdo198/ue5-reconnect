// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FCopyProductUserExternalAccountByAccountType
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_BEGIN(
        Connect,
        CopyProductUserExternalAccountByAccountType,
        EOS_CONNECT_COPYPRODUCTUSEREXTERNALACCOUNTBYACCOUNTTYPE_API_LATEST,
        EOS_Connect_ExternalAccountInfo)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
        ParamHelpers::TRequired<EOS_EExternalAccountType> AccountIdType;
    };

    class Result
    {
    public:
        EOS_ProductUserId ProductUserId;
        FString DisplayName;
        FString AccountId;
        EOS_EExternalAccountType AccountIdType;
        TOptional<int64> LastLoginTime;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()