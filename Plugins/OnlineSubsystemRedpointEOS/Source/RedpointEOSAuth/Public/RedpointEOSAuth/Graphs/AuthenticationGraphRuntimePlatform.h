// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3215911224, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_RUNTIME_PLATFORM FName(TEXT("RuntimePlatform"))

/**
 * This authentication graph leverages the runtime platform integration APIs to authenticate a user.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphRuntimePlatform : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    FAuthenticationGraphRuntimePlatform() = default;
    UE_NONCOPYABLE(FAuthenticationGraphRuntimePlatform);
    virtual ~FAuthenticationGraphRuntimePlatform() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(3215911224, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphRuntimePlatform)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()