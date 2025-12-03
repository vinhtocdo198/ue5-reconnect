// Copyright June Rhodes. All Rights Reserved.

#include "EOSNetDriverSettingsCheck.h"

#if !REDPOINT_EOS_UE_5_5_OR_LATER

#include "Misc/ConfigCacheIni.h"
#include "RedpointEOSConfig/EngineConfigHelpers.h"
#include "RedpointEOSEditor/Config/ConfigurationFilePathHelpers.h"
#include "RedpointEOSEditor/Config/ConfigurationFileType.h"

FEOSNetDriverSettingsCheck::FEOSNetDriverSettingsCheck()
{
}

const TArray<FEOSCheckEntry> FEOSNetDriverSettingsCheck::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    using namespace ::Redpoint::EOS::Editor::Config;

    if (Context == TEXT("Configuration") && SignalId == TEXT("Load"))
    {
        FConfigFile *F = Redpoint::EOS::Config::FEngineConfigHelpers::FindByName(
            GetFilePath(EConfigurationFileType::Engine, TEXT("")));
        if (F)
        {
            if (F->Contains("/Script/OnlineSubsystemUtils.IpNetDriver"))
            {
                return TArray<FEOSCheckEntry>{FEOSCheckEntry(
                    "FEOSNetDriverSettingsCheck::OldEntries",
                    "You have settings in DefaultEngine.ini for IpNetDriver that will be ignored when using the EOS "
                    "networking driver. You should either delete these settings from DefaultEngine.ini, or move "
                    "them to DefaultOnlineSubsystemRedpointEOS.ini under the "
                    "[/Script/RedpointEOSNetworking.RedpointEOSNetDriver] section. Refer to the "
                    "documentation for more information.",
                    TArray<FEOSCheckAction>{
                        FEOSCheckAction("FEOSNetDriverSettingsCheck::OpenDocumentation", "Read documentation")})};
            }
        }
    }

    return IEOSCheck::EmptyEntries;
}

void FEOSNetDriverSettingsCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
    if (CheckId == "FEOSNetDriverSettingsCheck::OldEntries")
    {
        if (ActionId == "FEOSNetDriverSettingsCheck::OpenDocumentation")
        {
            FPlatformProcess::LaunchURL(
                TEXT(
                    "https://docs.redpoint.games/eos-online-subsystem/docs/"
                    "networking_configuration#customizing-unetdriver-settings"),
                nullptr,
                nullptr);
        }
    }
}

#endif