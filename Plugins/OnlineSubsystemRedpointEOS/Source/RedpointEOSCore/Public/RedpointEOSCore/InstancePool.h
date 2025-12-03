// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(401947299, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS;

class REDPOINTEOSCORE_API IInstancePool
{
private:
    FSimpleMulticastDelegate OnBeforeAnyTickEvent;
    FSimpleMulticastDelegate OnAfterAnyTickEvent;

public:
    IInstancePool() = default;
    UE_NONCOPYABLE(IInstancePool);
    virtual ~IInstancePool() = default;

    static IInstancePool &Pool();

    virtual TSharedPtr<API::FPlatformInstance> TryGetExisting(FName InstanceName) const = 0;

    virtual API::FPlatformRefCountedHandle Create(FName InstanceName) = 0;
    virtual API::FPlatformRefCountedHandle CreateWithConfig(
        FName InstanceName,
        TSharedRef<Config::IConfig> Config,
        bool bOverrideIsDedicatedServer = false) = 0;

#if REDPOINT_EOS_UE_5_5_OR_LATER && WITH_EDITOR
    virtual API::FPlatformRefCountedHandle CreateWithPlatform(
        FName InstanceName,
        EOS_Platform_Options &PlatformOptions) = 0;
#endif

    FSimpleMulticastDelegate &OnBeforeAnyTick()
    {
        return this->OnBeforeAnyTickEvent;
    }

    FSimpleMulticastDelegate &OnAfterAnyTick()
    {
        return this->OnAfterAnyTickEvent;
    }
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(401947299, Redpoint::EOS::Core, IInstancePool)
}

REDPOINT_EOS_CODE_GUARD_END()