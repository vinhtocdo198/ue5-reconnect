// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemEOSEditorConfig.h"

#include "ISettingsContainer.h"
#include "ISettingsModule.h"
#include "Misc/Base64.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSEditor/Config/ConfigurationReader.h"
#include "RedpointEOSEditor/Config/ConfigurationWriter.h"
#include "RedpointEOSEditor/Config/ConfiguratorRegistry.h"
#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "RedpointEOSEditor/Config/Branch/ConfigurationReaderBranch.h"
#include "RedpointEOSEditor/Config/Branch/ConfigurationWriterBranch.h"
#else
#include "RedpointEOSEditor/Config/Legacy/ConfigurationReaderLegacy.h"
#include "RedpointEOSEditor/Config/Legacy/ConfigurationWriterLegacy.h"
#endif
#include "RedpointEOSEditor/Config/PlatformHelpers.h"
#include "RedpointEOSEditorModule.h"
#include "SourceControlHelpers.h"
#include "UObject/UObjectGlobals.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "OnlineSubsystemEOSEditorModule"

#define EOS_SECTION TEXT("EpicOnlineServices")

static auto GetReader()
{
#if REDPOINT_EOS_UE_5_5_OR_LATER
    return ::Redpoint::EOS::Editor::Config::Branch::FConfigurationReaderBranch();
#else
    return ::Redpoint::EOS::Editor::Config::Legacy::FConfigurationReaderLegacy();
#endif
}

static auto GetWriter()
{
#if REDPOINT_EOS_UE_5_5_OR_LATER
    return ::Redpoint::EOS::Editor::Config::Branch::FConfigurationWriterBranch();
#else
    return ::Redpoint::EOS::Editor::Config::Legacy::FConfigurationWriterLegacy();
#endif
}

UOnlineSubsystemEOSEditorConfig::UOnlineSubsystemEOSEditorConfig()
{
    using namespace ::Redpoint::EOS::Editor::Config;

    FConfiguratorContext Context;
    FConfiguratorRegistry::InitDefaults(Context, this);

    auto Reader = GetReader();
    FConfiguratorRegistry::Load(Context, Reader, this);
}

void UOnlineSubsystemEOSEditorConfig::LoadEOSConfig()
{
    using namespace ::Redpoint::EOS::Editor::Config;

    auto Reader = GetReader();
    FConfiguratorContext Context;
    FConfiguratorRegistry::Load(Context, Reader, this);

    if (!IsRunningCommandlet())
    {
        if (FConfiguratorRegistry::Validate(Context, this))
        {
            if (!USourceControlHelpers::IsEnabled() && !USourceControlHelpers::IsAvailable() &&
                USourceControlHelpers::CurrentProvider().Equals(TEXT("Perforce"), ESearchCase::IgnoreCase))
            {
                // @note: Attempt to workaround a Perforce issue where the engine is aware that it
                // is configured for Perforce, but has not yet connected to the Perforce depot. In this
                // case, we won't be able to check out configuration files to issue fixups. Delay
                // the fixups for a few seconds to allow the engine to connect.
                FTSTicker::GetCoreTicker().AddTicker(
                    FTickerDelegate::CreateWeakLambda(
                        this,
                        [this](float DeltaSeconds) {
                            this->FixupEOSConfig();
                            return false;
                        }),
                    5.0f);
            }
            else
            {
                this->FixupEOSConfig();
            }
        }
    }
}

void UOnlineSubsystemEOSEditorConfig::FixupEOSConfig()
{
    using namespace ::Redpoint::EOS::Editor::Config;

    FConfiguratorContext Context;
    auto Writer = GetWriter();
    FConfiguratorRegistry::Save(Context, Writer, this);

    switch (Writer.FlushChanges())
    {
    case EConfigurationWriterFlushResult::Success:
        break;
    case EConfigurationWriterFlushResult::FailedToCheckOutFiles:
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Failed to re-validate and save configuration changes when the configuration was loaded. The "
                 "current state of the configuration in Project Settings may not reflect the configuration on "
                 "disk. To resolve this issue, manually check out the 'Config' and 'Platforms' folders in your "
                 "source control provider."));
        break;
    }
}

void UOnlineSubsystemEOSEditorConfig::SaveEOSConfig()
{
    using namespace ::Redpoint::EOS::Editor::Config;

    FConfiguratorContext Context;
    FConfiguratorRegistry::Validate(Context, this);

    auto Writer = GetWriter();
    FConfiguratorRegistry::Save(Context, Writer, this);

    switch (Writer.FlushChanges())
    {
    case EConfigurationWriterFlushResult::Success:
        break;
    case EConfigurationWriterFlushResult::FailedToCheckOutFiles:
        UE_LOG(
            LogRedpointEOSEditor,
            Warning,
            TEXT("Failed to save the configuration changes to disk. The current state of the configuration in Project "
                 "Settings may not reflect the configuration on disk. To resolve this issue, manually check out the "
                 "'Config' and 'Platforms' folders in your source control provider."));
        break;
    }
}

const FName UOnlineSubsystemEOSEditorConfig::GetEditorAuthenticationGraphPropertyName()
{
    static const FName PropertyName =
        GET_MEMBER_NAME_CHECKED(UOnlineSubsystemEOSEditorConfig, EditorAuthenticationGraph);
    return PropertyName;
}

const FName UOnlineSubsystemEOSEditorConfig::GetAuthenticationGraphPropertyName()
{
    static const FName PropertyName = GET_MEMBER_NAME_CHECKED(UOnlineSubsystemEOSEditorConfig, AuthenticationGraph);
    return PropertyName;
}

const FName UOnlineSubsystemEOSEditorConfig::GetCrossPlatformAccountProviderPropertyName()
{
    static const FName PropertyName =
        GET_MEMBER_NAME_CHECKED(UOnlineSubsystemEOSEditorConfig, CrossPlatformAccountProvider);
    return PropertyName;
}

bool FOnlineSubsystemEOSEditorConfig::HandleSettingsSaved()
{
    this->Config->SaveEOSConfig();

    // Prevent the section that gets automatically added by the config serialization system from being added now.
    return false;
}

void FOnlineSubsystemEOSEditorConfig::RegisterSettings()
{
    if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        // Make the instanced version of the config (required so OverridePerObjectConfigSection will be called).
#if REDPOINT_EOS_IS_FREE_EDITION
        this->Config = NewObject<UOnlineSubsystemEOSEditorConfigFreeEdition>();
#else
        this->Config = NewObject<UOnlineSubsystemEOSEditorConfig>();
#endif
        this->Config->AddToRoot();

        this->Config->LoadEOSConfig();

        // Register the settings
        this->SettingsSection = SettingsModule->RegisterSettings(
            "Project",
            "Game",
            "Epic Online Services",
            LOCTEXT("EOSSettingsName", "Epic Online Services"),
            LOCTEXT("EOSSettingsDescription", "Configure Epic Online Services in your game."),
            this->Config);

        if (SettingsSection.IsValid())
        {
            SettingsSection->OnModified().BindRaw(this, &FOnlineSubsystemEOSEditorConfig::HandleSettingsSaved);
        }
    }
}

void FOnlineSubsystemEOSEditorConfig::UnregisterSettings()
{
    if (ISettingsModule *SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
    {
        SettingsModule->UnregisterSettings("Project", "Game", "Epic Online Services");
    }
}

#undef LOCTEXT_NAMESPACE