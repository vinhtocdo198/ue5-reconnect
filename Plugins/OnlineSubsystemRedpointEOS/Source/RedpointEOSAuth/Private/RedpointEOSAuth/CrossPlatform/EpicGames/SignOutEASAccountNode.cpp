// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASAccountNode.h"

#include "RedpointEOSAPI/Auth/Logout.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1848772398, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

void FSignOutEASAccountNode::Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;
    
    // Remove any candidates related to this account.
    for (int i = State->EOSCandidates.Num() - 1; i >= 0; i--)
    {
        auto Candidate = State->EOSCandidates[i];
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform &&
            Candidate.AssociatedCrossPlatformAccountId.IsValid() &&
            Candidate.AssociatedCrossPlatformAccountId->GetType().IsEqual(EPIC_GAMES_ACCOUNT_ID) &&
            this->AccountId->Compare(*Candidate.AssociatedCrossPlatformAccountId))
        {
            State->EOSCandidates.RemoveAt(i);
        }
    }

    // Clear the authenticated cross-platform account ID, if it's set to this account.
    if (State->AuthenticatedCrossPlatformAccountId.IsValid() &&
        this->AccountId->Compare(*State->AuthenticatedCrossPlatformAccountId))
    {
        State->AuthenticatedCrossPlatformAccountId.Reset();
    }

    // Sign out of the EAS account.
    FLogout::Execute(
        State->PlatformHandle,
        FLogout::Options{this->AccountId->GetEpicAccountId()},
        FLogout::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FLogout::Result &Info) {
                UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("Received EOS_Auth_Logout callback."));

                if (Info.ResultCode != EOS_EResult::EOS_Success)
                {
                    auto Err = ConvertError(
                        TEXT("FSignOutEASAccountNode::Execute"),
                        TEXT("Unable to sign out Epic Games account when required."),
                        Info.ResultCode);
                    UE_LOG(LogRedpointEOSAuth, Error, TEXT("%s"), *Err.ToLogString());
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                    return;
                }

                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()