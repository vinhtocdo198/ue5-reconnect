// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API::Common
{

enum class EOnlinePlatformType : uint32
{
    Unknown = 0,
    Epic = 100,
    PSN = 1000,
    Switch = 2000,
    XBL = 3000,
    Steam = 4000,
};

extern REDPOINTEOSAPI_API const TSet<EOnlinePlatformType> OnlinePlatformTypes_AllowedPlatforms_All;
extern REDPOINTEOSAPI_API const TSet<EOnlinePlatformType> OnlinePlatformTypes_AllowedPlatforms_Console;

REDPOINTEOSAPI_API FString OnlinePlatformTypeToString(EOnlinePlatformType InType);

}

REDPOINT_EOS_CODE_GUARD_END()