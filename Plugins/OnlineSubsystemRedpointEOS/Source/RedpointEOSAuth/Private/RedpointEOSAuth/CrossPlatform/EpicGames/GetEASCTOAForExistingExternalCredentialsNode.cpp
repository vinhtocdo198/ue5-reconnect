// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/GetEASCTOAForExistingExternalCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3507486269, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::OnPinGrantCancel(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (this->Widget.IsValid())
    {
        this->Widget->RemoveFromState(State);
        this->Widget.Reset();
    }

    // Should we continue instead of cancelling sign in here?
    State->ErrorMessages.Add(TEXT("Sign in was cancelled."));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    return;
}

void FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::OnLoginComplete(
    const FLogin::Result &Data,
    bool bThisCallIsResponsibleForLogout,
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (Data.PinGrantInfo.IsSet() && Data.ResultCode == EOS_EResult::EOS_Auth_PinGrantCode)
    {
        // The user needs to enter a PIN code on another device. Show the UI.

        check(this->Widget == nullptr);

        this->Widget = MakeShared<TUserInterfaceRef<
            IEOSUserInterface_EnterDevicePinCode,
            UEOSUserInterface_EnterDevicePinCode,
            UEOSUserInterface_EnterDevicePinCode_Context>>(
            State,
            State->Config->GetWidgetClass(
                TEXT("EnterDevicePinCode"),
                TEXT("/Script/RedpointEOSAuth.EOSDefaultUserInterface_EnterDevicePinCode")),
            TEXT("IEOSUserInterface_EnterDevicePinCode"));

        if (!this->Widget->IsValid())
        {
            State->ErrorMessages.Add(this->Widget->GetInvalidErrorMessage());
            this->Widget = nullptr;
            OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
            return;
        }

        check(!State->HasCurrentUserInterfaceWidget());

        this->Widget->GetContext()->SetOnCancel(UserInterface::FEOSUserInterface_EnterDevicePinCode_OnCancel::CreateSP(
            this,
            &FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::OnPinGrantCancel,
            State,
            OnDone));

        IEOSUserInterface_EnterDevicePinCode::Execute_SetupUserInterface(
            this->Widget->GetWidget(),
            this->Widget->GetContext(),
            Data.PinGrantInfo->VerificationURI,
            Data.PinGrantInfo->UserCode);

        this->Widget->AddToState(State);
        return;
    }

    // Remove the widget if it's currently visible.
    if (this->Widget.IsValid())
    {
        this->Widget->RemoveFromState(State);
        this->Widget.Reset();
    }

    if (Data.ResultCode == EOS_EResult::EOS_Success && EOSString_EpicAccountId::IsValid(Data.LocalUserId))
    {
        if (bThisCallIsResponsibleForLogout)
        {
            // Make sure we sign out of the Epic Games account on error or through normal logout request.
            State->AddLogoutNode(
                MakeShared<FSignOutEASAccountNode>(MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId)));
        }

        State->AuthenticatedCrossPlatformAccountId = MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId);
    }
    else if (Data.ResultCode == EOS_EResult::EOS_InvalidUser)
    {
        State->EASExternalContinuanceToken = Data.ContinuanceToken;
    }
    else
    {
        UE_LOG(
            LogRedpointEOSAuth,
            Error,
            TEXT("Got unexpected result when trying to obtain continuance token or EAS account using "
                 "external credentials (%s)."),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode)));
    }

    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::
    FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode()
    : Widget()
{
}

void FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
        using namespace ::Redpoint::EOS::API::Auth;

    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));
    checkf(
        State->ExistingExternalCredentials.IsValid(),
        TEXT("Expected the user to be signed in with external credentials already when running "
             "FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode node."));

    FAuthenticationHelpersEpicGames::Login(
        State->PlatformHandle,
        State->ExistingExternalCredentials->GetId(),
        State->ExistingExternalCredentials->GetToken(),
        EOS_ELoginCredentialType::EOS_LCT_ExternalAuth,
        FExternalCredentialType::FromString(State->ExistingExternalCredentials->GetType()),
        true,
        *State->Config,
        FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSP(
            this,
            &FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode::OnLoginComplete,
            State,
            OnDone));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()