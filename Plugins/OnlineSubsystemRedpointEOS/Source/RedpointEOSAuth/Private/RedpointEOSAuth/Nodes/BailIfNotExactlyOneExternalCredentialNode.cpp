// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/BailIfNotExactlyOneExternalCredentialNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1791657458, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FBailIfNotExactlyOneExternalCredentialNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    if (State->Config->IsAutomatedTesting())
    {
        // We are running through authentication unit tests, which are designed to test the logic flow of the OSS
        // authentication graph without actually requiring an external service.
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
#endif

    if (State->AvailableExternalCredentials.Num() != 1)
    {
        State->ErrorMessages.Add(TEXT("Authentication failed because there are no valid credentials"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
    }
    else
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()