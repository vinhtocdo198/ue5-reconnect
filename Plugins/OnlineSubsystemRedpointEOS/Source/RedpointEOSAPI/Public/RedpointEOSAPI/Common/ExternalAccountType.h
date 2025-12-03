// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Common
{

class FExternalAccountTypeMap
{
private:
    TMap<FString, EOS_EExternalAccountType> ToValueMap;
    TMap<EOS_EExternalAccountType, FString> FromValueMap;

public:
    FExternalAccountTypeMap();
    FExternalAccountTypeMap(const FExternalAccountTypeMap &) = delete;
    FExternalAccountTypeMap(FExternalAccountTypeMap &&) = delete;
    ~FExternalAccountTypeMap() = default;

    REDPOINTEOSAPI_API EOS_EExternalAccountType Get(const FString &InValue) const;
    REDPOINTEOSAPI_API FString Get(const EOS_EExternalAccountType &InValue) const;
};

REDPOINTEOSAPI_API extern FExternalAccountTypeMap ExternalAccountTypeMap;

}

REDPOINT_EOS_CODE_GUARD_END()