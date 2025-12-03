// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4048689343, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FClearEOSCandidatesNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FClearEOSCandidatesNode);
    FClearEOSCandidatesNode() = default;
    virtual ~FClearEOSCandidatesNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FClearEOSCandidatesNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(4048689343, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FClearEOSCandidatesNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()