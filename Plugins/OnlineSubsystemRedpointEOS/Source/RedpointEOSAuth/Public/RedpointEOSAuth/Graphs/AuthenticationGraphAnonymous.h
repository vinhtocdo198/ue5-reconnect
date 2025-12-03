// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3200112234, Redpoint::EOS::Auth::Graphs)
{
using namespace ::Redpoint::EOS::Auth;

#define EOS_AUTH_GRAPH_ANONYMOUS FName(TEXT("Anonymous"))

/**
 * This authentication graph performs anonymous authentication and nothing else. Suitable on mobile devices.
 */
class REDPOINTEOSAUTH_API FAuthenticationGraphAnonymous : public FAuthenticationGraph
{
protected:
    virtual TSharedRef<FAuthenticationGraphNode> CreateGraph(
        const TSharedRef<FAuthenticationGraphState> &InitialState) const override;

public:
    UE_NONCOPYABLE(FAuthenticationGraphAnonymous);
    FAuthenticationGraphAnonymous() = default;
    virtual ~FAuthenticationGraphAnonymous() override = default;

    static void Register();
};

}

namespace Redpoint::EOS::Auth::Graphs
{
REDPOINT_EOS_FILE_NS_EXPORT(3200112234, Redpoint::EOS::Auth::Graphs, FAuthenticationGraphAnonymous)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()