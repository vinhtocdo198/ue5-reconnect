// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountProvider.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1848772398, Redpoint::EOS::Auth::CrossPlatform::EpicGames)
{
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::Auth::CrossPlatform;

class REDPOINTEOSAUTH_API FSignOutEASAccountNode : public FAuthenticationGraphNode
{
private:
    TSharedRef<const FEpicGamesCrossPlatformAccountId> AccountId;

public:
    UE_NONCOPYABLE(FSignOutEASAccountNode);
    FSignOutEASAccountNode(const TSharedRef<const FEpicGamesCrossPlatformAccountId> &InAccountId)
        : AccountId(InAccountId){};
    virtual ~FSignOutEASAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSignOutEASAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::CrossPlatform::EpicGames
{
REDPOINT_EOS_FILE_NS_EXPORT(1848772398, Redpoint::EOS::Auth::CrossPlatform::EpicGames, FSignOutEASAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()