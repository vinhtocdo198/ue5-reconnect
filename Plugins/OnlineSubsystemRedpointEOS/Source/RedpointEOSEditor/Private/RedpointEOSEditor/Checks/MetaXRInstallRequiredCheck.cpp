// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Checks/MetaXRInstallRequiredCheck.h"

#if REDPOINT_EOS_UE_5_5_OR_LATER
#include "Modules/ModuleManager.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_UE_5_5_OR_LATER

namespace REDPOINT_EOS_FILE_NS_ID(300840337, Redpoint::EOS::Editor::Checks)
{

FMetaXRInstallRequiredCheck::FMetaXRInstallRequiredCheck()
    : SecondsUntilNextCheck(60.0f)
    , bNeverCheckAgain(false)
{
}

bool FMetaXRInstallRequiredCheck::ShouldTick() const
{
    return true;
}

const TArray<FEOSCheckEntry> FMetaXRInstallRequiredCheck::Tick(float DeltaSeconds)
{
    if (this->bNeverCheckAgain)
    {
        return TArray<FEOSCheckEntry>();
    }

    this->SecondsUntilNextCheck -= DeltaSeconds;
    if (this->SecondsUntilNextCheck > 0.0f)
    {
        return TArray<FEOSCheckEntry>();
    }

    // Check if the Meta XR module is loaded. If it is, we never need to check devices.
    auto *MetaXRModule = FModuleManager::Get().GetModule(TEXT("OculusXREditor"));
    if (MetaXRModule != nullptr)
    {
        this->bNeverCheckAgain = true;
        return TArray<FEOSCheckEntry>();
    }

    // Set up the timer so we'll check again in another 60 seconds.
    this->SecondsUntilNextCheck = 60.0f;

    // Get the list of devices that populate the "Quick Launch" menu, and check if any
    // Meta Quest devices are currently connected.
    auto *Module =
        static_cast<ITargetDeviceServicesModule *>(FModuleManager::Get().GetModule(TEXT("TargetDeviceServices")));
    if (Module != nullptr)
    {
        TArray<TSharedPtr<ITargetDeviceService, ESPMode::ThreadSafe>> Services;
        Module->GetDeviceServiceManager()->GetServices(Services);
        for (const auto &Service : Services)
        {
            UE_LOG(
                LogRedpointEOSEditor,
                Verbose,
                TEXT("Device service: DevicePlatformName '%s', DeviceName '%s'"),
                *Service->GetDevicePlatformName().ToString(),
                *Service->GetDeviceName());

            if (Service->GetDevicePlatformName().IsEqual(FName(TEXT("Android"))) &&
                Service->GetDeviceName().Contains(TEXT("Quest")))
            {
                // We detected a Meta Quest device being connected but the Meta XR plugin is not installed.
                // Return the editor check once and then never check again (so as to not bug the user repeatedly about
                // it).
                this->bNeverCheckAgain = true;
                return TArray<FEOSCheckEntry>{FEOSCheckEntry(
                    "FMetaXRInstallRequiredCheck::MissingPlugin",
                    "You are missing the 'Meta XR' plugin required for EOS to work correctly with Meta Quest devices. "
                    "Please install or enable the plugin in your project, otherwise your game will not be able to "
                    "authenticate with Epic Online Services on Meta Quest.",
                    TArray<FEOSCheckAction>{FEOSCheckAction(
                        "FMetaXRInstallRequiredCheck::OpenMetaXRPluginInstall",
                        "Install Meta XR Plugin")})};
            }
        }
    }

    return TArray<FEOSCheckEntry>();
}

void FMetaXRInstallRequiredCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
    if (CheckId == "FMetaXRInstallRequiredCheck::MissingPlugin")
    {
        if (ActionId == "FMetaXRInstallRequiredCheck::OpenMetaXRPluginInstall")
        {
            FPlatformProcess::LaunchURL(
                TEXT("https://developers.meta.com/horizon/downloads/package/unreal-engine-5-integration"),
                nullptr,
                nullptr);
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()