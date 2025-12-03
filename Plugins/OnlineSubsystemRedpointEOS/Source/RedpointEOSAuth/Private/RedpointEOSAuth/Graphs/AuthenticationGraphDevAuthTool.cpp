// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphDevAuthTool.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/ChainEASResultToEOSNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDefaultDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGames/TryPIEDeveloperAuthenticationEASCredentialsNode.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_CrossPlatformAccountPresent.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_LoginComplete.h"
#include "RedpointEOSAuth/Nodes/BailIfAlreadyAuthenticatedNode.h"
#include "RedpointEOSAuth/Nodes/LoginWithSelectedEOSAccountNode.h"
#include "RedpointEOSAuth/Nodes/SelectCrossPlatformAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2588657432, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::Nodes;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphDevAuthTool::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    // Force the cross-platform provider to be Epic Games if we are using this provider.
    InitialState->CrossPlatformAccountProvider = MakeShared<CrossPlatform::FEpicGamesCrossPlatformAccountProvider>();

    return MakeShared<FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<FBailIfAlreadyAuthenticatedNode>())
        ->Add(
            MakeShared<FAuthenticationGraphNodeUntil_LoginComplete>(
                TEXT("The credential provided by the Developer Authentication Tool could not be used. Credentials in "
                     "the Developer Authentication Tool expire after about 4 hours, so please restart the Developer "
                     "Authentication Tool and try again."))
                ->Add(
                    MakeShared<FAuthenticationGraphNodeUntil_CrossPlatformAccountPresent>(
                        TEXT("Unable to authenticate with Developer Authentication Tool. Ensure you have started "
                             "it using "
                             "the EOS dropdown in the toolbar."),
                        TEXT("Authentication"),
                        TEXT("UnableToAuthenticateWithDevTool"))
                        ->Add(MakeShared<CrossPlatform::EpicGames::FTryPIEDeveloperAuthenticationEASCredentialsNode>())
                        ->Add(MakeShared<
                              CrossPlatform::EpicGames::FTryDefaultDeveloperAuthenticationEASCredentialsNode>()))
                ->Add(MakeShared<CrossPlatform::EpicGames::FChainEASResultToEOSNode>())
                ->Add(MakeShared<FSelectCrossPlatformAccountNode>())
                ->Add(MakeShared<FLoginWithSelectedEOSAccountNode>()));
}

void FAuthenticationGraphDevAuthTool::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_DEV_AUTH_TOOL,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_DevAuthTool", "Developer Authentication Tool Only"),
        MakeShared<FAuthenticationGraphDevAuthTool>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()