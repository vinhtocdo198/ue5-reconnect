// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/Configurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1623824531, Redpoint::EOS::Editor::Config::Configurators)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FPresenceConfigurator : public IConfigurator
{
public:
    FPresenceConfigurator() = default;
    UE_NONCOPYABLE(FPresenceConfigurator);
    virtual ~FPresenceConfigurator() override = default;

public:
    virtual void InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance) override;
    virtual void Load(
        FConfiguratorContext &Context,
        IConfigurationReader &Reader,
        UOnlineSubsystemEOSEditorConfig *Instance) override;
    virtual void Save(
        FConfiguratorContext &Context,
        IConfigurationWriter &Writer,
        UOnlineSubsystemEOSEditorConfig *Instance) override;
};

}

namespace Redpoint::EOS::Editor::Config::Configurators
{
REDPOINT_EOS_FILE_NS_EXPORT(1623824531, Redpoint::EOS::Editor::Config::Configurators, FPresenceConfigurator);
}

REDPOINT_EOS_CODE_GUARD_END()
