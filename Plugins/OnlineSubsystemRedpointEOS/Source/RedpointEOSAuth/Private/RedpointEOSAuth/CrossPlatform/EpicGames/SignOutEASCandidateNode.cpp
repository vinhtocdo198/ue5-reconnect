// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/SignOutEASCandidateNode.h"

#include "RedpointEOSAPI/Auth/Logout.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/WeakPtrHelpers.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4260767515, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;
using namespace ::Redpoint::EOS::API;

void FSignOutEASCandidateNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    using namespace ::Redpoint::EOS::API::Auth;

    TSharedPtr<const FEpicGamesCrossPlatformAccountId> AccountToSignOut = nullptr;
    for (int i = State->EOSCandidates.Num() - 1; i >= 0; i--)
    {
        auto Candidate = State->EOSCandidates[i];
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            checkf(!AccountToSignOut.IsValid(), TEXT("Multiple EAS candidates in authentication state!"));
            AccountToSignOut =
                StaticCastSharedPtr<const FEpicGamesCrossPlatformAccountId>(Candidate.AssociatedCrossPlatformAccountId);
            State->EOSCandidates.RemoveAt(i);
        }
    }

    if (AccountToSignOut == nullptr)
    {
        checkf(
            !State->AuthenticatedCrossPlatformAccountId.IsValid(),
            TEXT("Cross-platform account ID is set, but no EAS candidate was found."));
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
        return;
    }

    checkf(
        AccountToSignOut->Compare(*State->AuthenticatedCrossPlatformAccountId),
        TEXT("Authenticated cross-platform account ID must match the candidate's ID"));

    FLogout::Execute(
        State->PlatformHandle,
        FLogout::Options{AccountToSignOut->GetEpicAccountId()},
        FLogout::CompletionDelegate::CreateSPLambda(
            this,
            [State, OnDone](const FLogout::Result &Info) {
                if (Info.ResultCode != EOS_EResult::EOS_Success)
                {
                    State->ErrorMessages.Add(
                        FString::Printf(TEXT("Unable to sign out Epic Games account when required")));
                    auto Err = ConvertError(
                        TEXT("FSignOutEASCandidateNode::Execute"),
                        TEXT("Unable to sign out Epic Games account when required."),
                        Info.ResultCode);
                    UE_LOG(LogRedpointEOSAuth, Error, TEXT("%s"), *Err.ToLogString());
                    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
                    return;
                }

                State->AuthenticatedCrossPlatformAccountId.Reset();
                OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }));
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()