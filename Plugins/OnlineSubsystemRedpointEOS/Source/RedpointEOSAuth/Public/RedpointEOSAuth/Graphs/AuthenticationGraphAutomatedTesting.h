// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2581304592, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_AUTOMATED_TESTING FName(TEXT("AutomatedTesting"))
#define EOS_AUTH_GRAPH_AUTOMATED_TESTING_OSS FName(TEXT("AutomatedTestingOSS"))

/**
 * This authentication graph is used internally when the unit tests are running.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphAutomatedTesting : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphAutomatedTesting);
    FAuthenticationGraphAutomatedTesting() = default;
    virtual ~FAuthenticationGraphAutomatedTesting() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(2581304592, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphAutomatedTesting)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()