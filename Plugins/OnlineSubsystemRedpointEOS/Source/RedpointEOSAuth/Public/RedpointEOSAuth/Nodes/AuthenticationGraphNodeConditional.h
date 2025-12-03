// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphCondition.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(2776947903, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeConditional : public FAuthenticationGraphNode
{
private:
    TArray<TTuple<FAuthenticationGraphCondition, TSharedPtr<FAuthenticationGraphNode>>> Conditions;

    void HandleOnDone(
        EAuthenticationGraphNodeResult Result,
        TSharedPtr<FAuthenticationGraphNode> Node,
        TSharedRef<FAuthenticationGraphState> State,
        FAuthenticationGraphNodeOnDone OnDone);

    static bool AlwaysTrue(const FAuthenticationGraphState &State);

protected:
    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) final;

public:
    FAuthenticationGraphNodeConditional() = default;
    UE_NONCOPYABLE(FAuthenticationGraphNodeConditional);
    virtual ~FAuthenticationGraphNodeConditional() override = default;

    TSharedRef<FAuthenticationGraphNodeConditional> If(
        FAuthenticationGraphCondition Condition,
        TSharedRef<FAuthenticationGraphNode> Node);
    TSharedRef<FAuthenticationGraphNodeConditional> Else(TSharedRef<FAuthenticationGraphNode> Node);

    virtual FString GetDebugName() const override;

    virtual void CollectChildNodes(FAuthenticationGraphNodeCollector &Collector) override;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(2776947903, Redpoint::EOS::Auth::Nodes, FAuthenticationGraphNodeConditional)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()