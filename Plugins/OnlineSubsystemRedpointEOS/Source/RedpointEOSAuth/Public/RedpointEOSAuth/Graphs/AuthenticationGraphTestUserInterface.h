// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1819072263, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_TEST_USER_INTERFACE FName(TEXT("TestUserInterface"))

/**
 * This authentication graph presents user interface widgets without going through authentication, so that you can test
 * UI behaviour in VR without setting up accounts in a specific way.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphTestUserInterface : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphTestUserInterface);
    FAuthenticationGraphTestUserInterface() = default;
    virtual ~FAuthenticationGraphTestUserInterface() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(1819072263, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphTestUserInterface)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()