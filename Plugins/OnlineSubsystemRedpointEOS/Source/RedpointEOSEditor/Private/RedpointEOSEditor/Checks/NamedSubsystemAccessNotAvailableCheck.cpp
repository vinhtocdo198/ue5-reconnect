// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Checks/NamedSubsystemAccessNotAvailableCheck.h"

#if REDPOINT_EOS_IS_FREE_EDITION
#include "Modules/ModuleManager.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_IS_FREE_EDITION

namespace REDPOINT_EOS_FILE_NS_ID(847567554, Redpoint::EOS::Editor::Checks)
{

FNamedSubsystemAccessNotAvailableCheck::FNamedSubsystemAccessNotAvailableCheck()
    : NextAllowedDisplayTime(FDateTime::MinValue())
{
}

const TArray<FEOSCheckEntry> FNamedSubsystemAccessNotAvailableCheck::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    if (Context == TEXT("NamedSubsystemAccess") && SignalId == TEXT("NamedSubsystemAccessNotAvailableFreeEdition") &&
        FDateTime::UtcNow() > this->NextAllowedDisplayTime)
    {
        const_cast<FNamedSubsystemAccessNotAvailableCheck *>(this)->NextAllowedDisplayTime =
            FDateTime::UtcNow() + FTimespan::FromMinutes(5);

        return TArray<FEOSCheckEntry>{FEOSCheckEntry(
            "FNamedSubsystemAccessNotAvailableCheck::NotAvailableFreeEdition",
            "Accessing online subsystems other than Redpoint EOS via blueprints is not supported in the Free Edition of EOS Online Framework. To use access online subsystems other than Redpoint EOS, please upgrade to the Paid Edition.",
            TArray<FEOSCheckAction>{
                FEOSCheckAction("FNamedSubsystemAccessNotAvailableCheck::OpenPurchasePage", "Upgrade Now")})};
    }

    return TArray<FEOSCheckEntry>();
}

void FNamedSubsystemAccessNotAvailableCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
    if (CheckId == "FNamedSubsystemAccessNotAvailableCheck::NotAvailableFreeEdition")
    {
        if (ActionId == "FNamedSubsystemAccessNotAvailableCheck::OpenPurchasePage")
        {
            FPlatformProcess::LaunchURL(TEXT("https://unreal.tools/eos"), nullptr, nullptr);
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()