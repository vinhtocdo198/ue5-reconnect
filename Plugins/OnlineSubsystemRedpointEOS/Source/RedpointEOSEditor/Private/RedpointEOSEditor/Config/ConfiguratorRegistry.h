// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemEOSEditorConfig.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationReader.h"
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/Configurator.h"
#include "RedpointEOSEditor/Config/ConfiguratorContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(225667077, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FConfiguratorRegistry
{
private:
    TArray<TSharedRef<IConfigurator>> Configurers;
    static TSharedPtr<FConfiguratorRegistry> RegistryInstance;

public:
    FConfiguratorRegistry();

    static void InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance);
    static void Load(
        FConfiguratorContext &Context,
        IConfigurationReader &Reader,
        UOnlineSubsystemEOSEditorConfig *Instance);
    static bool Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance);
    static void Save(
        FConfiguratorContext &Context,
        IConfigurationWriter &Writer,
        UOnlineSubsystemEOSEditorConfig *Instance);
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(225667077, Redpoint::EOS::Editor::Config, FConfiguratorRegistry)
}

REDPOINT_EOS_CODE_GUARD_END()
