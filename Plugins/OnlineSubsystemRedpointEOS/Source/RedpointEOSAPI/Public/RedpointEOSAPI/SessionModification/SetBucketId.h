// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/ObjectSync.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::SessionModification
{

class REDPOINTEOSAPI_API FSetBucketId
{
    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_BEGIN(
        SessionModification,
        SetBucketId,
        EOS_SESSIONMODIFICATION_SETBUCKETID_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<FString> BucketId;
    };

    REDPOINT_EOSSDK_API_CALL_OBJECT_SYNC_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()