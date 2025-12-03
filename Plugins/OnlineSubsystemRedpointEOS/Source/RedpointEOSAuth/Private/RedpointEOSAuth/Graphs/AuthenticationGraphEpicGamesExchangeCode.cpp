// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphEpicGamesExchangeCode.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/ChainEASResultToEOSNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/PerformInteractiveEASLoginNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryExchangeCodeAuthenticationNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_CrossPlatformAccountPresent.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1852378913, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth::CrossPlatform::EpicGames;
using namespace ::Redpoint::EOS::Auth::Nodes;

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphEpicGamesExchangeCode::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    // Force the cross-platform provider to be Epic Games if we are using this provider.
    InitialState->CrossPlatformAccountProvider = MakeShared<CrossPlatform::FEpicGamesCrossPlatformAccountProvider>();

    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
        ->Add(MakeShared<FAuthenticationGraphNodeUntil_Forever>()
                  ->Add(MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>(
                            TEXT("Unable to sign in to Epic Games. Please restart the game."))
                            ->Add(MakeShared<FTryExchangeCodeAuthenticationNode>())
                            ->Add(MakeShared<FPerformInteractiveEASLoginNode>()))
                  ->Add(MakeShared<FChainEASResultToEOSNode>()))
        ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
        ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>());
}

void FAuthenticationGraphEpicGamesExchangeCode::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_EPIC_GAMES_EXCHANGE_CODE,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_EpicGamesExchangeCode", "Epic Games Exchange Code"),
        MakeShared<FAuthenticationGraphEpicGamesExchangeCode>());
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()