// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSConfig/Config.h"

namespace Redpoint::EOS::Config
{

class REDPOINTEOSCONFIG_API FEngineConfigHelpers
{
public:
    static FConfigFile *FindByName(const FString &InFilename);
};

}