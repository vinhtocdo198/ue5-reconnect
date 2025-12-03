// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Releasable.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectAsyncListReleasable.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionSearch
{

class REDPOINTEOSAPI_API FFind
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_BEGIN(
        SessionSearch,
        Find,
        EOS_SESSIONSEARCH_FIND_API_LATEST,
        GetSearchResultCount,
        EOS_SESSIONSEARCH_GETSEARCHRESULTCOUNT_API_LATEST,
        CopySearchResultByIndex,
        EOS_SESSIONSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST,
        SessionDetails)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    typedef TSharedRef<TReleasable<EOS_HSessionDetails>> ResultEntry;

    REDPOINT_EOSSDK_API_CALL_OBJECT_ASYNC_LIST_RELEASABLE_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()