// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointEOSConfig/Config.h"

namespace Redpoint::EOS::Config
{

class REDPOINTEOSCONFIG_API FDependentModuleLoader
{
public:
    static void LoadConfigDependentModules(IConfig &InConfig);
    static void LoadPlatformDependentModules();
};

}