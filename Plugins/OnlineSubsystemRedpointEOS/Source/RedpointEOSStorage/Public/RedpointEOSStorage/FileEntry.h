// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(352169454, Redpoint::EOS::Storage)
{

class REDPOINTEOSSTORAGE_API FFileEntry
{
public:
    uint32 FileSizeBytes;
    FString MD5Hash;
    FString FilePath;
    TOptional<FDateTime> LastModifiedTime;
    uint32 UnencryptedDataSizeBytes;
};

}

namespace Redpoint::EOS::Storage
{
REDPOINT_EOS_FILE_NS_EXPORT(352169454, Redpoint::EOS::Storage, FFileEntry)
}

REDPOINT_EOS_CODE_GUARD_END()