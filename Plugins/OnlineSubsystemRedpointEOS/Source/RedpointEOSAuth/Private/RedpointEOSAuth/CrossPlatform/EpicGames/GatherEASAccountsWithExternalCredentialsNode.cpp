// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/GatherEASAccountsWithExternalCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPersistentEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3364425848, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::API::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FGatherEASAccountsWithExternalCredentialsNode::OnResultCallback(
    const FLogin::Result &Data,
    bool bThisCallIsResponsibleForLogout,
    TSharedRef<FAuthenticationGraphState> State,
    TSharedRef<IOnlineExternalCredentials> ExternalCredentials,
    TFunction<void(FGatherEASAccountResponse)> OnPlatformDone)
{
    if (Data.ResultCode == EOS_EResult::EOS_Success && EOSString_EpicAccountId::IsValid(Data.LocalUserId))
    {
        // Add the native platform's attributes.
        for (const auto &KV : ExternalCredentials->GetAuthAttributes())
        {
            if (KV.Key != EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH)
            {
                State->AddResultUserAuthAttribute(KV.Key, KV.Value);
            }
            else
            {
                State->AddResultUserAuthAttribute("epic.authenticatedWith", KV.Value);
            }
        }

        // Pass the native subsystem on.
        State->Metadata.Add(EOS_METADATA_EAS_NATIVE_SUBSYSTEM, ExternalCredentials->GetNativeSubsystemName());

        if (bThisCallIsResponsibleForLogout)
        {
            // Make sure we sign out of the Epic Games account on error or through normal logout request.
            State->AddLogoutNode(
                MakeShared<FSignOutEASAccountNode>(MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
        }

        // Return the Epic Games account.
        OnPlatformDone(FGatherEASAccountResponse(Data.LocalUserId, nullptr));
    }
    else if (Data.ResultCode == EOS_EResult::EOS_InvalidUser)
    {
        // This credential is not associated with any Epic Games account.
        OnPlatformDone(FGatherEASAccountResponse(nullptr, Data.ContinuanceToken));
    }
    else if (Data.ResultCode == EOS_EResult::EOS_Auth_UserInterfaceRequired)
    {
        // This credential requires interactive authentication, so it can't be used without
        // the user explicitly opting to use an Epic Games account.
        OnPlatformDone(FGatherEASAccountResponse(nullptr, nullptr));
    }
    else
    {
        State->ErrorMessages.Add(FString::Printf(
            TEXT("FGatherEASAccountsWithExternalCredentialsNode: External credential "
                 "'%s' failed to authenticate with EAS: %s"),
            *ExternalCredentials->GetType(),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
        OnPlatformDone(FGatherEASAccountResponse(nullptr, nullptr));
    }
}

void FGatherEASAccountsWithExternalCredentialsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnStepDone)
{
    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    FMultiOperation<TSharedRef<IOnlineExternalCredentials>, FGatherEASAccountResponse>::RunSP<FAuthenticationGraphNode>(
        this,
        State->AvailableExternalCredentials,
        [this, State](
            const TSharedRef<IOnlineExternalCredentials> &ExternalCredentials,
            TFunction<void(FGatherEASAccountResponse)> OnPlatformDone) -> bool {
            /**
             * External credentials that return `ESupportedLinkType::None` will still be attempted here, until we
             * can update the Epic Games cross-platform account provider graph to avoiding linking/non-interactive
             * sign-in flows entirely when we don't have a linkable external credential.
             */
            if (ExternalCredentials->GetProviderSupportedLinkType(EPIC_GAMES_ACCOUNT_ID) ==
                ESupportedLinkType::PersistentAuth)
            {
                TOptional<FString> RefreshToken = ExternalCredentials->ReadData("EASRefreshToken");
                if (RefreshToken.IsSet())
                {
                    FAuthenticationHelpersEpicGames::Login(
                        State->PlatformHandle,
                        TEXT(""),
                        RefreshToken.GetValue(),
                        EOS_ELoginCredentialType::EOS_LCT_PersistentAuth,
                        (EOS_EExternalCredentialType)0,
                        true,
                        *State->Config,
                        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSP(
                            StaticCastSharedRef<FGatherEASAccountsWithExternalCredentialsNode>(this->AsShared()),
                            &FGatherEASAccountsWithExternalCredentialsNode::OnResultCallback,
                            State,
                            ExternalCredentials,
                            MoveTemp(OnPlatformDone)));
                    return true;
                }
                else
                {
                    OnPlatformDone(FGatherEASAccountResponse(nullptr, nullptr));
                    return true;
                }
            }
            else
            {
                FAuthenticationHelpersEpicGames::Login(
                    State->PlatformHandle,
                    ExternalCredentials->GetId(),
                    ExternalCredentials->GetToken(),
                    EOS_ELoginCredentialType::EOS_LCT_ExternalAuth,
                    FExternalCredentialType::FromString(ExternalCredentials->GetType()),
                    false,
                    *State->Config,
                    FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSP(
                        StaticCastSharedRef<FGatherEASAccountsWithExternalCredentialsNode>(this->AsShared()),
                        &FGatherEASAccountsWithExternalCredentialsNode::OnResultCallback,
                        State,
                        ExternalCredentials,
                        MoveTemp(OnPlatformDone)));
                return true;
            }
        },
        [State, OnStepDone](const TArray<FGatherEASAccountResponse> &Results) {
            // Select the first valid Epic Account ID from the results.
            for (auto Result : Results)
            {
                if (EOSString_EpicAccountId::IsValid(Result.Key))
                {
                    State->AuthenticatedCrossPlatformAccountId =
                        MakeShared<FEpicGamesCrossPlatformAccountId>(Result.Key);
                    break;
                }
            }

            // It no account was found, select the first continuance token.
            if (!State->AuthenticatedCrossPlatformAccountId.IsValid())
            {
                for (auto Result : Results)
                {
                    if (!EOSString_ContinuanceToken::IsNone(Result.Value))
                    {
                        State->EASExternalContinuanceToken = Result.Value;
                        break;
                    }
                }
            }

            OnStepDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        });
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()