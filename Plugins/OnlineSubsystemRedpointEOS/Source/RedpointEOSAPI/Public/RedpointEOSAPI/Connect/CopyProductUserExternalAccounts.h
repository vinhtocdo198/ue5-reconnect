// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Connect
{

class REDPOINTEOSAPI_API FCopyProductUserExternalAccounts
{
    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_COPY_BEGIN(
        Connect,
        GetProductUserExternalAccountCount,
        EOS_CONNECT_GETPRODUCTUSEREXTERNALACCOUNTCOUNT_API_LATEST,
        CopyProductUserExternalAccountByIndex,
        EOS_CONNECT_COPYPRODUCTUSEREXTERNALACCOUNTBYINDEX_API_LATEST,
        Connect,
        ExternalAccountInfo)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
    };

    class ResultEntry
    {
    public:
        EOS_ProductUserId ProductUserId;
        FString DisplayName;
        FString AccountId;
        EOS_EExternalAccountType AccountIdType;
        TOptional<int64> LastLoginTime;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()