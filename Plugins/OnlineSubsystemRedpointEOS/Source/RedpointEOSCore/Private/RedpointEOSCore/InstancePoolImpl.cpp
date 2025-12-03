// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSCore/InstancePoolImpl.h"

#include "Engine/Engine.h"
#include "RedpointEOSAPI/Memory.h"
#include "RedpointEOSAPI/Platform/Tick.h"
#include "RedpointEOSConfig/IniConfig.h"
#include "RedpointEOSCore/InstanceFactory.h"
#include "RedpointEOSCore/Logging.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSCore/SDKGlobal.h"
#include "RedpointEOSCore/Utils/RegulatedTicker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(129125291, Redpoint::EOS::Core)
{
using namespace ::Redpoint::EOS;
using namespace ::Redpoint::EOS::Core;

FInstancePoolImpl::FInstancePoolImpl()
    : TickerHandle(Utils::FRegulatedTicker::GetCoreTicker().AddTicker(
          FTickerDelegate::CreateRaw(this, &FInstancePoolImpl::Tick)))
    , Instances()
    , bIsInShutdown(false)
    , RuntimePlatform()
    , bShouldPollForApplicationStatus(false)
    , bShouldPollForNetworkStatus(false)
{
}

FInstancePoolImpl::~FInstancePoolImpl()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->ShutdownAll();
    Utils::FRegulatedTicker::GetCoreTicker().RemoveTicker(this->TickerHandle);
    this->TickerHandle.Reset();
}

void FInstancePoolImpl::ShutdownAll()
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    this->bIsInShutdown = true;
    for (const auto &KV : this->Instances)
    {
        auto Ptr = KV.Value.Pin();
        if (Ptr.IsValid())
        {
            if (!Ptr->Instance()->IsShutdown())
            {
                Ptr->Instance()->ForceShutdown();
            }
        }
    }
    this->Instances.Empty();
}

bool FInstancePoolImpl::Tick(float DeltaSeconds)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    using namespace ::Redpoint::EOS::API::Platform;

    this->OnBeforeAnyTick().Broadcast();

    for (const auto &KV : this->Instances)
    {
        auto Ptr = KV.Value.Pin();
        if (Ptr.IsValid())
        {
            if (!Ptr->Instance()->IsShutdown())
            {
                Ptr->Instance()->OnBeforeTick().Broadcast(Ptr->Instance().Get());
                FTick::Execute(Ptr->Instance());
                Ptr->Instance()->OnAfterTick().Broadcast(Ptr->Instance().Get());
            }
        }
    }

    {
        EOS_EApplicationStatus NewApplicationStatus = EOS_EApplicationStatus::EOS_AS_Foreground;
        EOS_ENetworkStatus NewNetworkStatus = EOS_ENetworkStatus::EOS_NS_Online;
        bool bShouldSetNewApplicationStatus = false;
        bool bShouldSetNewNetworkStatus = false;

        if (this->bShouldPollForApplicationStatus && this->bShouldPollForNetworkStatus)
        {
            bShouldSetNewApplicationStatus =
                this->RuntimePlatform->PollLifecycleApplicationStatus(NewApplicationStatus);
            bShouldSetNewNetworkStatus = this->RuntimePlatform->PollLifecycleNetworkStatus(NewNetworkStatus);
        }
        else if (this->bShouldPollForApplicationStatus)
        {
            bShouldSetNewApplicationStatus =
                this->RuntimePlatform->PollLifecycleApplicationStatus(NewApplicationStatus);
        }
        else if (this->bShouldPollForNetworkStatus)
        {
            bShouldSetNewNetworkStatus = this->RuntimePlatform->PollLifecycleNetworkStatus(NewNetworkStatus);
        }

        if (bShouldSetNewApplicationStatus || bShouldSetNewNetworkStatus)
        {
            for (const auto &KV : this->Instances)
            {
                auto Ptr = KV.Value.Pin();
                if (Ptr.IsValid())
                {
                    if (!Ptr->Instance()->IsShutdown())
                    {
                        EOS_HPlatform Platform = Ptr->Instance()->Handle();
                        if (bShouldSetNewApplicationStatus)
                        {
                            EOS_EApplicationStatus OldApplicationStatus = EOS_Platform_GetApplicationStatus(Platform);
                            if (OldApplicationStatus != NewApplicationStatus)
                            {
                                UE_LOG(
                                    LogRedpointEOSCore,
                                    Verbose,
                                    TEXT("FInstancePoolImpl::Tick: Platform instance %p application status moving "
                                         "from [%d] to [%d]"),
                                    Platform,
                                    OldApplicationStatus,
                                    NewApplicationStatus);
                                EOS_Platform_SetApplicationStatus(Platform, NewApplicationStatus);
                                Ptr->Instance()->OnApplicationStatusChanged().Broadcast(
                                    Ptr->Instance().Get(),
                                    OldApplicationStatus,
                                    NewApplicationStatus);
                            }
                        }
                        if (bShouldSetNewNetworkStatus)
                        {
                            auto OldNetworkStatus = EOS_Platform_GetNetworkStatus(Platform);
                            if (OldNetworkStatus != NewNetworkStatus)
                            {
                                EOS_Platform_SetNetworkStatus(Platform, NewNetworkStatus);
                                Ptr->Instance()->OnNetworkStatusChanged().Broadcast(
                                    Ptr->Instance().Get(),
                                    OldNetworkStatus,
                                    NewNetworkStatus);
                            }
                        }
                    }
                }
            }
        }
    }

    this->OnAfterAnyTick().Broadcast();

    return true;
}

TSharedPtr<FPlatformInstance> FInstancePoolImpl::TryGetExisting(FName InstanceName) const
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto ExistingEntry = this->Instances.Find(InstanceName);
    if (ExistingEntry != nullptr)
    {
        auto Ptr = ExistingEntry->Pin();
        if (Ptr.IsValid())
        {
            return Ptr->Instance();
        }
    }
    return nullptr;
}

API::FPlatformRefCountedHandle FInstancePoolImpl::Create(FName InstanceName)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    return this->CreateWithConfig(InstanceName, MakeShared<Config::FIniConfig>(), false);
}

API::FPlatformRefCountedHandle FInstancePoolImpl::CreateWithConfig(
    FName InstanceName,
    TSharedRef<Config::IConfig> Config,
    bool bOverrideIsDedicatedServer)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto PlatformInstanceEnvironment = MakeShared<API::FPlatformInstanceEnvironment>(
        InstanceName,
        bOverrideIsDedicatedServer ? true : FInstanceFactory::IsTrueDedicated(InstanceName),
        Config);

    if (this->bIsInShutdown)
    {
        UE_LOG(LogRedpointEOSCore, Warning, TEXT("Ignoring request to create platform instance during shutdown."));
        return API::MakeRefCountedPlatformHandle(
            API::FPlatformInstance::CreateDeadWithNoInstance(PlatformInstanceEnvironment));
    }

    if (!FSDKGlobal::Initialize(*Config))
    {
        // The EOS SDK could not be initialized globally, return a dead instance.
        return API::MakeRefCountedPlatformHandle(
            API::FPlatformInstance::CreateDeadWithNoInstance(PlatformInstanceEnvironment));
    }

    // Initialise the lifecycle manager logic if needed.
    if (!this->RuntimePlatform.IsValid())
    {
        this->RuntimePlatform = FModule::GetModuleChecked().GetRuntimePlatform();
        this->RuntimePlatform->RegisterLifecycleHandlers(this->AsShared());
        this->bShouldPollForApplicationStatus = this->RuntimePlatform->ShouldPollLifecycleApplicationStatus();
        this->bShouldPollForNetworkStatus = this->RuntimePlatform->ShouldPollLifecycleNetworkStatus();
    }

    auto ExistingEntry = this->Instances.Find(InstanceName);
    if (ExistingEntry != nullptr)
    {
        auto Ptr = ExistingEntry->Pin();
        if (Ptr.IsValid())
        {
            return Ptr.ToSharedRef();
        }
    }

    API::FPlatformHandle Instance = FInstanceFactory::Create(PlatformInstanceEnvironment);
    API::FPlatformRefCountedHandle RefHandle = API::MakeRefCountedPlatformHandle(Instance);
    this->Instances.Emplace(InstanceName, RefHandle);
    return RefHandle;
}

#if REDPOINT_EOS_UE_5_5_OR_LATER && WITH_EDITOR

// @note: This just exists to support Fab, so the platform handle isn't really set up properly - it's just enough to get
// a handle and tick it.
API::FPlatformRefCountedHandle FInstancePoolImpl::CreateWithPlatform(
    FName InstanceName,
    EOS_Platform_Options &PlatformOptions)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    auto Config = MakeShared<Config::FIniConfig>();

    auto PlatformInstanceEnvironment = MakeShared<API::FPlatformInstanceEnvironment>(InstanceName, false, Config);

    if (this->bIsInShutdown)
    {
        UE_LOG(LogRedpointEOSCore, Warning, TEXT("Ignoring request to create platform instance during shutdown."));
        return API::MakeRefCountedPlatformHandle(
            API::FPlatformInstance::CreateDeadWithNoInstance(PlatformInstanceEnvironment));
    }

    if (!FSDKGlobal::Initialize(*Config))
    {
        // The EOS SDK could not be initialized globally, return a dead instance.
        return API::MakeRefCountedPlatformHandle(
            API::FPlatformInstance::CreateDeadWithNoInstance(PlatformInstanceEnvironment));
    }

    // Initialise the lifecycle manager logic if needed.
    if (!this->RuntimePlatform.IsValid())
    {
        this->RuntimePlatform = FModule::GetModuleChecked().GetRuntimePlatform();
        this->RuntimePlatform->RegisterLifecycleHandlers(this->AsShared());
        this->bShouldPollForApplicationStatus = this->RuntimePlatform->ShouldPollLifecycleApplicationStatus();
        this->bShouldPollForNetworkStatus = this->RuntimePlatform->ShouldPollLifecycleNetworkStatus();
    }

    checkf(
        this->Instances.Find(InstanceName) == nullptr,
        TEXT("Unexpected use of CreateWithPlatform for non-Fab scenario!"));

    API::FPlatformHandle Instance =
        FInstanceFactory::CreateFromRawOptions(PlatformInstanceEnvironment, PlatformOptions);
    API::FPlatformRefCountedHandle RefHandle = API::MakeRefCountedPlatformHandle(Instance);
    this->Instances.Emplace(InstanceName, RefHandle);
    return RefHandle;
}

#endif

void FInstancePoolImpl::UpdateApplicationStatus(const EOS_EApplicationStatus &InNewStatus)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &KV : this->Instances)
    {
        auto Ptr = KV.Value.Pin();
        if (Ptr.IsValid())
        {
            if (!Ptr->Instance()->IsShutdown())
            {
                EOS_HPlatform Platform = Ptr->Instance()->Handle();
                EOS_EApplicationStatus OldApplicationStatus = EOS_Platform_GetApplicationStatus(Platform);
                UE_LOG(
                    LogRedpointEOSCore,
                    Verbose,
                    TEXT("FInstancePoolImpl::UpdateApplicationStatus: Platform instance %p application status moving "
                         "from [%d] to [%d]"),
                    Platform,
                    OldApplicationStatus,
                    InNewStatus);
                EOS_Platform_SetApplicationStatus(Platform, InNewStatus);
            }
        }
    }
}

void FInstancePoolImpl::UpdateNetworkStatus(const EOS_ENetworkStatus &InNewStatus)
{
    REDPOINT_EOS_USE_LLM_TAG(Core);

    for (const auto &KV : this->Instances)
    {
        auto Ptr = KV.Value.Pin();
        if (Ptr.IsValid())
        {
            if (!Ptr->Instance()->IsShutdown())
            {
                EOS_Platform_SetNetworkStatus(Ptr->Instance()->Handle(), InNewStatus);
            }
        }
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()