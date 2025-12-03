// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2588657432, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_DEV_AUTH_TOOL FName(TEXT("DevAuthTool"))

/**
 * This authentication graph is used internally when "Login before play-in-editor" is enabled.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphDevAuthTool : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphDevAuthTool);
    FAuthenticationGraphDevAuthTool() = default;
    virtual ~FAuthenticationGraphDevAuthTool() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(2588657432, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphDevAuthTool)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()