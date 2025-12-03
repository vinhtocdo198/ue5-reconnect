// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "EOSUserInterface_SignInOrCreateAccount.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode_AutomationSetting.h"
#include "RedpointEOSAuth/UserInterface/UserInterfaceRef.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2409578879, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FPromptToSignInOrCreateAccountNode : public FAuthenticationGraphNode
{
private:
    TSharedPtr<Redpoint::EOS::Auth::UserInterface::TUserInterfaceRef<
        IEOSUserInterface_SignInOrCreateAccount,
        UEOSUserInterface_SignInOrCreateAccount,
        UEOSUserInterface_SignInOrCreateAccount_Context>>
        Widget;

    virtual void SelectChoice(
        EEOSUserInterface_SignInOrCreateAccount_Choice SelectedChoice,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    UE_NONCOPYABLE(FPromptToSignInOrCreateAccountNode);
    FPromptToSignInOrCreateAccountNode() = default;
    virtual ~FPromptToSignInOrCreateAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPromptToSignInOrCreateAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2409578879, Redpoint::EOS::Auth::Nodes, FPromptToSignInOrCreateAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()