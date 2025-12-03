// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/CrossPlatform/EpicGames/DeletePersistentAuthOnLogoutNode.h"

#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/DeletePersistentAuthNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1202632218, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;

void FDeletePersistentAuthOnLogoutNode::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    // Delete the persistent authentication on logout or failure (we do it on failure as well in case
    // the persistent authentication is preventing sign in from working).
    State->AddLogoutNode(MakeShared<FDeletePersistentAuthNode>());
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()