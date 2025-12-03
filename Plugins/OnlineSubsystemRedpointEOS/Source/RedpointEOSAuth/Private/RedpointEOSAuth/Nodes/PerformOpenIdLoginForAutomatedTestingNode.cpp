// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/PerformOpenIdLoginForAutomatedTestingNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(968583338, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Auth::Nodes;

class FAutomatedTestingExternalCredentials : public IOnlineExternalCredentials
{
private:
    FString Token;

public:
    UE_NONCOPYABLE(FAutomatedTestingExternalCredentials);
    FAutomatedTestingExternalCredentials(FString InToken)
        : Token(MoveTemp(InToken)){};
    virtual ~FAutomatedTestingExternalCredentials() override = default;
    virtual FText GetProviderDisplayName() const override
    {
        return FText::FromString(TEXT("AutomatedTesting"));
    }
    virtual FString GetType() const override
    {
        return TEXT("OPENID_ACCESS_TOKEN");
    }
    virtual FString GetId() const override
    {
        return TEXT("");
    }
    virtual FString GetToken() const override
    {
        return this->Token;
    }
    virtual TMap<FString, FString> GetAuthAttributes() const override
    {
        return TMap<FString, FString>();
    }
    virtual FName GetNativeSubsystemName() const override
    {
        return NAME_None;
    }
    virtual void Refresh(
        TSoftObjectPtr<UWorld> InWorld,
        int32 LocalUserNum,
        FOnlineExternalCredentialsRefreshComplete OnComplete) override
    {
        OnComplete.ExecuteIfBound(false);
    }
};

void FPerformOpenIdLoginForAutomatedTestingNode::Execute(
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
            [this, State, OnDone](const Connect::FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (bThisCallIsResponsibleForLogout)
                {
                    State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                }
                if (Data.ResultCode == EOS_EResult::EOS_Success || Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                {
                    FString JwtToken = State->Metadata["AUTOMATED_TESTING_JWT"];
                    TSharedRef<FAutomatedTestingExternalCredentials> Credentials =
                        MakeShared<FAutomatedTestingExternalCredentials>(JwtToken);

                    FAuthenticationGraphEOSCandidate Candidate =
                        State->AddEOSConnectCandidateFromExternalCredentials(Data, Credentials);
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