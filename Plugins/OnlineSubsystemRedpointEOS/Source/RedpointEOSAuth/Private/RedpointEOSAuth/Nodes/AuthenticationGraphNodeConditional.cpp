// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeConditional.h"

#include "RedpointEOSAuth/AuthenticationGraphNodeCollector.h"
#include "RedpointEOSAuth/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2776947903, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FAuthenticationGraphNodeConditional::HandleOnDone(
    EAuthenticationGraphNodeResult Result,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedPtr<FAuthenticationGraphNode> Node,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FAuthenticationGraphNodeOnDone OnDone)
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph exiting node: %s"),
        &State.Get(),
        *Node->GetDebugName());

    OnDone.ExecuteIfBound(Result);
}

bool FAuthenticationGraphNodeConditional::AlwaysTrue(const FAuthenticationGraphState &State)
{
    return true;
}

void FAuthenticationGraphNodeConditional::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    for (const auto &ConditionalNode : this->Conditions)
    {
        checkf(
            ConditionalNode.Key.IsBound(),
            TEXT("Conditional authentication nodes must have a callback bound to the delegate."));
        if (ConditionalNode.Key.Execute(*State))
        {
            TSharedPtr<FAuthenticationGraphNode> Node = ConditionalNode.Value;
            UE_LOG(
                LogRedpointEOSAuth,
                Verbose,
                TEXT("%p: Authentication graph entering node: %s"),
                &State.Get(),
                *Node->GetDebugName());
            Node->Execute(
                State,
                FAuthenticationGraphNodeOnDone::CreateSP(
                    this,
                    &FAuthenticationGraphNodeConditional::HandleOnDone,
                    Node,
                    State,
                    OnDone));
            return;
        }
    }

    // No matching condition, treat as failure.
    OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
}

TSharedRef<FAuthenticationGraphNodeConditional> FAuthenticationGraphNodeConditional::If(
    FAuthenticationGraphCondition Condition,
    TSharedRef<FAuthenticationGraphNode> Node)
{
    this->Conditions.Add(TTuple<FAuthenticationGraphCondition, TSharedPtr<FAuthenticationGraphNode>>(Condition, Node));
    return StaticCastSharedRef<FAuthenticationGraphNodeConditional>(this->AsShared());
}

TSharedRef<FAuthenticationGraphNodeConditional> FAuthenticationGraphNodeConditional::Else(
    TSharedRef<FAuthenticationGraphNode> Node)
{
    this->Conditions.Add(TTuple<FAuthenticationGraphCondition, TSharedPtr<FAuthenticationGraphNode>>(
        FAuthenticationGraphCondition::CreateStatic(&FAuthenticationGraphNodeConditional::AlwaysTrue),
        Node));
    return StaticCastSharedRef<FAuthenticationGraphNodeConditional>(this->AsShared());
}

FString FAuthenticationGraphNodeConditional::GetDebugName() const
{
    return TEXT("FAuthenticationGraphNodeConditional");
}

void FAuthenticationGraphNodeConditional::CollectChildNodes(FAuthenticationGraphNodeCollector &Collector)
{
    for (const auto &KV : this->Conditions)
    {
        Collector.AddNode(KV.Value.ToSharedRef());
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()