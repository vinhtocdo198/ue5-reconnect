// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::RTCAudio
{

class REDPOINTEOSAPI_API FDeviceInformation
{
public:
    FString Id;
    FString DisplayName;
    bool bIsDefaultDevice;
    bool bIsInputDevice;
};

}

REDPOINT_EOS_CODE_GUARD_END()