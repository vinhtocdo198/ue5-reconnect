// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSEditor/Config/Configurator.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(638085313, Redpoint::EOS::Editor::Config::Configurators)
{
using namespace ::Redpoint::EOS::Editor::Config;

class FAndroidConfigurator : public IConfigurator
{
public:
    FAndroidConfigurator() = default;
    UE_NONCOPYABLE(FAndroidConfigurator);
    virtual ~FAndroidConfigurator() override = default;

private:
    int MinSDKVersion;

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
REDPOINT_EOS_FILE_NS_EXPORT(638085313, Redpoint::EOS::Editor::Config::Configurators, FAndroidConfigurator);
}

REDPOINT_EOS_CODE_GUARD_END()
