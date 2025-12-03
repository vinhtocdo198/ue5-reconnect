// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/TryDeviceIdAuthenticationNode.h"

#include "RedpointEOSAuth/AuthenticationConstants.h"
#include "RedpointEOSAuth/AuthenticationGraphRefreshEOSCredentials.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/AuthenticationHelpersConnect.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSConfig/Config.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(77089298, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

void FTryDeviceIdAuthenticationNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    using namespace ::Redpoint::EOS::Auth::Nodes;

    FAuthenticationGraphRefreshEOSCredentials RefreshCallback = FAuthenticationGraphRefreshEOSCredentials::CreateLambda(
        [](const TSharedPtr<FAuthenticationGraphRefreshEOSCredentialsInfo> &Info) {
            UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("Refreshing device ID login for user..."));
            FAuthenticationHelpersConnect::Login(
                Info->PlatformHandle,
                TEXT("Anonymous"),
                TEXT(""),
                TEXT("Anonymous"),
                EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN,
                FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateLambda(
                    [Info](const Connect::FLogin::Result &Data, bool) {
                        // FEOSAuthentication does EOS_Connect_Login, which is all we need to do.
                        if (Data.ResultCode == EOS_EResult::EOS_Success)
                        {
                            UE_LOG(
                                LogRedpointEOSAuth,
                                Verbose,
                                TEXT("Successfully refreshed device ID login for user"));
                            Info->OnComplete.ExecuteIfBound(true);
                        }
                        else
                        {
                            UE_LOG(
                                LogRedpointEOSAuth,
                                Error,
                                TEXT("Failed to refresh device ID login for user, got result code: %s"),
                                ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode)));
                            Info->OnComplete.ExecuteIfBound(false);
                        }
                    }));
        });

    FAuthenticationHelpersConnect::Login(
        State->PlatformHandle,
        TEXT("Anonymous"),
        TEXT(""),
        TEXT("Anonymous"),
        EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN,
        FAuthenticationHelpersConnect::FLoginCompletionDelegate::CreateSPLambda(
            this,
            [State,
             OnDone,
             RefreshCallback](const Connect::FLogin::Result &Data, bool bThisCallIsResponsibleForLogout) {
                if (bThisCallIsResponsibleForLogout)
                {
                    State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));
                }
                if (Data.ResultCode == EOS_EResult::EOS_InvalidUser || Data.ResultCode == EOS_EResult::EOS_Success)
                {
                    TMap<FString, FString> Attributes;
                    Attributes.Add(EOS_AUTH_ATTRIBUTE_AUTHENTICATEDWITH, TEXT("deviceId"));
                    State->AddEOSConnectCandidate(
                        NSLOCTEXT("OnlineSubsystemRedpointEOS", "PlatformDisplayName_DeviceId", "This Device"),
                        Attributes,
                        Data,
                        RefreshCallback,
                        NAME_None,
                        EAuthenticationGraphEOSCandidateType::DeviceId);
                }
                else
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("Error while authenticating against device ID: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()