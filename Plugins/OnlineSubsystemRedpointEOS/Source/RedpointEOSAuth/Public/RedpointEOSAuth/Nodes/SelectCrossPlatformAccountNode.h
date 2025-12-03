// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraph.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2887753562, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

enum ESelectCrossPlatformAccountMode
{
    /**
     * The default behaviour; the cross-platform account will be selected, even if there's no
     * EOS account for it.
     */
    PermitContinuanceToken,

    /**
     * If the cross-platform account is defined by a continuance token (no EOS account), then
     * authentication will fail with an error.
     */
    ExistingAccountOnly,
};

class REDPOINTEOSAUTH_API FSelectCrossPlatformAccountNode : public FAuthenticationGraphNode
{
private:
    ESelectCrossPlatformAccountMode Mode;

public:
    UE_NONCOPYABLE(FSelectCrossPlatformAccountNode);
    FSelectCrossPlatformAccountNode(
        ESelectCrossPlatformAccountMode InMode = ESelectCrossPlatformAccountMode::PermitContinuanceToken)
        : Mode(InMode){};
    virtual ~FSelectCrossPlatformAccountNode() override = default;

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) override;

    virtual FString GetDebugName() const override
    {
        return TEXT("FSelectCrossPlatformAccountNode");
    }
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2887753562, Redpoint::EOS::Auth::Nodes, ESelectCrossPlatformAccountMode)
REDPOINT_EOS_FILE_NS_EXPORT(2887753562, Redpoint::EOS::Auth::Nodes, FSelectCrossPlatformAccountNode)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()