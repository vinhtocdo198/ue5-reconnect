// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2028868234, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_CROSS_PLATFORM_ONLY FName(TEXT("CrossPlatformOnly"))

/**
 * This authentication graph only uses the cross-platform account provider to authenticate a user, and never obtains
 * external credentials.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphCrossPlatformOnly : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    FAuthenticationGraphCrossPlatformOnly() = default;
    UE_NONCOPYABLE(FAuthenticationGraphCrossPlatformOnly);
    virtual ~FAuthenticationGraphCrossPlatformOnly() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(2028868234, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphCrossPlatformOnly)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()