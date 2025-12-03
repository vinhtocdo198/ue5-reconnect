// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/IOSConfigurator.h"

#include "RedpointEOSEditorModule.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3140785597, Redpoint::EOS::Editor::Config::Configurators)
{

void FIOSConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
}

void FIOSConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    // This value isn't stored in the instance.
    this->ShipForBitcode = true;
    Reader.GetBool(
        TEXT("bShipForBitcode"),
        this->ShipForBitcode,
        TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"),
        EConfigurationFileType::Engine,
        FName(TEXT("IOS")));
}

bool FIOSConfigurator::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        // We must fix up IOS settings.
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Detected iOS is not configured to ship for bitcode. This configuration setting will be automatically "
                 "updated."));
        return this->ShipForBitcode;
    }

    return false;
}

void FIOSConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        // IOS required settings.
        Writer.SetBool(
            TEXT("bShipForBitcode"),
            false,
            TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"),
            EConfigurationFileType::Engine,
            FName(TEXT("IOS")));
        // Set in DefaultEngine.ini as well since it's a build-time thing.
        Writer.SetBool(
            TEXT("bShipForBitcode"),
            false,
            TEXT("/Script/IOSRuntimeSettings.IOSRuntimeSettings"),
            EConfigurationFileType::Engine);
        this->ShipForBitcode = false;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()