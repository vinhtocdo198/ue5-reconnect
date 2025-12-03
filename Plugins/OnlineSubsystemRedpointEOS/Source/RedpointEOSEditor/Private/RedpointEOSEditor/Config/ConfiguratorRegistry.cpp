// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/ConfiguratorRegistry.h"

#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#include "RedpointEOSEditor/Config/Configurators/AndroidConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/AuthenticationConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/CoreConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/CrossPlatformConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/FrameworkConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/IOSConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/NetworkingConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/PresenceConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/StatsConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/StorageConfigurator.h"
#include "RedpointEOSEditor/Config/Configurators/VoiceChatConfigurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(225667077, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config::Configurators;

TSharedPtr<FConfiguratorRegistry> FConfiguratorRegistry::RegistryInstance = nullptr;

FConfiguratorRegistry::FConfiguratorRegistry()
{
    this->Configurers.Add(MakeShared<FCoreConfigurator>());
    this->Configurers.Add(MakeShared<FAndroidConfigurator>());
    this->Configurers.Add(MakeShared<FAuthenticationConfigurator>());
    this->Configurers.Add(MakeShared<FNetworkingConfigurator>());
    this->Configurers.Add(MakeShared<FPresenceConfigurator>());
    this->Configurers.Add(MakeShared<FStatsConfigurator>());
    this->Configurers.Add(MakeShared<FStorageConfigurator>());
    this->Configurers.Add(MakeShared<FVoiceChatConfigurator>());
    this->Configurers.Add(MakeShared<FCrossPlatformConfigurator>());
    this->Configurers.Add(MakeShared<FIOSConfigurator>());
    this->Configurers.Add(MakeShared<FFrameworkConfigurator>());
}

void FConfiguratorRegistry::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (!FConfiguratorRegistry::RegistryInstance.IsValid())
    {
        FConfiguratorRegistry::RegistryInstance = MakeShared<FConfiguratorRegistry>();
    }

    for (const auto &Configurer : FConfiguratorRegistry::RegistryInstance->Configurers)
    {
        Configurer->InitDefaults(Context, Instance);
    }
}

void FConfiguratorRegistry::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (!FConfiguratorRegistry::RegistryInstance.IsValid())
    {
        FConfiguratorRegistry::RegistryInstance = MakeShared<FConfiguratorRegistry>();
    }

    for (const auto &Configurer : FConfiguratorRegistry::RegistryInstance->Configurers)
    {
        Configurer->Load(Context, Reader, Instance);
    }

    Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Configuration"),
        TEXT("Load"));
}

bool FConfiguratorRegistry::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (!FConfiguratorRegistry::RegistryInstance.IsValid())
    {
        FConfiguratorRegistry::RegistryInstance = MakeShared<FConfiguratorRegistry>();
    }

    bool bDidModify = false;
    for (const auto &Configurer : FConfiguratorRegistry::RegistryInstance->Configurers)
    {
        if (Configurer->Validate(Context, Instance))
        {
            bDidModify = true;
        }
    }
    return bDidModify;
}

void FConfiguratorRegistry::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (!FConfiguratorRegistry::RegistryInstance.IsValid())
    {
        FConfiguratorRegistry::RegistryInstance = MakeShared<FConfiguratorRegistry>();
    }

    for (const auto &Configurer : FConfiguratorRegistry::RegistryInstance->Configurers)
    {
        Configurer->Save(Context, Writer, Instance);
    }

    Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
        TEXT("Configuration"),
        TEXT("Save"));
}

}

REDPOINT_EOS_CODE_GUARD_END()