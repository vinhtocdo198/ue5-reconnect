// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3513844636, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FTryExchangeCodeAuthenticationNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FTryExchangeCodeAuthenticationNode);
    FTryExchangeCodeAuthenticationNode() = default;
    virtual ~FTryExchangeCodeAuthenticationNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FTryExchangeCodeAuthenticationNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3513844636,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FTryExchangeCodeAuthenticationNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()