// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3873693879, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_ALWAYS_FAIL FName(TEXT("AlwaysFail"))

/**
 * This authentication graph always fails, and is used when there is no viable authentication graph to use.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphAlwaysFail : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphAlwaysFail);
    FAuthenticationGraphAlwaysFail() = default;
    virtual ~FAuthenticationGraphAlwaysFail() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(3873693879, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphAlwaysFail)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()