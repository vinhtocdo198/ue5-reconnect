// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "RedpointEOSAPI/SDK.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(184451355, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FGetExternalCredentialsFromLocalPlatformNode : public FAuthenticationGraphNode
{
public:
    FGetExternalCredentialsFromLocalPlatformNode() = default;
    UE_NONCOPYABLE(FGetExternalCredentialsFromLocalPlatformNode);
    virtual ~FGetExternalCredentialsFromLocalPlatformNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FGetExternalCredentialsFromLocalPlatformNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(184451355, Redpoint::EOS::Auth::Nodes, FGetExternalCredentialsFromLocalPlatformNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()