// Copyright June Rhodes. All Rights Reserved.

#include "EOSDeveloperAuthToolNotRunningButAttemptedCheck.h"

#include "RedpointEOSEditor/DevAuthToolLauncher.h"

const TArray<FEOSCheckEntry> FEOSDeveloperAuthToolNotRunningButAttemptedCheck::ProcessCustomSignal(
    const FString &Context,
    const FString &SignalId) const
{
    using namespace ::Redpoint::EOS::Editor;

    if (Context == TEXT("Authentication") && SignalId == TEXT("UnableToAuthenticateWithDevTool"))
    {
#if PLATFORM_WINDOWS || PLATFORM_MAC
        if (!FDevAuthToolLauncher::Get().IsDevAuthToolRunning())
        {
            return TArray<FEOSCheckEntry>{FEOSCheckEntry(
                "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::NotRunning",
                "The game tried to authenticate with the Developer Authentication Tool, but it isn't running.",
                TArray<FEOSCheckAction>{
#if PLATFORM_WINDOWS || PLATFORM_MAC
                    FEOSCheckAction(
                        "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::LaunchDevAuthTool",
                        "Launch Developer Authentication Tool"),
#endif
                    FEOSCheckAction(
                        "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::OpenDocumentation",
                        "Read documentation")})};
        }
#endif
    }
    return IEOSCheck::EmptyEntries;
}

void FEOSDeveloperAuthToolNotRunningButAttemptedCheck::HandleAction(const FString &CheckId, const FString &ActionId)
    const
{
    using namespace ::Redpoint::EOS::Editor;

    if (CheckId == "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::NotRunning")
    {
#if PLATFORM_WINDOWS || PLATFORM_MAC
        if (ActionId == "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::LaunchDevAuthTool")
        {
            FDevAuthToolLauncher::Get().Launch(true);
        }
        else
#endif
            if (ActionId == "FEOSDeveloperAuthToolNotRunningButAttemptedCheck::OpenDocumentation")
        {
            FPlatformProcess::LaunchURL(
                TEXT("https://docs.redpoint.games/eos-online-subsystem/docs/auth/dev_auth_tool_usage"),
                nullptr,
                nullptr);
        }
    }
}