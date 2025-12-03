// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/Nodes/AuthenticationGraphNodeUntil.h"

#include "RedpointEOSAuth/AuthenticationGraphNodeCollector.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Editor/EditorSignalling.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(797784821, Redpoint::EOS::Auth::Nodes)
{

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

void FAuthenticationGraphNodeUntil::Continue(
    EAuthenticationGraphNodeResult Result,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State)
{
    checkf(
        this->UntilCondition.IsBound(),
        TEXT("'Until' authentication nodes must have a callback bound to the until condition delegate."));
    if (Result == EAuthenticationGraphNodeResult::Error || this->UntilCondition.Execute(*this->CurrentState) ||
        this->CurrentSequenceNum >= this->Sequence.Num() - 1)
    {
        // Copy state and done handler out first, then reset them in case the OnDone
        // re-enters this sequence.
        TSharedPtr<FAuthenticationGraphState> CachedState = this->CurrentState;
        FAuthenticationGraphNodeOnDone CachedDone = this->CurrentSequenceDone;
        this->CurrentSequenceNum = -1;
        this->CurrentState = nullptr;
        this->CurrentSequenceDone = FAuthenticationGraphNodeOnDone();
        if (Result == EAuthenticationGraphNodeResult::Continue && this->RequireConditionPass())
        {
            if (!this->UntilCondition.Execute(*CachedState))
            {
                if (this->ErrorMessage.IsEmpty())
                {
                    CachedState->ErrorMessages.Add(
                        FString::Printf(TEXT("Condition was not met in sequence node: %s"), *this->GetDebugName()));
                }
                else
                {
                    CachedState->ErrorMessages.Add(this->ErrorMessage);
                }
#if WITH_EDITOR
                if (!this->EditorSignalContext.IsEmpty())
                {
                    Redpoint::EOS::Core::Editor::FEditorSignalling::OnEditorCustomSignal().Broadcast(
                        this->EditorSignalContext,
                        this->EditorSignalId);
                }
#endif
                CachedDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
            }
            else
            {
                CachedDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Continue);
            }
        }
        else
        {
            CachedDone.ExecuteIfBound(Result);
        }
        return;
    }

    this->CurrentSequenceNum++;
    TSharedPtr<FAuthenticationGraphNode> Node = this->Sequence[this->CurrentSequenceNum];
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph entering node: %s"),
        &State.Get(),
        *Node->GetDebugName());
    Node->Execute(
        this->CurrentState.ToSharedRef(),
        FAuthenticationGraphNodeOnDone::CreateSP(this, &FAuthenticationGraphNodeUntil::HandleOnDone, Node, State));
}

void FAuthenticationGraphNodeUntil::HandleOnDone(
    EAuthenticationGraphNodeResult Result,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedPtr<FAuthenticationGraphNode> Node,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    TSharedRef<FAuthenticationGraphState> State)
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph exiting node: %s"),
        &State.Get(),
        *Node->GetDebugName());

    this->Continue(Result, State);
}

FAuthenticationGraphNodeUntil::FAuthenticationGraphNodeUntil(
    FAuthenticationGraphCondition InUntilCondition,
    FString InErrorMessage,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FString InEditorSignalContext,
    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    FString InEditorSignalId)
    : CurrentState(nullptr)
    , CurrentSequenceNum(0)
    , UntilCondition(MoveTemp(InUntilCondition))
    , ErrorMessage(MoveTemp(InErrorMessage))
#if WITH_EDITOR
    , EditorSignalContext(MoveTemp(InEditorSignalContext))
    , EditorSignalId(MoveTemp(InEditorSignalId))
#endif
{
}

void FAuthenticationGraphNodeUntil::Execute(
    TSharedRef<FAuthenticationGraphState> State,
    FAuthenticationGraphNodeOnDone OnDone)
{
    if (this->Sequence.Num() == 0)
    {
        // No steps configured.
        OnDone.ExecuteIfBound(EAuthenticationGraphNodeResult::Error);
        return;
    }

    check(!this->CurrentState.IsValid());
    this->CurrentSequenceNum = 0;
    this->CurrentState = State;
    this->CurrentSequenceDone = OnDone;

    TSharedPtr<FAuthenticationGraphNode> Node = this->Sequence[this->CurrentSequenceNum];
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph entering node: %s"),
        &State.Get(),
        *Node->GetDebugName());
    Node->Execute(
        this->CurrentState.ToSharedRef(),
        FAuthenticationGraphNodeOnDone::CreateSP(this, &FAuthenticationGraphNodeUntil::HandleOnDone, Node, State));
}

bool FAuthenticationGraphNodeUntil::RequireConditionPass() const
{
    return true;
}

TSharedRef<FAuthenticationGraphNodeUntil> FAuthenticationGraphNodeUntil::Add(
    const TSharedRef<FAuthenticationGraphNode> &Node)
{
    this->Sequence.Add(Node);
    return StaticCastSharedRef<FAuthenticationGraphNodeUntil>(this->AsShared());
}

void FAuthenticationGraphNodeUntil::CollectChildNodes(FAuthenticationGraphNodeCollector &Collector)
{
    for (const auto &Node : this->Sequence)
    {
        Collector.AddNode(Node);
    }
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()