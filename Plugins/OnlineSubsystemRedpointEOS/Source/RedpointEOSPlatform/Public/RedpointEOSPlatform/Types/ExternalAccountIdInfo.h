// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2969602476, Redpoint::EOS::Platform::Types)
{

struct REDPOINTEOSPLATFORM_API FExternalAccountIdInfo
{
public:
    EOS_EExternalAccountType AccountIdType;
    FString AccountId;

    FExternalAccountIdInfo() = default;
    FExternalAccountIdInfo(const FExternalAccountIdInfo &) = default;
    FExternalAccountIdInfo(FExternalAccountIdInfo &&) = default;
    ~FExternalAccountIdInfo() = default;
};

}

namespace Redpoint::EOS::Platform::Types
{
REDPOINT_EOS_FILE_NS_EXPORT(2969602476, Redpoint::EOS::Platform::Types, FExternalAccountIdInfo)
}

REDPOINT_EOS_CODE_GUARD_END()