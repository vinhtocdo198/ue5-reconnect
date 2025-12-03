// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveEASLoginNode.h"
#include "RedpointEOSAuth/UserInterface/UserInterfaceRef.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3726591427, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::UserInterface;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

class REDPOINTEOSAUTH_API FPerformInteractiveLinkExternalCredentialsToEASAccountNode : public FAuthenticationGraphNode
{
private:
    TSharedPtr<TUserInterfaceRef<
        IEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode,
        UEOSUserInterface_EnterDevicePinCode_Context>>
        Widget;

    void OnLinkCancel(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone);

    void HandleEASAuthenticationCallback(
        const FEOSAuthCallbackInfo &Data,
        bool bThisCallIsResponsibleForLogout,
        const TSharedRef<FAuthenticationGraphState> &State,
        const FAuthenticationGraphNodeOnDone &OnDone);

public:
    UE_NONCOPYABLE(FPerformInteractiveLinkExternalCredentialsToEASAccountNode);
    FPerformInteractiveLinkExternalCredentialsToEASAccountNode() = default;
    virtual ~FPerformInteractiveLinkExternalCredentialsToEASAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("PerformInteractiveLinkExternalCredentialsToEASAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3726591427,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FPerformInteractiveLinkExternalCredentialsToEASAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()