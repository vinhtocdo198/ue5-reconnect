// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/LinkUnconnectedEOSAccountToSignedInEASAccountNode.h"

#include "RedpointEOSAPI/Connect/LinkAccount.h"
#include "RedpointEOSAuth/AuthenticationGraphEOSCandidateType.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2677171652, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FLinkUnconnectedEOSAccountToSignedInEASAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    
    // For this node, we expect:
    //
    // - An EOS candidate that is an Epic Games account, which is currently signed in as per the
    // State->AuthenticatedEpicAccountId.
    // - An EOS candidate for the native platform that has a continuance token (and is not a signed in account).
    //

    if (State->EOSCandidates.Num() != 2)
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    bool bHasEASCandidate = false;
    bool bHasContinuanceCandidate = false;
    EOS_ContinuanceToken ContinuanceToken = {};
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform &&
            State->GetSelectedEOSCandidate().Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            bHasEASCandidate = true;
        }
        else if (
            Candidate.Type == EAuthenticationGraphEOSCandidateType::Generic && Candidate.ContinuanceToken != nullptr)
        {
            bHasContinuanceCandidate = true;
            ContinuanceToken = Candidate.ContinuanceToken;
        }
    }

    if (!bHasEASCandidate || !bHasContinuanceCandidate)
    {
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    Connect::FLinkAccount::Execute(
        State->PlatformHandle,
        Connect::FLinkAccount::Options{ContinuanceToken, State->GetSelectedEOSCandidate().ProductUserId},
        Connect::FLinkAccount::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const Connect::FLinkAccount::Result &Info) {
                if (Info.ResultCode != EOS_EResult::EOS_Success)
                {
                    UE_LOG(
                        LogRedpointEOSAuth,
                        Error,
                        TEXT("LinkUnconnectedEOSAccountToSignedInEASAccountNode: Unable to associate native credential "
                             "with EOS account signed in with Epic Games: %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Info.ResultCode)));
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()