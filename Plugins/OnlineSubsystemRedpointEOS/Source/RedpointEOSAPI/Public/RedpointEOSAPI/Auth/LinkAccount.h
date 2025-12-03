// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Auth/PinGrantCode.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FLinkAccount
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Auth, LinkAccount, EOS_AUTH_LINKACCOUNT_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ContinuanceToken> ContinuanceToken;
        ParamHelpers::TRequired<EOS_ELinkAccountFlags> LinkAccountFlags;
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_EpicAccountId LocalUserId;
        TOptional<FPinGrantInfo> PinGrantInfo;
        EOS_EpicAccountId SelectedAccountId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()