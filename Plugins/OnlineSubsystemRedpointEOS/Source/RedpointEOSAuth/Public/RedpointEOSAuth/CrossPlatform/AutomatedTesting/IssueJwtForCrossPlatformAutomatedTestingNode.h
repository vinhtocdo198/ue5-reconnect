// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "PlatformHttp.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(788135354, Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FIssueJwtForCrossPlatformAutomatedTestingNode : public FAuthenticationGraphNode
{
private:
    void OnHttpResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bConnectedSuccessfully,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    UE_NONCOPYABLE(FIssueJwtForCrossPlatformAutomatedTestingNode);
    FIssueJwtForCrossPlatformAutomatedTestingNode() = default;
    virtual ~FIssueJwtForCrossPlatformAutomatedTestingNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FIssueJwtForCrossPlatformAutomatedTestingNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting
{
REDPOINT_EOS_FILE_NS_EXPORT(
    788135354,
    Redpoint::EOS::Auth::CrossPlatform::AutomatedTesting,
    FIssueJwtForCrossPlatformAutomatedTestingNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()