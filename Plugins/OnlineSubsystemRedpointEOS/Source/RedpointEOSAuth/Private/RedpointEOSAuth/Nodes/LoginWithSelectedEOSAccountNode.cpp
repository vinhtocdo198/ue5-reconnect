// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"

#include "RedpointEOSAPI/Connect/CreateUser.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Nodes/SignOutEOSAccountNode.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1677570301, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FLoginWithSelectedEOSAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
        using namespace ::Redpoint::EOS::Auth::Nodes;

    check(State->HasSelectedEOSCandidate());

    FAuthenticationGraphEOSCandidate Candidate = State->GetSelectedEOSCandidate();
    if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
    {
        // This account is already authenticated, we don't need to call EOS_CreateUser, so
        // just set the result user ID in the state and call it done.
        State->ResultUserId = MakeShared<FUniqueNetIdEOS>(Candidate.ProductUserId);
        State->AddResultUserAuthAttributes(Candidate.UserAuthAttributes);
        State->ResultRefreshCallback = Candidate.RefreshCallback;
        State->ResultExternalCredentials = Candidate.ExternalCredentials;
        State->ResultCrossPlatformAccountId = Candidate.AssociatedCrossPlatformAccountId;
        State->ResultNativeSubsystemName = Candidate.NativeSubsystemName;
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    if (EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
    {
        // If we don't have a continuance token, then this isn't a valid candidate and
        // should not have been selected.
        State->ErrorMessages.Add(TEXT("LoginWithSelectedEOSAccountNode: Reached with unusable candidiate selected."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    // Otherwise, call EOS_CreateUser with the continuance token.
    FCreateUser::Execute(
        State->PlatformHandle,
        FCreateUser::Options{Candidate.ContinuanceToken},
        FCreateUser::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone, Candidate](const FCreateUser::Result &Data) {
                if (Data.ResultCode != EOS_EResult::EOS_Success)
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("LoginWithSelectedEOSAccountNode: Unable to create EOS user: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Data.ResultCode))));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                    return;
                }

                // If we create an account, then we must be responsible for signing it out.
                State->AddLogoutNode(MakeShared<FSignOutEOSAccountNode>(Data.LocalUserId));

                // Success, we have created the EOS account.
                State->ResultUserId = MakeShared<FUniqueNetIdEOS>(Data.LocalUserId);
                State->AddResultUserAuthAttributes(Candidate.UserAuthAttributes);
                State->ResultRefreshCallback = Candidate.RefreshCallback;
                State->ResultCrossPlatformAccountId = Candidate.AssociatedCrossPlatformAccountId;
                State->ResultExternalCredentials = Candidate.ExternalCredentials;
                State->ResultNativeSubsystemName = Candidate.NativeSubsystemName;
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                return;
            }));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()