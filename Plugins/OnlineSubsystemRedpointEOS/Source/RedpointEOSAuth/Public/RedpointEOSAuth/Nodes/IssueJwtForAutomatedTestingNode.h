// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "PlatformHttp.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER && (!defined(UE_BUILD_SHIPPING) || !UE_BUILD_SHIPPING)

namespace REDPOINT_EOS_FILE_NS_ID(3436057713, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FIssueJwtForAutomatedTestingNode : public FAuthenticationGraphNode
{
private:
    void OnHttpResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bConnectedSuccessfully,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    UE_NONCOPYABLE(FIssueJwtForAutomatedTestingNode);
    FIssueJwtForAutomatedTestingNode() = default;
    virtual ~FIssueJwtForAutomatedTestingNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FIssueJwtForAutomatedTestingNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(3436057713, Redpoint::EOS::Auth::Nodes, FIssueJwtForAutomatedTestingNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()