// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Sanctions
{

class REDPOINTEOSAPI_API FQueryActivePlayerSanctions
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_BEGIN(
        Sanctions,
        QueryActivePlayerSanctions,
        EOS_SANCTIONS_QUERYACTIVEPLAYERSANCTIONS_API_LATEST,
        GetPlayerSanctionCount,
        EOS_SANCTIONS_GETPLAYERSANCTIONCOUNT_API_LATEST,
        CopyPlayerSanctionByIndex,
        EOS_SANCTIONS_COPYPLAYERSANCTIONBYINDEX_API_LATEST,
        PlayerSanction)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> TargetUserId;
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    class ResultEntry
    {
    public:
        FDateTime TimePlaced;
        FString Action;
        TOptional<FDateTime> TimeExpires;
        FString ReferenceId;
    };

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()