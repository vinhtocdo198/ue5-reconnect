// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Platform.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/InstancePool.h"
#include "RedpointEOSPlatform/LifecycleManager.h"
#include "RedpointEOSPlatform/RuntimePlatform.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(129125291, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS::Platform;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Config;

class FInstancePoolImpl : public IInstancePool, public ILifecycleManager, public TSharedFromThis<FInstancePoolImpl>
{
private:
    FDelegateHandle TickerHandle;
    TMap<FName, FPlatformWeakRefCountedHandle> Instances;
    bool bIsInShutdown;
    TSharedPtr<IRuntimePlatform> RuntimePlatform;
    bool bShouldPollForApplicationStatus;
    bool bShouldPollForNetworkStatus;

    bool Tick(float DeltaSeconds);

public:
    FInstancePoolImpl();
    UE_NONCOPYABLE(FInstancePoolImpl);
    virtual ~FInstancePoolImpl() override;

    void ShutdownAll();

    virtual TSharedPtr<FPlatformInstance> TryGetExisting(FName InstanceName) const override;

    virtual FPlatformRefCountedHandle Create(FName InstanceName) override;
    virtual FPlatformRefCountedHandle CreateWithConfig(
        FName InstanceName,
        TSharedRef<IConfig> Config,
        bool bOverrideIsDedicatedServer = false) override;

#if REDPOINT_EOS_UE_5_5_OR_LATER && WITH_EDITOR
    virtual FPlatformRefCountedHandle CreateWithPlatform(FName InstanceName, EOS_Platform_Options &PlatformOptions)
        override;
#endif

    virtual void UpdateApplicationStatus(const EOS_EApplicationStatus &InNewStatus) override;
    virtual void UpdateNetworkStatus(const EOS_ENetworkStatus &InNewStatus) override;
};

}

namespace Redpoint::EOS::Core
{
REDPOINT_EOS_FILE_NS_EXPORT(129125291, Redpoint::EOS::Core, FInstancePoolImpl)
}

REDPOINT_EOS_CODE_GUARD_END()