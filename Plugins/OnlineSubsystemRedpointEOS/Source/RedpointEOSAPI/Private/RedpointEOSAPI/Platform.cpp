// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAPI/Platform.h"

#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/Platform/Release.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::API
{

FPlatformInstanceEnvironment::FPlatformInstanceEnvironment(
    const FName &InInstanceName,
    bool bInIsDedicatedServer,
    TSharedRef<Redpoint::EOS::Config::IConfig> InConfig)
    : InstanceName(InInstanceName)
    , bIsDedicatedServer(bInIsDedicatedServer)
    , Config(InConfig)
{
}

FPlatformInstance::FPlatformInstance(FPlatformInstanceEnvironmentRef InEnvironment)
    : bIsShutdown(true)
    , Instance(nullptr)
    , Environment(InEnvironment)
    , InstanceSystems()
    , InstanceSystemsLock()
    , OnBeforeTickEvent()
    , OnAfterTickEvent()
    , OnShutdownEvent()
{
}

TSharedRef<IPlatformInstanceSystem> FPlatformInstance::GetSystemInternal(
    FName InSystemName,
    TFunction<TSharedRef<IPlatformInstanceSystem>(
        FPlatformHandle InPlatformHandle,
        FPlatformInstanceEnvironmentRef InEnvironment)> InConstructSystem)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    // Optimistic check outside the lock.
    {
        TSharedPtr<IPlatformInstanceSystem> *ExistingSystem = this->InstanceSystems.Find(InSystemName);
        if (ExistingSystem != nullptr && ExistingSystem->IsValid())
        {
            return ExistingSystem->ToSharedRef();
        }
    }

    // Check and construct within the lock.
    {
        FScopeLock Lock(&this->InstanceSystemsLock);
        TSharedPtr<IPlatformInstanceSystem> *ExistingSystem = this->InstanceSystems.Find(InSystemName);
        if (ExistingSystem != nullptr && ExistingSystem->IsValid())
        {
            return ExistingSystem->ToSharedRef();
        }
        else
        {
            TSharedRef<IPlatformInstanceSystem> NewSystem = InConstructSystem(this->AsShared(), this->Environment);
            this->InstanceSystems.Add(InSystemName, NewSystem);
            return NewSystem;
        }
    }
}

FPlatformInstance::FPlatformInstance(EOS_HPlatform InInstance, FPlatformInstanceEnvironmentRef InEnvironment)
    : bIsShutdown(false)
    , Instance(InInstance)
    , Environment(InEnvironment)
    , InstanceSystems()
    , InstanceSystemsLock()
    , OnBeforeTickEvent()
    , OnAfterTickEvent()
    , OnShutdownEvent()
{
    checkf(Instance != nullptr, TEXT("EOS_HPlatform instance must not be null when instantiating PlatformInstance."));
}

FPlatformInstance::~FPlatformInstance()
{
}

FPlatformHandle FPlatformInstance::CreateDeadWithNoInstance(FPlatformInstanceEnvironmentRef InEnvironment)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    return MakeShareable(new FPlatformInstance(InEnvironment));
}

void FPlatformInstance::ForceShutdown()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    this->OnShutdown().Broadcast(*this);
    Platform::FRelease::Execute(this->AsShared());
    this->bIsShutdown = true;
    this->Instance = nullptr;
}

bool FPlatformInstance::IsShutdown() const
{
    return this->bIsShutdown;
}

Private::FPlatformRefCountedHandleInternal::FPlatformRefCountedHandleInternal(const FPlatformHandle &InInstance)
    : _Instance(InInstance)
{
}

Private::FPlatformRefCountedHandleInternal::~FPlatformRefCountedHandleInternal()
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    _Instance->ForceShutdown();
}

const FPlatformHandle &Private::FPlatformRefCountedHandleInternal::Instance() const
{
    return _Instance;
}

FPlatformRefCountedHandle MakeRefCountedPlatformHandle(const FPlatformHandle &Handle)
{
    REDPOINT_EOS_USE_LLM_TAG(API);

    return MakeShared<Private::FPlatformRefCountedHandleInternal>(Handle);
}

}

REDPOINT_EOS_CODE_GUARD_END()