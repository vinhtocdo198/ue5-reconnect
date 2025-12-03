// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2798274585, Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FPerformOpenIdLoginForCrossPlatformFPNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FPerformOpenIdLoginForCrossPlatformFPNode);
    FPerformOpenIdLoginForCrossPlatformFPNode() = default;
    virtual ~FPerformOpenIdLoginForCrossPlatformFPNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FPerformOpenIdLoginForCrossPlatformFPNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2798274585,
    Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty,
    FPerformOpenIdLoginForCrossPlatformFPNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()