// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/ClearEOSCandidatesNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSCore/Id/UniqueNetIdEOS.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4048689343, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FClearEOSCandidatesNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    State->EOSCandidates.Empty();
    State->AuthenticatedCrossPlatformAccountId.Reset();
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()