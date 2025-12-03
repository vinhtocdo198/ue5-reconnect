// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSPlatform/RuntimePlatformRegistry.h"

#include "RedpointEOSConfig/DependentModuleLoader.h"
#include "RedpointEOSPlatform/Logging.h"

namespace REDPOINT_EOS_FILE_NS_ID(1189415427, Redpoint::EOS::Platform)
{

bool FRuntimePlatformRegistry::IsRuntimePlatformValid() const
{
    return this->RuntimePlatform.IsValid();
}

void FRuntimePlatformRegistry::UnloadRuntimePlatform()
{
    this->RuntimePlatform->Unload();
    this->RuntimePlatform.Reset();
}

TSharedRef<IRuntimePlatform> FRuntimePlatformRegistry::GetRuntimePlatform() const
{
    if (!this->RuntimePlatform.IsValid())
    {
        // Try to get RuntimePlatform populated by
        // using FDependentModuleLoader.
        Redpoint::EOS::Config::FDependentModuleLoader::LoadPlatformDependentModules();

        // If RuntimePlatform is still not populated, this
        // is a hard error - we won't be able to dereference
        // RuntimePlatform to get any platform.
        checkf(
            this->RuntimePlatform.IsValid(),
            TEXT("Expected SetRuntimePlatform to be called before GetRuntimePlatform!"));
    }

    return this->RuntimePlatform.ToSharedRef();
}

void FRuntimePlatformRegistry::SetRuntimePlatform(const TSharedRef<IRuntimePlatform> &InRuntimePlatform)
{
    checkf(
        !this->RuntimePlatform.IsValid(),
        TEXT("Expected the runtime platform not to be set when SetRuntimePlatform called!"));
    this->RuntimePlatform = InRuntimePlatform;

    this->RuntimePlatform->Load();
}

void FRuntimePlatformRegistry::AddRuntimePlatformIntegration(
    const FName &InName,
    const TSharedRef<IRuntimePlatformIntegration> &InRuntimePlatformIntegration)
{
    if (!this->RuntimePlatformIntegrations.Contains(InName))
    {
        this->RuntimePlatformIntegrations.Add(InName, InRuntimePlatformIntegration);
        this->RuntimePlatformIntegrationsArray.Add(InRuntimePlatformIntegration);
        UE_LOG(LogRedpointEOSPlatform, Verbose, TEXT("Added runtime integration: %s"), *InName.ToString());
    }
}

void FRuntimePlatformRegistry::RemoveRuntimePlatformIntegration(const FName &InName)
{
    if (this->RuntimePlatformIntegrations.Contains(InName))
    {
        auto &RuntimePlatformIntegration = this->RuntimePlatformIntegrations[InName];
        for (int i = this->RuntimePlatformIntegrationsArray.Num() - 1; i >= 0; i--)
        {
            if (this->RuntimePlatformIntegrationsArray[i] == RuntimePlatformIntegration)
            {
                this->RuntimePlatformIntegrationsArray.RemoveAt(i);
            }
        }
        this->RuntimePlatformIntegrations.Remove(InName);
        UE_LOG(LogRedpointEOSPlatform, Verbose, TEXT("Removed runtime integration: %s"), *InName.ToString());
    }
}

const TArray<TSharedRef<IRuntimePlatformIntegration>> &FRuntimePlatformRegistry::GetRuntimePlatformIntegrations() const
{
    return this->RuntimePlatformIntegrationsArray;
}

}