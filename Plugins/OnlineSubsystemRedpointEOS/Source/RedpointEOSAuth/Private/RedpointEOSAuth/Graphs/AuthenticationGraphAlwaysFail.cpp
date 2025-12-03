// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Graphs/AuthenticationGraphAlwaysFail.h"

#include "RedpointEOSAuth/AuthenticationGraphRegistry.h"
#include "RedpointEOSAuth/Nodes/FailAuthenticationNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3873693879, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

TSharedRef<FAuthenticationGraphNode> FAuthenticationGraphAlwaysFail::CreateGraph(
    const TSharedRef<FAuthenticationGraphState> &InitialState) const
{
    return MakeShared<Nodes::FFailAuthenticationNode>(
        TEXT("There is no usable authentication mechanism on this platform."));
}

void FAuthenticationGraphAlwaysFail::Register()
{
    FAuthenticationGraphRegistry::Register(
        EOS_AUTH_GRAPH_ALWAYS_FAIL,
        NSLOCTEXT("OnlineSubsystemRedpointEOS", "AuthGraph_AlwaysFail", "Always Fail"),
        MakeShared<FAuthenticationGraphAlwaysFail>());
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()