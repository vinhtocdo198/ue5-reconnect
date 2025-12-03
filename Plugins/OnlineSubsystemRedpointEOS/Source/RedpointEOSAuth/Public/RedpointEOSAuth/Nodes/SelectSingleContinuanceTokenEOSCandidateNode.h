// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4072958602, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FSelectSingleContinuanceTokenEOSCandidateNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FSelectSingleContinuanceTokenEOSCandidateNode);
    FSelectSingleContinuanceTokenEOSCandidateNode() = default;
    virtual ~FSelectSingleContinuanceTokenEOSCandidateNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSelectSingleContinuanceTokenEOSCandidateNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(4072958602, Redpoint::EOS::Auth::Nodes, FSelectSingleContinuanceTokenEOSCandidateNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()