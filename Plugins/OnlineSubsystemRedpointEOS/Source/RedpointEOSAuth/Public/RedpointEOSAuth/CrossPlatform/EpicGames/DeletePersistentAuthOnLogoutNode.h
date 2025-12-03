// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1202632218, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FDeletePersistentAuthOnLogoutNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FDeletePersistentAuthOnLogoutNode);
    FDeletePersistentAuthOnLogoutNode() = default;
    virtual ~FDeletePersistentAuthOnLogoutNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FDeletePersistentAuthOnLogoutNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1202632218,
    Redpoint::EOS::Auth::CrossPlatform::EpicGames,
    FDeletePersistentAuthOnLogoutNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()