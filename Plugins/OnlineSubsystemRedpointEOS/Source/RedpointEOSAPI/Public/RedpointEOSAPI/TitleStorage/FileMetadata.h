// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::TitleStorage
{

class REDPOINTEOSAPI_API FFileMetadata
{
public:
    uint32 FileSizeBytes;
    FString MD5Hash;
    FString Filename;
    uint32 UnencryptedDataSizeBytes;
};

}

REDPOINT_EOS_CODE_GUARD_END()