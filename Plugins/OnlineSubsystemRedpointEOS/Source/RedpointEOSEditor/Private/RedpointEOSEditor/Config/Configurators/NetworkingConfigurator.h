// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/Configurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(660091597, Redpoint::EOS::Editor::Config::Configurators)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FNetworkingConfigurator : public IConfigurator
{
public:
    FNetworkingConfigurator() = default;
    UE_NONCOPYABLE(FNetworkingConfigurator);
    virtual ~FNetworkingConfigurator() override = default;

public:
    virtual void InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance) override;
    virtual void Load(
        FConfiguratorContext &Context,
        IConfigurationReader &Reader,
        UOnlineSubsystemEOSEditorConfig *Instance) override;
    virtual bool Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance) override;
    virtual void Save(
        FConfiguratorContext &Context,
        IConfigurationWriter &Writer,
        UOnlineSubsystemEOSEditorConfig *Instance) override;
};

}

namespace Redpoint::EOS::Editor::Config::Configurators
{
REDPOINT_EOS_FILE_NS_EXPORT(660091597, Redpoint::EOS::Editor::Config::Configurators, FNetworkingConfigurator);
}

REDPOINT_EOS_CODE_GUARD_END()
