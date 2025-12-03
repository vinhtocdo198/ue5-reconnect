// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

namespace REDPOINT_EOS_FILE_NS_ID(1124880738, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS::Config;

class FSDKGlobal
{
private:
    static bool bInitialized;
    static bool bShutdown;

public:
    /**
     * Attempts to initialize the global parts of the EOS SDK. If the EOS SDK can't be initialized, this returns false.
     */
    static bool Initialize(IConfig &InConfig);

    /**
     * Shuts down the EOS SDK globally. Once this is called, the SDK can't be initialized again and no platform calls
     * will succeed.
     */
    static void Shutdown();
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(1124880738, Redpoint::EOS::Core, FSDKGlobal)
}