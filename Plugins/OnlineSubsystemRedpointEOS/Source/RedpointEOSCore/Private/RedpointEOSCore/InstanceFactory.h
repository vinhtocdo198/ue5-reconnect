// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2644256279, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS::API;

class FInstanceFactory
{
public:
    static bool IsTrueDedicated(const FName &InInstanceName);
    static FPlatformHandle Create(const FPlatformInstanceEnvironmentRef &InEnvironment);

#if REDPOINT_EOS_UE_5_5_OR_LATER && WITH_EDITOR
    static FPlatformHandle CreateFromRawOptions(
        const FPlatformInstanceEnvironmentRef &InEnvironment,
        EOS_Platform_Options &PlatformOptions);
#endif
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(2644256279, Redpoint::EOS::Core, FInstanceFactory)
}

REDPOINT_EOS_CODE_GUARD_END()