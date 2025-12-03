// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/SyncCopy.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Presence
{

class REDPOINTEOSAPI_API FCopyPresence
{
    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_BEGIN(
        Presence,
        CopyPresence,
        EOS_PRESENCE_COPYPRESENCE_API_LATEST,
        EOS_Presence_Info)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_EpicAccountId> LocalUserId;
        ParamHelpers::TRequired<EOS_EpicAccountId> TargetUserId;
    };

    class Result
    {
    public:
        EOS_Presence_EStatus Status;
        EOS_EpicAccountId UserId;
        FString ProductId;
        FString ProductVersion;
        FString Platform;
        FString RichText;
        TMap<FString, FString> DataRecords;
        FString ProductName;
        FString IntegratedPlatform;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()