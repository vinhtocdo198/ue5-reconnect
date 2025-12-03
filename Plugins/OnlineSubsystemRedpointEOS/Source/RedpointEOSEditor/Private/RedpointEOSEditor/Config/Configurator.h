// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "OnlineSubsystemEOSEditorConfig.h"
#include "OnlineSubsystemEOSEditorConfigDetails.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/ConfigurationReader.h"
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/ConfiguratorContext.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3665883060, Redpoint::EOS::Editor::Config)
{
using namespace ::Redpoint::EOS::Editor::Config;

class IConfigurator
{
public:
    IConfigurator() = default;
    UE_NONCOPYABLE(IConfigurator);
    virtual ~IConfigurator() = default;

    /**
     * Initializes default settings for the instance.
     */
    virtual void InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance) = 0;

    /**
     * Loads the settings from the reader to the instance.
     */
    virtual void Load(
        FConfiguratorContext &Context,
        IConfigurationReader &Reader,
        UOnlineSubsystemEOSEditorConfig *Instance) = 0;

    /**
     * Validates the instance's current settings, and updates
     * them inline if they're invalid.
     *
     * Return true if the instance needs to be saved (i.e. changes
     * were made that need to be persisted to disk).
     */
    virtual bool Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
    {
        return false;
    }

    /**
     * Save the settings from the instance to the writer.
     */
    virtual void Save(
        FConfiguratorContext &Context,
        IConfigurationWriter &Writer,
        UOnlineSubsystemEOSEditorConfig *Instance) = 0;

    /**
     * Customizes how configuration is rendered in the Project Settings page.
     */
    virtual void CustomizeProjectSettings(
        FConfiguratorContext &Context,
        FOnlineSubsystemEOSEditorConfigDetails &Details,
        IDetailLayoutBuilder &Builder)
    {
    }
};

}

namespace Redpoint::EOS::Editor::Config
{
REDPOINT_EOS_FILE_NS_EXPORT(3665883060, Redpoint::EOS::Editor::Config, IConfigurator);
}

REDPOINT_EOS_CODE_GUARD_END()
