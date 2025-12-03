// Copyright June Rhodes. All Rights Reserved.

#include "EOSMissingScopeAuthorizationCheck.h"

const TArray<FEOSCheckEntry> FEOSMissingScopeAuthorizationCheck::ProcessLogMessage(
    EOS_ELogLevel InLogLevel,
    const FString &Category,
    const FString &Message) const
{
    if (Message.Contains("scope_consent_required") || Message.Contains("corrective_action_required"))
    {
        return TArray<FEOSCheckEntry>{FEOSCheckEntry(
            "FEOSMissingScopeAuthorizationCheck::Misconfigured",
            "An Epic Games account you are trying to use with the Developer Authentication Tool needs to go through "
            "the scope approval flow. The SDK will launch a web browser window where you can approve access.",
            TArray<FEOSCheckAction>{
                FEOSCheckAction("FEOSMissingScopeAuthorizationCheck::OpenDocumentation", "Read documentation")})};
    }
    return IEOSCheck::EmptyEntries;
}

void FEOSMissingScopeAuthorizationCheck::HandleAction(const FString &CheckId, const FString &ActionId) const
{
}