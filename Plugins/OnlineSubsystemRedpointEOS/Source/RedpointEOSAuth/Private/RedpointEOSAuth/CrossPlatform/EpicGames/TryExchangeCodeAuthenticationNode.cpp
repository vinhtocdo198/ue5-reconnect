// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryExchangeCodeAuthenticationNode.h"

#include "Misc/CommandLine.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPersistentEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3513844636, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FTryExchangeCodeAuthenticationNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;

#if PLATFORM_WINDOWS || PLATFORM_MAC || PLATFORM_LINUX
    FString AuthType;
    if (!FParse::Value(FCommandLine::Get(), TEXT("AUTH_TYPE="), AuthType, true))
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
    if (AuthType != TEXT("exchangecode"))
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("TryExchangeCodeAuthenticationNode: Detected Epic Games Launcher, performing exchange code "
             "authentication."));

    // Try doing auth with EOS_LCT_ExchangeCode, which is the mechanism used by Epic Games Launcher.
    FString ExchangeCode;
    if (!FParse::Value(FCommandLine::Get(), TEXT("AUTH_PASSWORD="), ExchangeCode, true))
    {
        // No provided exchange code, even though we expected to get one.
        UE_LOG(
            LogRedpointEOSAuth,
            Warning,
            TEXT("TryExchangeCodeAuthenticationNode: Missing -AUTH_PASSWORD= parameter for exchange code "
                 "authentication."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    FAuthenticationHelpersEpicGames::Login(
        State->PlatformHandle,
        TEXT(""),
        ExchangeCode,
        EOS_ELoginCredentialType::EOS_LCT_ExchangeCode,
        (EOS_EExternalCredentialType)0,
        true,
        *State->Config,
        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (Data.ResultCode != EOS_EResult::EOS_Success || !EOSString_EpicAccountId::IsValid(Data.LocalUserId))
                {
                    // Unable to authenticate with exchange code.
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                    return;
                }

                if (bThisCallIsResponsibleForLogout)
                {
                    // Make sure we sign out of the Epic Games account on error or through normal logout request.
                    State->AddLogoutNode(MakeShared<FSignOutEASAccountNode>(
                        MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
                }

                // This is *definitely* the Epic Games subsystem if we're doing exchange code.
                State->Metadata.Add(EOS_METADATA_EAS_NATIVE_SUBSYSTEM, FName(TEXT("RedpointEAS")));

                // Store how we authenticated with Epic.
                State->AddResultUserAuthAttribute("epic.authenticatedWith", "exchangeCode");

                // Set the authenticated Epic account ID into the state.
                State->AuthenticatedCrossPlatformAccountId =
                    MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId);
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                return;
            }));
#else
    // This authentication mode is not supported on non-desktop platforms.
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
#endif
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()