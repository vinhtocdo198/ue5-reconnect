// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/LinkEASContinuanceTokenToExistingAccountNode.h"

#include "RedpointEOSAPI/Connect/LinkAccount.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(722433906, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::API;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FLinkEASContinuanceTokenToExistingAccountNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Connect;
    
    check(State->ExistingUserId.IsValid());
    check(State->ExistingUserId->HasValidProductUserId());
    check(EOSString_EpicAccountId::IsValid(State->GetAuthenticatedEpicAccountId()));

    EOS_ProductUserId ExistingProductUserId = State->ExistingUserId->GetProductUserId();
    EOS_ContinuanceToken EASContinuanceToken = EOSString_ContinuanceToken::None;
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            if (!EOSString_ProductUserId::IsValid(Candidate.ProductUserId) &&
                !EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
            {
                EASContinuanceToken = Candidate.ContinuanceToken;
                break;
            }
        }
    }
    check(!EOSString_ContinuanceToken::IsNone(EASContinuanceToken));

    Connect::FLinkAccount::Execute(
        State->PlatformHandle,
        Connect::FLinkAccount::Options{EASContinuanceToken, ExistingProductUserId},
        Connect::FLinkAccount::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const Connect::FLinkAccount::Result &Info) {
                if (Info.ResultCode != EOS_EResult::EOS_Success)
                {
                    State->ErrorMessages.Add(FString::Printf(
                        TEXT("Link account operation failed with result code %s"),
                        ANSI_TO_TCHAR(EOS_EResult_ToString(Info.ResultCode))));
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
                    return;
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()