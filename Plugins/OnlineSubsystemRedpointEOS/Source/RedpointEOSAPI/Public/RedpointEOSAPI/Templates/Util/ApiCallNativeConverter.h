// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Templates::Util
{

class REDPOINTEOSAPI_API FApiCallNativeConverter
{
public:
    FApiCallNativeConverter() = default;
    FApiCallNativeConverter(const FApiCallNativeConverter &) = delete;
    FApiCallNativeConverter(FApiCallNativeConverter &&) = delete;
    ~FApiCallNativeConverter() = default;
    FString FromAnsi(const char *InString) const;
    FString FromUtf8(const char *InString) const;
};

}

REDPOINT_EOS_CODE_GUARD_END()