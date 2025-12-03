// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Config/Configurators/AndroidConfigurator.h"

#include "RedpointEOSEditor/Config/PlatformHelpers.h"
#include "RedpointEOSEditorModule.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(638085313, Redpoint::EOS::Editor::Config::Configurators)
{

void FAndroidConfigurator::InitDefaults(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
}

void FAndroidConfigurator::Load(
    FConfiguratorContext &Context,
    IConfigurationReader &Reader,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    // This value isn't stored in the instance.
    FString MinSDKVersionString = TEXT("19");
    Reader.GetString(
        TEXT("MinSDKVersion"),
        MinSDKVersionString,
        TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"),
        EConfigurationFileType::Engine,
        FName(TEXT("Android")));
    this->MinSDKVersion = FCString::Atoi(*MinSDKVersionString);
}

bool FAndroidConfigurator::Validate(FConfiguratorContext &Context, UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        // We must fix up Android settings.
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Detected Android platform minimum SDK version is too low. This configuration setting will be "
                 "automatically updated."));
        return this->MinSDKVersion < 23;
    }

    return false;
}

void FAndroidConfigurator::Save(
    FConfiguratorContext &Context,
    IConfigurationWriter &Writer,
    UOnlineSubsystemEOSEditorConfig *Instance)
{
    if (Context.bAutomaticallyConfigureEngineLevelSettings)
    {
        // Android required settings.
        Writer.SetString(
            TEXT("MinSDKVersion"),
            TEXT("23"),
            TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"),
            EConfigurationFileType::Engine,
            FName(TEXT("Android")));
        // Set in DefaultEngine.ini as well since it's a build-time thing.
        Writer.SetString(
            TEXT("MinSDKVersion"),
            TEXT("23"),
            TEXT("/Script/AndroidRuntimeSettings.AndroidRuntimeSettings"),
            EConfigurationFileType::Engine);
        this->MinSDKVersion = 23;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()
