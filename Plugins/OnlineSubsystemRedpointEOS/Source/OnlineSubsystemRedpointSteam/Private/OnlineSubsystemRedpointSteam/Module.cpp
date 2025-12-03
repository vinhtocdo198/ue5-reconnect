// Copyright June Rhodes. All Rights Reserved.

#include "Modules/ModuleManager.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam
{

#if defined(REDPOINT_EOS_STEAM_ENABLED)

class FOnlineSubsystemRedpointSteamModule : public IModuleInterface, public IOnlineFactory
{
private:
    bool IsRegisteredAsSubsystem;

public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    virtual IOnlineSubsystemPtr CreateSubsystem(FName InstanceName) override;
};

void FOnlineSubsystemRedpointSteamModule::StartupModule()
{
    if (IsRunningCommandlet())
    {
        return;
    }

    FModuleManager &ModuleManager = FModuleManager::Get();
    auto OSS = ModuleManager.GetModule("OnlineSubsystem");
    if (OSS != nullptr)
    {
        ((FOnlineSubsystemModule *)OSS)->RegisterPlatformService(OnlineSubsystem::SubsystemName, this);
        this->IsRegisteredAsSubsystem = true;
    }

    UE_LOG(LogRedpointSteam, Verbose, TEXT("Redpoint Steam module has finished starting up."));
}

void FOnlineSubsystemRedpointSteamModule::ShutdownModule()
{
    if (this->IsRegisteredAsSubsystem)
    {
        FModuleManager &ModuleManager = FModuleManager::Get();
        auto OSS = ModuleManager.GetModule("OnlineSubsystem");
        if (OSS != nullptr)
        {
            ((FOnlineSubsystemModule *)OSS)->UnregisterPlatformService(OnlineSubsystem::SubsystemName);
            this->IsRegisteredAsSubsystem = false;
        }

        UE_LOG(LogRedpointSteam, Verbose, TEXT("Redpoint Steam module shutdown complete."));
    }
}

IOnlineSubsystemPtr FOnlineSubsystemRedpointSteamModule::CreateSubsystem(FName InstanceName)
{
    auto Inst = MakeShared<OnlineSubsystem::FOnlineSubsystemRedpointSteam, ESPMode::ThreadSafe>(InstanceName);
    if (Inst->IsEnabled())
    {
        if (!Inst->Init())
        {
            UE_LOG(LogRedpointSteam, Warning, TEXT("Unable to init Redpoint Steam online subsystem."));
            Inst->Shutdown();
            return nullptr;
        }
    }
    else
    {
        UE_LOG(LogRedpointSteam, Warning, TEXT("Redpoint Steam online subsystem is not enabled."));
        Inst->Shutdown();
        return nullptr;
    }

    return Inst;
}

#else

class FOnlineSubsystemRedpointSteamModule : public IModuleInterface
{
};

#endif

}

IMPLEMENT_MODULE(
    Redpoint::EOS::OnlineSubsystemRedpointSteam::FOnlineSubsystemRedpointSteamModule,
    OnlineSubsystemRedpointSteam)