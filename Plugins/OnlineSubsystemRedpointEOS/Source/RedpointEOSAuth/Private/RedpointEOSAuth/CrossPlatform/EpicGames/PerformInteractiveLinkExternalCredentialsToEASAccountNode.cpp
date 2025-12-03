// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveLinkExternalCredentialsToEASAccountNode.h"

#include "RedpointEOSAuth/AuthenticationHelpersEpicGames.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3726591427, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FPerformInteractiveLinkExternalCredentialsToEASAccountNode::OnLinkCancel(
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (this->Widget.IsValid())
    {
        this->Widget->RemoveFromState(State);
        this->Widget.Reset();
    }

    State->ErrorMessages.Add(TEXT("Account link was cancelled."));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    return;
}

void FPerformInteractiveLinkExternalCredentialsToEASAccountNode::HandleEASAuthenticationCallback(
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
            &FPerformInteractiveLinkExternalCredentialsToEASAccountNode::OnLinkCancel,
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
        // Link failed. Treat as a failure because the user has already done something interactive.
        State->ErrorMessages.Add(FString::Printf(
            TEXT("PerformInteractiveLinkExternalCredentialsToEASAccountNode: Interactive authentication failed with "
                 "error %s"),
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

void FPerformInteractiveLinkExternalCredentialsToEASAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
        using namespace ::Redpoint::EOS::API::Auth;

    check(!EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));
    check(!EOSString_ContinuanceToken::IsNone(State->EASExternalContinuanceToken));

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

#endif

}

REDPOINT_EOS_CODE_GUARD_END()