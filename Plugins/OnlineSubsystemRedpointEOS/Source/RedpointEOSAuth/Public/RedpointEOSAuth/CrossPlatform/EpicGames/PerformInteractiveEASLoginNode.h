// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "EOSUserInterface_EnterDevicePinCode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/EOSAuthCallbackInfo.h"
#include "RedpointEOSAuth/UserInterface/UserInterfaceRef.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1395327004, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::UserInterface;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

class REDPOINTEOSAUTH_API FPerformInteractiveEASLoginNode : public FAuthenticationGraphNode
{
private:
    TSharedPtr<TUserInterfaceRef<
        IEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode_Context>>
        Widget;

    void OnSignInCancel(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone);

    void HandleEASAuthenticationCallback(
        const FEOSAuthCallbackInfo &Data,
        bool bThisCallIsResponsibleForLogout,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FAuthenticationGraphNodeOnDone &OnDone);

public:
    FPerformInteractiveEASLoginNode();
    UE_NONCOPYABLE(FPerformInteractiveEASLoginNode);
    virtual ~FPerformInteractiveEASLoginNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPerformInteractiveEASLoginNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(1395327004, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FPerformInteractiveEASLoginNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()