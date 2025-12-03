// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPIEDeveloperAuthenticationEASCredentialsNode.h"

#include "Engine/Engine.h"
#include "Misc/CommandLine.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1794112417, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

FString FTryPIEDeveloperAuthenticationEASCredentialsNode::GetCredentialName(TSharedRef<FAuthenticationGraphState> State)
{
#if WITH_EDITOR
    FString CommandLine = FString(FCommandLine::Get()).ToUpper();
    if (CommandLine.Contains(TEXT("-GAME")))
    {
        // For standalone games launched from the editor, we actually have a hint at what context they're running for,
        // in the form of the GameUserSettingsINI path. The filename of this path contains the real "PIE instance" ID.
        TArray<FString> Tokens;
        TArray<FString> Switches;
        FCommandLine::Parse(*CommandLine, Tokens, Switches);
        for (const auto &Token : Tokens)
        {
            if (Token.StartsWith(TEXT("GAMEUSERSETTINGSINI=PIEGAMEUSERSETTINGS")))
            {
                int Start = Token.Find(TEXT("PIEGAMEUSERSETTINGS"), ESearchCase::CaseSensitive, ESearchDir::FromEnd) +
                            FString(TEXT("PIEGAMEUSERSETTINGS")).Len();
                int End = Token.Len();
                FString PIENumberAsString = Token.Mid(Start, End - Start);
                return FString::Printf(TEXT("Context_%d"), FCString::Atoi(*PIENumberAsString) + 1);
            }
        }
    }
#endif
#if !UE_BUILD_SHIPPING
    FString DevAuthToolName;
    if (FParse::Value(FCommandLine::Get(), TEXT("-DevAuthToolName="), DevAuthToolName))
    {
        DevAuthToolName = DevAuthToolName.TrimStartAndEnd();
        if (!DevAuthToolName.IsEmpty())
        {
            return DevAuthToolName;
        }
    }
#endif
    FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(State->WorldContextHandle);
    if (WorldContext != nullptr)
    {
        bool bIsHostingDedicatedServer = false;
        for (const auto &WC : GEngine->GetWorldContexts())
        {
            if (WC.RunAsDedicated)
            {
                bIsHostingDedicatedServer = true;
                break;
            }
        }

        // If bIsHostingDedicatedServer is true, then the first client context is PIEInstance 2, so we need to adjust
        // down for that.
        return FString::Printf(TEXT("Context_%d"), WorldContext->PIEInstance + (bIsHostingDedicatedServer ? 0 : 1));
    }
    return TEXT("");
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()