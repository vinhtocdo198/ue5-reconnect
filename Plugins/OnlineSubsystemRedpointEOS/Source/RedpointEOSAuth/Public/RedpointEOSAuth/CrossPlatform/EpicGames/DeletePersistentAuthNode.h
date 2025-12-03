// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(3012757551, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FDeletePersistentAuthNode : public FAuthenticationGraphNode
{
public:
    UE_NONCOPYABLE(FDeletePersistentAuthNode);
    FDeletePersistentAuthNode() = default;
    virtual ~FDeletePersistentAuthNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FDeletePersistentAuthNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(3012757551, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FDeletePersistentAuthNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()