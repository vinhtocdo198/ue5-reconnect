// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"
#include "PlatformHttp.h"
#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1215427675, Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FGetJwtForSimpleFirstPartyNode : public FAuthenticationGraphNode
{
private:
    void OnHttpResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bConnectedSuccessfully,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

public:
    UE_NONCOPYABLE(FGetJwtForSimpleFirstPartyNode);
    FGetJwtForSimpleFirstPartyNode() = default;
    virtual ~FGetJwtForSimpleFirstPartyNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FGetJwtForSimpleFirstPartyNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1215427675,
    Redpoint::EOS::Auth::CrossPlatform::SimpleFirstParty,
    FGetJwtForSimpleFirstPartyNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()