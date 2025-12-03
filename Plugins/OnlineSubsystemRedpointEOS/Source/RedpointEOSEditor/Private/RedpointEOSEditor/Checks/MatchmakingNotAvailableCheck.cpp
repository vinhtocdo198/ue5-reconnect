// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSEditor/Checks/MatchmakingNotAvailableCheck.h"

#if REDPOINT_EOS_IS_FREE_EDITION
#include "Modules/ModuleManager.h"
#include "RedpointEOSEditorModule.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if REDPOINT_EOS_IS_FREE_EDITION

namespace REDPOINT_EOS_FILE_NS_ID(2943692109, Redpoint::EOS::Editor::Checks)
{

FMatchmakingNotAvailableCheck::FMatchmakingNotAvailableCheck()
    : NextAllowedDisplayTime(FDateTime::MinValue())
{
}

const TArray<FEOSCheckEntry> FMatchmakingNotAvailableCheck::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    if (Context == TEXT("Matchmaking") && SignalId == TEXT("MatchmakingNotAvailableFreeEdition") &&
        FDateTime::UtcNow() > this->NextAllowedDisplayTime)
    {
        const_cast<FMatchmakingNotAvailableCheck *>(this)->NextAllowedDisplayTime =
            FDateTime::UtcNow() + FTimespan::FromMinutes(5);

        return TArray<FEOSCheckEntry>{FEOSCheckEntry(
            "FMatchmakingNotAvailableCheck::NotAvailableFreeEdition",
            "Team-based matchmaking is not available in the Free Edition of EOS Online Framework. To use team-based "
            "matchmaking, upgrade to the Paid Edition.",
            TArray<FEOSCheckAction>{
                FEOSCheckAction("FMatchmakingNotAvailableCheck::OpenPurchasePage", "Upgrade Now")})};
    }

    return TArray<FEOSCheckEntry>();
}

void FMatchmakingNotAvailableCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
    if (CheckId == "FMatchmakingNotAvailableCheck::NotAvailableFreeEdition")
    {
        if (ActionId == "FMatchmakingNotAvailableCheck::OpenPurchasePage")
        {
            FPlatformProcess::LaunchURL(TEXT("https://unreal.tools/eos"), nullptr, nullptr);
        }
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()