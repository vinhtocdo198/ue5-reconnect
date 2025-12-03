// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/PlayerDataStorage/FileMetadata.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListCopyErrorableCount.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::PlayerDataStorage
{

class REDPOINTEOSAPI_API FQueryFileList
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_BEGIN(
        PlayerDataStorage,
        QueryFileList,
        EOS_PLAYERDATASTORAGE_QUERYFILELIST_API_LATEST,
        GetFileMetadataCount,
        EOS_PLAYERDATASTORAGE_GETFILEMETADATACOUNT_API_LATEST,
        CopyFileMetadataAtIndex,
        EOS_PLAYERDATASTORAGE_COPYFILEMETADATAATINDEX_API_LATEST,
        FileMetadata)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
    };

    using ResultEntry = FFileMetadata;

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_ERRORABLE_COUNT_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()