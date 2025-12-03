// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4260767515, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FSignOutEASCandidateNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FSignOutEASCandidateNode);
    FSignOutEASCandidateNode() = default;
    virtual ~FSignOutEASCandidateNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSignOutEASCandidateNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(4260767515, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FSignOutEASCandidateNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()