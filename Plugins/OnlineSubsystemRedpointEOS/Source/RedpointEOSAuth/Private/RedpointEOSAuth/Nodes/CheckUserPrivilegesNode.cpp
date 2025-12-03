// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/CheckUserPrivilegesNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Module.h"
#include "RedpointEOSPlatform/Services/RuntimePlatformAuthService.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(320275565, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Core;
using namespace ::Redpoint::EOS::Platform::Services;
using namespace ::Redpoint::EOS::API;

void FCheckUserPrivilegesNode::Execute(
    TSharedRef<FAuthenticationGraphState> InState,
    FAuthenticationGraphNodeOnDone InOnDone)
{
    // Only perform user privilege checks if configured to do so.
    if (!InState->Config->GetRequireOnlinePlayUserPrivilege())
    {
        InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    // Get the runtime platform authentication service.
    auto AuthService = InState->GetAuthService();
    if (!AuthService.IsValid())
    {
        UE_LOG(LogRedpointEOSAuth, Error, TEXT("Unable to locate the platform authentication service."));
        InState->ErrorMessages.Add(TEXT("Unable to locate the platform authentication service."));
        InOnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    // Ask the runtime platform integration service if the user is permitted to sign into Epic Online Services.
    auto CallContext = MakeShared<FRuntimePlatformServiceCallContext>(
        InState->PlatformHandle,
        InState->LocalUserNum,
        InState->GetWorld());
    AuthService->IsOnlinePlayPermitted(
        CallContext,
        IRuntimePlatformAuthService::FOnEnsureSignedInOnLocalPlatform::CreateSPLambda(
            this->AsShared(),
            [InState, InOnDone](const FError &ErrorCode) {
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