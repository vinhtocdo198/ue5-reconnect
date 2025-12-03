// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Auth
{

class REDPOINTEOSAPI_API FCopyIdToken
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_BEGIN(Auth, CopyIdToken, EOS_AUTH_COPYIDTOKEN_API_LATEST, EOS_Auth_IdToken)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> AccountId;
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