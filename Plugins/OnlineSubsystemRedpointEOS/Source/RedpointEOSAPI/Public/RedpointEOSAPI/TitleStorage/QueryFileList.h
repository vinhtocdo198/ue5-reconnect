// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/AsyncListCopy.h"
#include "RedpointEOSAPI/TitleStorage/FileMetadata.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::TitleStorage
{

class REDPOINTEOSAPI_API FQueryFileList
{
    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_BEGIN(
        TitleStorage,
        QueryFileList,
        EOS_TITLESTORAGE_QUERYFILELIST_API_LATEST,
        GetFileMetadataCount,
        EOS_TITLESTORAGE_GETFILEMETADATACOUNT_API_LATEST,
        CopyFileMetadataAtIndex,
        EOS_TITLESTORAGE_COPYFILEMETADATAATINDEX_API_LATEST,
        FileMetadata)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<TArray<FString>> ListOfTags;
    };

    using ResultEntry = FFileMetadata;

    REDPOINT_EOSSDK_API_CALL_ASYNC_LIST_COPY_END()
};

}

REDPOINT_EOS_CODE_GUARD_END()