// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/FailAuthenticationDueToConflictingAccountsNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2332495194, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FFailAuthenticationDueToConflictingAccountsNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    FString ExistingLocalEOSAccountId = State->ExistingUserId->ToString();
    FString CrossPlatformAccountId = State->AuthenticatedCrossPlatformAccountId->ToString();
    FString CrossPlatformEOSAccountId = TEXT("Unavailable");
    for (const auto &Candidate : State->EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
            {
                verifyf(
                    EOSString_ProductUserId::ToString(Candidate.ProductUserId, CrossPlatformEOSAccountId) ==
                        EOS_EResult::EOS_Success,
                    TEXT("Unable to convert PUID for error message"));
                break;
            }
        }
    }

    State->ErrorMessages.Add(FString::Printf(
        TEXT("The specified cross-platform account has already played this title, so you can not "
             "link it against the game data of this local account. Please contact game support with the following "
             "information: \n"
             " - Local EOS account PUID: %s\n"
             " - Cross-platform account ID: %s\n"
             " - Cross-platform EOS account PUID: %s"),
        *ExistingLocalEOSAccountId,
        *CrossPlatformAccountId,
        *CrossPlatformEOSAccountId));
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()