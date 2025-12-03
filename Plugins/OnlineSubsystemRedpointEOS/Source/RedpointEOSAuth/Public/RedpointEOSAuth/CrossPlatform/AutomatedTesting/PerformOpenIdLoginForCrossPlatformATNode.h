// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/SDK.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(365535988, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode);
    FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode() = default;
    virtual ~FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting
{
REDPOINT_EOS_FILE_NS_EXPORT(
    365535988,
    Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting,
    FPerformOpenIdLoginForCrossPlatformAutomatedTestingNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()