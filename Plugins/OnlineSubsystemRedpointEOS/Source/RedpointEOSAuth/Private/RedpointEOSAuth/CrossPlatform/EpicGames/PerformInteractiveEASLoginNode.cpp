// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveEASLoginNode.h"

#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1395327004, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

FPerformInteractiveEASLoginNode::FPerformInteractiveEASLoginNode()
    : Widget()
{
}

void FPerformInteractiveEASLoginNode::OnSignInCancel(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (this->Widget.IsValid())
    {
        this->Widget->RemoveFromState(State);
        this->Widget.Reset();
    }

    State->ErrorMessages.Add(TEXT("Sign in was cancelled."));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    return;
}

void FPerformInteractiveEASLoginNode::HandleEASAuthenticationCallback(
    const FEOSAuthCallbackInfo &Data,
    bool bThisCallIsResponsibleForLogout,
    const TSharedRef<FAuthenticationGraphState> &State,
    const FAuthenticationGraphNodeOnDone &OnDone)
{
    if (Data.PinGrantInfo().IsSet() && Data.ResultCode() == EOS_EResult::EOS_Auth_PinGrantCode)
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
            &FPerformInteractiveEASLoginNode::OnSignInCancel,
            State,
            OnDone));

        IEOSUserInterface_EnterDevicePinCode::Execute_SetupUserInterface(
            this->Widget->GetWidget(),
            this->Widget->GetContext(),
            Data.PinGrantInfo()->VerificationURI,
            Data.PinGrantInfo()->UserCode);

        this->Widget->AddToState(State);
        return;
    }

    // Remove the widget if it's currently visible.
    if (this->Widget.IsValid())
    {
        this->Widget->RemoveFromState(State);
        this->Widget.Reset();
    }

    if (Data.ResultCode() != EOS_EResult::EOS_Success || !EOSString_EpicAccountId::IsValid(Data.LocalUserId()))
    {
        // Authentication failed. In this case, the user has interactively logged in. It's be weird if
        // the user logged in through their web browser, we get a backend error and then continue to
        // sign them in with some other unrelated account.
        //
        // So instead we treat this as a full authentication failure.
        State->ErrorMessages.Add(FString::Printf(
            TEXT("PerformInteractiveEASLoginNode: Interactive authentication failed with error %s"),
            ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode()))));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    if (bThisCallIsResponsibleForLogout)
    {
        // Make sure we sign out of the Epic Games account on error or through normal logout request.
        State->AddLogoutNode(
            MakeShared<FSignOutEASAccountNode>(MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId())));
    }

    // Store how we authenticated with Epic.
    State->AddResultUserAuthAttribute("epic.authenticatedWith", "interactive");

    // Set the authenticated Epic account ID into the state.
    State->AuthenticatedCrossPlatformAccountId = MakeShared<FEpicGamesCrossPlatformAccountId>(Data.LocalUserId());

    // Otherwise, we are done.
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

void FPerformInteractiveEASLoginNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;
    
    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    bool bIsNSAContinuanceToken = State->EASExternalContinuanceToken != nullptr &&
                                  State->Metadata.Contains("EAS_EXTERNAL_CONTINUANCE_TOKEN_IS_NSA") &&
                                  State->Metadata["EAS_EXTERNAL_CONTINUANCE_TOKEN_IS_NSA"].GetValue<bool>();

    if (State->EASExternalContinuanceToken != nullptr && !bIsNSAContinuanceToken)
    {
        FAuthenticationHelpersEpicGames::LinkAccount(
            State->PlatformHandle,
            State->EASExternalContinuanceToken,
            EOS_ELinkAccountFlags::EOS_LA_NoFlags,
            nullptr,
            FAuthenticationHelpersEpicGames::FLinkAccountCompletionDelegate::CreateSPLambda(
                this,
                [this, State, OnDone](const FLinkAccount::Result &Data, bool bThisCallIsResponsibleForLogout) {
                    this->HandleEASAuthenticationCallback(
                        FEOSAuthCallbackInfo(Data),
                        bThisCallIsResponsibleForLogout,
                        State,
                        OnDone);
                }));
    }
    else
    {
        FAuthenticationHelpersEpicGames::Login(
            State->PlatformHandle,
            TEXT(""),
            TEXT(""),
            EOS_ELoginCredentialType::EOS_LCT_AccountPortal,
            (EOS_EExternalCredentialType)0,
            true,
            *State->Config,
            FAuthenticationHelpersEpicGames::FLoginCompletionDelegate::CreateSPLambda(
                this,
                [this, State, OnDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                    this->HandleEASAuthenticationCallback(
                        FEOSAuthCallbackInfo(Data),
                        bThisCallIsResponsibleForLogout,
                        State,
                        OnDone);
                }));
    }
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()