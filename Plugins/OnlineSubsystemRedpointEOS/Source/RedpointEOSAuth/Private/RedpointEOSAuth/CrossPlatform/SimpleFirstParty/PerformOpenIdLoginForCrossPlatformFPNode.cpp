// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/SimpleFirstParty/PerformOpenIdLoginForCrossPlatformFPNode.h"

#include "RedpointEOSAPI/Connect/Login.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/CrossPlatform/SimpleFirstPartyCrossPlatformAccountId.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2798274585, Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

void FPerformOpenIdLoginForCrossPlatformFPNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
        using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Auth::Nodes;

    if (!State->Metadata.Contains(TEXT("FIRST_PARTY_ACCESS_TOKEN")) ||
        !State->Metadata.Contains(TEXT("FIRST_PARTY_USER_ID")))
    {
        State->ErrorMessages.Add(TEXT("Missing first party credentials to complete login"));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    FAuthenticationHelpersConnect::Login(
        State->PlatformHandle,
        TEXT(""),
        State->Metadata["FIRST_PARTY_ACCESS_TOKEN"],
        TEXT(""),
        EOS_EExternalCredentialType::EOS_ECT_OPENID_ACCESS_TOKEN,
        FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [this, State, OnDone](const Connect::FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (bThisCallIsResponsibleForLogout)
                {
                    State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                }
                if (Data.ResultCode == EOS_EResult::EOS_Success || Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                {
                    FAuthenticationGraphEOSCandidate Candidate = State->AddEOSConnectCandidate(
                        FText::FromString(TEXT("SimpleFirstParty")),
                        TMap<FString, FString>(),
                        Data,
                        FAuthenticationGraphRefreshEOSCredentials(),
                        NAME_None,
                        EAuthenticationGraphEOSCandidateType::CrossPlatform,
                        MakeShared<FSimpleFirstPartyCrossPlatformAccountId>(
                            FCString::Atoi64(*(State->Metadata["FIRST_PARTY_USER_ID"].GetValue<FString>()))));
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
                        TEXT("PerformOpenIdLoginForAutomatedTestingNode: OpenID "
                             "failed to authenticate with EOS Connect: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                }
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()