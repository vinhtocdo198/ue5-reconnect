// Copyright June Rhodes. All Rights Reserved.

#include "EOSEditorRequiresRestartForEpicEOSSDK.h"

#include "OnlineSubsystem.h"
#include "UnrealEdMisc.h"

const TArray<FEOSCheckEntry> FEOSEditorRequiresRestartForEpicEOSSDK::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    if (Context == TEXT("Configuration") && SignalId == TEXT("OldEOSSDKCopiedByBuildSystem"))
    {
        IOnlineSubsystem *OSS = IOnlineSubsystem::Get();
        if (OSS == nullptr || !OSS->GetSubsystemName().IsEqual(FName(TEXT("RedpointEOS"))))
        {
            return TArray<FEOSCheckEntry>{FEOSCheckEntry(
                "FEOSEditorRequiresRestartForEpicEOSSDK::OldEOSSDK",
                "The Unreal build system copied an older version of the EOS SDK out during the build. Your "
                "configuration has been updated to ensure the correct EOS SDK version will be used, but this change "
                "requires an editor restart to take effect.",
                TArray<FEOSCheckAction>{
                    FEOSCheckAction("FEOSEditorRequiresRestartForEpicEOSSDK::RestartEditor", "Restart editor")})};
        }
    }
    return IEOSCheck::EmptyEntries;
}

void FEOSEditorRequiresRestartForEpicEOSSDK::HandleAction(const FString &CheckId, const FString &ActionId) const
{
    if (CheckId == "FEOSEditorRequiresRestartForEpicEOSSDK::OldEOSSDK")
    {
        if (ActionId == "FEOSEditorRequiresRestartForEpicEOSSDK::RestartEditor")
        {
            const bool bWarn = false;
            FUnrealEdMisc::Get().RestartEditor(bWarn);
        }
    }
}