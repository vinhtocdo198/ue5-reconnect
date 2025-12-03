// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphNode.h"

#include "EOSUserInterface_EnterDevicePinCode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/EOSAuthCallbackInfo.h"
#include "RedpointEOSAuth/UserInterface/UserInterfaceRef.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3507486269, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::UserInterface;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;
using namespace ::Redpoint::EOS::API::Auth;

class REDPOINTEOSAUTH_API FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode
    : public FAuthenticationGraphNode
{
private:
    TSharedPtr<TUserInterfaceRef<
        IEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode_Context>>
        Widget;

    void OnPinGrantCancel(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone);

    void OnLoginComplete(
        const FLogin::Result &Data,
        bool bThisCallIsResponsibleForLogout,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode();
    UE_NONCOPYABLE(FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode);
    virtual ~FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3507486269,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FGetEASContinuanceTokenOrAccountForExistingExternalCredentialsNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()