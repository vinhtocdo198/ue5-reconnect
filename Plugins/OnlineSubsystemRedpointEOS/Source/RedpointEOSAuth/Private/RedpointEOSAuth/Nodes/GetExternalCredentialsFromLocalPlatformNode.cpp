// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/GetExternalCredentialsFromLocalPlatformNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(184451355, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::API;

void FGetExternalCredentialsFromLocalPlatformNode::Execute(
    TSharedRef<FAuthenticationGraphState> InState,
    FAuthenticationGraphNodeOnDone InOnDone)
{
#if !defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING
    if (InState->Config->IsAutomatedTesting())
    {
        // We are running through authentication unit tests, which are designed to test the logic flow of the OSS
        // authentication graph without actually requiring an external service.
        InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }
#endif

    // Get the runtime platform authentication service.
    auto AuthService = InState->GetAuthService();
    if (!AuthService.IsValid())
    {
        UE_LOG(LogRedpointEOSAuth, Error, TEXT("Unable to locate the platform authentication service."));
        InState->ErrorMessages.Add(TEXT("Unable to locate the platform authentication service."));
        InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    // Ask the runtime platform integration to update the authentication graph state with the credentials from the local
    // platform.
    auto CallContext = MakeShared<FRuntimePlatformServiceCallContext>(
        InState->PlatformHandle,
        InState->LocalUserNum,
        InState->GetWorld());
    AuthService->GetCredentialsFromLocalPlatformDuringAuthenticationGraph(
        CallContext,
        InState,
        IRuntimePlatformAuthService::FOnGetCredentialsFromLocalPlatformDuringAuthenticationGraph::
            CreateSPLambda(this->AsShared(), [InState, InOnDone](const FError &ErrorCode) {
                if (!ErrorCode.WasSuccessful())
                {
                    UE_LOG(LogRedpointEOSAuth, Error, TEXT("Authentication failure: %s"), *ErrorCode.ToLogString());
                    InState->ErrorMessages.Add(ErrorCode.ToLogString());
                    InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                }
                else
                {
                    InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                }
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()