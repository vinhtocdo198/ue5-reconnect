// Copyright June Rhodes. All Rights Reserved.

#include "EOSWrongOnlineContextForPlayInEditorCheck.h"

const TArray<FEOSCheckEntry> FEOSWrongOnlineContextForPlayInEditorCheck::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    if (Context == TEXT("Authentication") && SignalId == TEXT("WrongOnlineContextForPlayInEditor"))
    {
        return TArray<FEOSCheckEntry>{FEOSCheckEntry(
            "FEOSWrongOnlineContextForPlayInEditorCheck::IncorrectCall",
            "Something in your game is calling IOnlineSubsystem::Get(), or Online::Get...() without a world parameter. "
            "These calls return the wrong online subsystem for play-in-editor to work. Please change your code to use "
            "Online::GetSubsystem(this->GetWorld()) instead. If you are using the 'Advanced Sessions Plugin', upgrade "
            "to a later version to fix this issue.",
            TArray<FEOSCheckAction>())};
    }
    return IEOSCheck::EmptyEntries;
}

void FEOSWrongOnlineContextForPlayInEditorCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
}