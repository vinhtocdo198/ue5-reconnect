// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(2484283026, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FPerformAutomatedTestingEASLoginNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FPerformAutomatedTestingEASLoginNode);
    FPerformAutomatedTestingEASLoginNode() = default;
    virtual ~FPerformAutomatedTestingEASLoginNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPerformAutomatedTestingEASLoginNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2484283026,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FPerformAutomatedTestingEASLoginNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()