// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAuth/AuthenticationGraphCondition.h"
#include "RedpointEOSAuth/AuthenticationGraphNode.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(797784821, Redpoint::EOS::Auth::Nodes)
{
using namespace ::Redpoint::EOS::Auth;

class REDPOINTEOSAUTH_API FAuthenticationGraphNodeUntil : public FAuthenticationGraphNode
{
private:
    TArray<TSharedRef<FAuthenticationGraphNode>> Sequence;
    TSharedPtr<FAuthenticationGraphState> CurrentState;
    int CurrentSequenceNum;
    FAuthenticationGraphNodeOnDone CurrentSequenceDone;
    FAuthenticationGraphCondition UntilCondition;
    FString ErrorMessage;
#if WITH_EDITOR
    FString EditorSignalContext;
    FString EditorSignalId;
#endif

    void Continue(EAuthenticationGraphNodeResult Result, TSharedRef<FAuthenticationGraphState> State);
    void HandleOnDone(
        EAuthenticationGraphNodeResult Result,
        TSharedPtr<FAuthenticationGraphNode> Node,
        TSharedRef<FAuthenticationGraphState> State);

protected:
    FAuthenticationGraphNodeUntil() = delete;
    FAuthenticationGraphNodeUntil(
        FAuthenticationGraphCondition InUntilCondition,
        FString InErrorMessage = TEXT(""),
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        FString InEditorSignalContext = TEXT(""),
        // NOLINTNEXTLINE(performance-unnecessary-value-param)
        FString InEditorSignalId = TEXT(""));
    UE_NONCOPYABLE(FAuthenticationGraphNodeUntil);

    virtual void Execute(TSharedRef<FAuthenticationGraphState> State, FAuthenticationGraphNodeOnDone OnDone) final;
    virtual bool RequireConditionPass() const;

public:
    virtual ~FAuthenticationGraphNodeUntil() override = default;

    TSharedRef<FAuthenticationGraphNodeUntil> Add(const TSharedRef<FAuthenticationGraphNode> &Node);

    virtual void CollectChildNodes(FAuthenticationGraphNodeCollector &Collector) override final;
};

}

namespace Redpoint::EOS::Auth::Nodes
{
REDPOINT_EOS_FILE_NS_EXPORT(797784821, Redpoint::EOS::Auth::Nodes, FAuthenticationGraphNodeUntil)
}

#endif

REDPOINT_EOS_CODE_GUARD_END()