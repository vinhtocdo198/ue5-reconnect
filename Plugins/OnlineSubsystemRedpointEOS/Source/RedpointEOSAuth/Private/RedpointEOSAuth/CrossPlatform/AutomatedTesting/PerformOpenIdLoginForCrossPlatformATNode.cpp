// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/AutomatedTesting/PerformOpenIdLoginForCrossPlatformATNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/CrossPlatform/AutomatedTestingCrossPlatformAccountId.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(365535988, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::Auth::Nodes;

void FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
        using namespace ::Redpoint::EOS::API::Connect;

    checkf(State->Metadata.Contains(TEXT("AUTOMATED_TESTING_JWT")), TEXT("AUTOMATED_TESTING_JWT metadata not present"));

    FAuthenticationHelpersConnect::Login(
        State->PlatformHandle,
        TEXT(""),
        State->Metadata["AUTOMATED_TESTING_JWT"],
        TEXT(""),
        EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN,
        FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [this, State, OnDone](const FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (bThisCallIsResponsibleForLogout)
                {
                    State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                }
                if (Data.ResultCode == EOS_EResult::EOS_Success || Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                {
                    FAuthenticationGraphEOSCandidate Candidate = State->AddEOSConnectCandidate(
                        FText::FromString(TEXT("AutomatedTestingCrossPlatform")),
                        TMap<FString, FString>(),
                        Data,
                        FAuthenticationGraphRefreshEOSCredentials(),
                        NAME_None,
                        EAuthenticationGraphEOSCandidateType::CrossPlatform,
                        MakeShared<FAutomatedTestingCrossPlatformAccountId>(
                            TEXT("ThisValueDoesNotMatterForTestingPurposes")));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                }
                else if (Data.ResultCode == EOS_EResult::EOS_UnexpectedError)
                {
                    // Epic's authentication API has intermittent issues with OpenID and returns EOS_UnexpectedError in
                    // these cases. Just try again.
                    this->Execute(State, OnDone);
                }
                else
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("PerformOpenIdLoginForCrossPlatformAutomatedTestingNode: OpenID "
                             "failed to authenticate with EOS Connect: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                }
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()