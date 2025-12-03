// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/GatherEOSAccountsWithExternalCredentialsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/ExternalCredentialType.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSAuth/OnlineExternalCredentials.h"
#include "RedpointEOSConfig/Config.h"
#include "RedpointEOSCore/Utils/MultiOperation.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2696708671, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

void FGatherEOSAccountsWithExternalCredentialsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnStepDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Core::Utils;
        using namespace ::Redpoint::EOS::Auth::Nodes;

    FMultiOperation<TSharedRef<IOnlineExternalCredentials>, bool>::RunSP(
        this,
        State->AvailableExternalCredentials,
        [this, State](
            const TSharedRef<IOnlineExternalCredentials> &ExternalCredentials,
            TFunction<void(bool)> OnPlatformDone) -> bool {
            FAuthenticationHelpersConnect::Login(
                State->PlatformHandle,
                ExternalCredentials->GetId(),
                ExternalCredentials->GetToken(),
                ExternalCredentials->GetUserDisplayName(),
                FExternalCredentialType::FromString(ExternalCredentials->GetType()),
                FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateSPLambda(
                    this,
                    [State,
                     ExternalCredentials,
                     OnPlatformDone](const Connect::FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                        if (bThisCallIsResponsibleForLogout)
                        {
                            State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                        }
                        if (Data.ResultCode == EOS_EResult::EOS_Success ||
                            Data.ResultCode == EOS_EResult::EOS_InvalidUser)
                        {
                            State->AddEOSConnectCandidateFromExternalCredentials(Data, ExternalCredentials);
                        }
                        else
                        {
                            State->ErrorMessages.Add(FString::Printf(
                                TEXT("GatherEOSAccountsWithNativePlatformCredentialsNode: External credential "
                                     "'%s' failed to authenticate with EOS Connect: %s"),
                                *ExternalCredentials->GetType(),
                                ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                        }

                        OnPlatformDone(true);
                    }));
            return true;
        },
        [OnStepDone](const TArray<bool> &Results) {
            OnStepDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        });
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()