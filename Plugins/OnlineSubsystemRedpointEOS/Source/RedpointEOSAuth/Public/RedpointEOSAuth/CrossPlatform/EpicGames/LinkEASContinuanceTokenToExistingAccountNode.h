// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(722433906, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FLinkEASContinuanceTokenToExistingAccountNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FLinkEASContinuanceTokenToExistingAccountNode);
    FLinkEASContinuanceTokenToExistingAccountNode() = default;
    virtual ~FLinkEASContinuanceTokenToExistingAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FLinkEASContinuanceTokenToExistingAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    722433906,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FLinkEASContinuanceTokenToExistingAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()