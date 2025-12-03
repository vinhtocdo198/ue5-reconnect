// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Auth/PinGrantCode.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Async.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FLogin
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_BEGIN(Auth, Login, EOS_AUTH_LOGIN_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> CredentialId;
        ParamHelpers::TRequired<FString> CredentialToken;
        ParamHelpers::TRequired<EOS_ELoginCredentialType> CredentialType;
        ParamHelpers::TRequired<EOS_EExternalCredentialType> CredentialExternalType;
        ParamHelpers::TRequired<EOS_EAuthScopeFlags> ScopeFlags;
        ParamHelpers::TRequired<bool> bNoUserInterface;
    };

    class Result
    {
    public:
        EOS_EResult ResultCode;
        EOS_EpicAccountId LocalUserId;
        TOptional<FPinGrantInfo> PinGrantInfo;
        EOS_ContinuanceToken ContinuanceToken;
        EOS_EpicAccountId SelectedAccountId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()