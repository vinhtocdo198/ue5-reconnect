// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphTestUserInterface.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil_Forever.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"
#include "RedpointEOSAuth/Nodes/PromptToSignInOrCreateAccountNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1819072263, Redpoint::EOS::Auth::Graphs)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphTestUserInterface::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    return MakeShared<Nodes::FAuthenticationGraphNodeUntil_Forever>()
        ->Add(MakeShared<Nodes::FPromptToSignInOrCreateAccountNode>())
        ->Add(MakeShared<Nodes::FFailAuthenticationNode>(
            TEXT("This authentication graph intentionally errors after selecting an option, as it is used for testing "
                 "the user interface in XR/VR.")));
}

void FAuthenticationGraphTestUserInterface::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_TEST_USER_INTERFACE,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_TestUserInterface", "Test User Interface for XR/VR"),
        MakeShared<FAuthenticationGraphTestUserInterface>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()