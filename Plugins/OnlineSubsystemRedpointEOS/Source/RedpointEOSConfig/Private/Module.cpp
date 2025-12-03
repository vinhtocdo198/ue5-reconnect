// Copyright June Rhodes. All Rights Reserved.

#include "HAL/IConsoleManager.h"
#include "HAL/PlatformProcess.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/MessageDialog.h"
#include "Modules/ModuleManager.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSConfig/DynamicConfigLayers/DynamicConfigLayerLoader.h"
#include "RedpointEOSConfig/Logging.h"

#define LOCTEXT_NAMESPACE "RedpointEOSConfig"

class REDPOINTEOSCONFIG_API FRedpointEOSConfigModule : public FDefaultModuleImpl
{
public:
    virtual void StartupModule() override
    {
        using namespace ::Redpoint::EOS::Config;

#if REDPOINT_EOS_UE_5_5_OR_LATER
        // Load our dynamic config layers into the engine.
        DynamicConfigLayers::FDynamicConfigLayerLoader::RegisterEventHandlerToLoadDynamicConfigLayers();
#endif

#if WITH_EDITOR
        auto *CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("net.AllowPIESeamlessTravel"));
        if (CVar != nullptr)
        {
            CVar->Set(true);
        }
        else
        {
            UE_LOG(
                LogRedpointEOSConfig,
                Warning,
                TEXT("Unable to locate net.AllowPIESeamlessTravel console variable to force it to true. Alternatively, "
                     "set [ConsoleVariables] net.AllowPIESeamlessTravel=true in your INI files."));
        }

        if (!IsRunningCommandlet())
        {
            TArray<TSharedRef<IPlugin>> EnabledPlugins = IPluginManager::Get().GetEnabledPlugins();
            bool bIsLegacyOnlineSubsystemBlueprintsEnabled = false;
            bool bIsLegacyMatchmakingEnabled = false;
            for (const auto &EnabledPlugin : EnabledPlugins)
            {
                if (EnabledPlugin->GetName() == TEXT("OnlineSubsystemBlueprints"))
                {
                    bIsLegacyOnlineSubsystemBlueprintsEnabled = true;
                }
                if (EnabledPlugin->GetName() == TEXT("Matchmaking"))
                {
                    bIsLegacyMatchmakingEnabled = true;
                }
            }
            if (bIsLegacyOnlineSubsystemBlueprintsEnabled || bIsLegacyMatchmakingEnabled)
            {
                FText LegacyWarningTitle = LOCTEXT("LegacyPluginTitle", "Error: Legacy plugins detected");

            ShowMessageBox:
                auto ReturnType = FMessageDialog::Open(
                    EAppMsgType::CancelRetryContinue,
                    FText::FormatOrdered(
                        LOCTEXT(
                            "LegacyDescription",
                            "This project has the {0} still enabled. The Redpoint 'EOS Online Framework' plugin now "
                            "incorporates the functionality of the\n'Online Subsystem Blueprints' and 'Matchmaking' "
                            "plugins, and they must be disabled or removed from your .uproject file to "
                            "continue.\n\nPlease follow the migration guide at: "
                            "https://docs.redpoint.games/docs/support/migrate.\n\nPress 'Cancel' to exit the editor "
                            "and open the migration guide.\nPress 'Continue' to open your project anyway (WARNING: "
                            "Continuing with these plugins enabled may corrupt your project!)."),
                        (bIsLegacyOnlineSubsystemBlueprintsEnabled && bIsLegacyMatchmakingEnabled)
                            ? LOCTEXT("LegacyBoth", "legacy Online Subsystem Blueprints and Matchmaking plugins")
                        : bIsLegacyOnlineSubsystemBlueprintsEnabled
                            ? LOCTEXT("LegacyOSB", "legacy Online Subsystem Blueprints plugin")
                            : LOCTEXT("LegacyMM", "legacy Matchmaking plugin")),
                    LegacyWarningTitle);
                if (ReturnType == EAppReturnType::Retry)
                {
                    goto ShowMessageBox;
                }

                bool bContinueOpening = false;
                if (ReturnType == EAppReturnType::Continue)
                {
                    ReturnType = FMessageDialog::Open(
                        EAppMsgType::YesNo,
                        LOCTEXT(
                            "LegacyFinalWarning",
                            "FINAL WARNING.\n\nYou are about to continue with conflicting plugins enabled in your "
                            "project. This may CORRUPT YOUR PROJECT, including all of your blueprint assets.\n\nAre "
                            "you certain you want to continue?\n\nPress 'Yes' to destroy all your hard work.\nPress "
                            "'No' to exit the editor and open the migration guide."),
                        LegacyWarningTitle);
                    if (ReturnType == EAppReturnType::Yes)
                    {
                        bContinueOpening = true;
                    }
                }

                if (!bContinueOpening)
                {
                    FPlatformProcess::LaunchURL(
                        TEXT("https://docs.redpoint.games/docs/support/migrate"),
                        nullptr,
                        nullptr);
                    FPlatformMisc::RequestExit(
                        true,
                        TEXT("Legacy plugin conflicts with Redpoint EOS Online Framework."));
                }
            }
        }
#endif
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRedpointEOSConfigModule, RedpointEOSConfig);