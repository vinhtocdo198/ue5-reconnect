// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSAuth/AuthenticationGraph.h"

#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSAuth/AuthenticationGraphEOSCandidateType.h"
#include "RedpointEOSAuth/AuthenticationGraphNodeCollector.h"
#include "RedpointEOSAuth/AuthenticationGraphState.h"
#include "RedpointEOSAuth/Logging.h"
#include "RedpointEOSCore/Legacy/EOSString.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Auth
{

REDPOINT_EOS_DECLARE_CYCLE_STAT(Auth, TEXT("Done"), Done);
REDPOINT_EOS_DECLARE_CYCLE_STAT(Auth, TEXT("Logout"), Logout);
REDPOINT_EOS_DEFINE_STAT(Auth, Done);
REDPOINT_EOS_DEFINE_STAT(Auth, Logout);

}

namespace REDPOINT_EOS_FILE_NS_ID(591988451, Redpoint::EOS::Auth)
{
using namespace ::Redpoint::EOS::Core::Utils;
using namespace ::Redpoint::EOS::Auth;
using namespace ::Redpoint::EOS::API;

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

bool FAuthenticationGraph::Condition_Unauthenticated(const FAuthenticationGraphState &State)
{
    return !State.ExistingUserId.IsValid();
}

bool FAuthenticationGraph::Condition_HasCrossPlatformAccountProvider(const FAuthenticationGraphState &State)
{
    return State.CrossPlatformAccountProvider.IsValid();
}

bool FAuthenticationGraph::Condition_RequireCrossPlatformAccount(const FAuthenticationGraphState &State)
{
    return State.Config->GetRequireCrossPlatformAccount();
}

bool FAuthenticationGraph::Condition_CrossPlatformAccountIsValid(const FAuthenticationGraphState &State)
{
    return State.AuthenticatedCrossPlatformAccountId.IsValid();
}

bool FAuthenticationGraph::Condition_CrossPlatformAccountIsValidWithPUID(const FAuthenticationGraphState &State)
{
    if (!State.AuthenticatedCrossPlatformAccountId.IsValid())
    {
        return false;
    }

    for (const auto &Candidate : State.EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform &&
            !EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
        {
            // The cross-platform candidate doesn't have an EOS account (no PUID).
            return false;
        }
    }

    return true;
}

bool FAuthenticationGraph::Condition_OneSuccessfulCandidate(const FAuthenticationGraphState &State)
{
    int SuccessCount = 0;
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
        {
            SuccessCount++;
        }
    }

    // Exactly one success.
    return SuccessCount == 1;
}

bool FAuthenticationGraph::Condition_MoreThanOneSuccessfulCandidate(const FAuthenticationGraphState &State)
{
    int SuccessCount = 0;
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
        {
            SuccessCount++;
        }
    }

    // Multiple success.
    return SuccessCount > 1;
}

bool FAuthenticationGraph::Condition_NoSuccessfulCandidate_AtLeastOneContinuanceToken(
    const FAuthenticationGraphState &State)
{
    int SuccessCount = 0;
    int ContinuanceCount = 0;
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
        {
            SuccessCount++;
        }
        if (!EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
        {
            ContinuanceCount++;
        }
    }

    // No success, at least one continuation token.
    return SuccessCount == 0 && ContinuanceCount > 0;
}

bool FAuthenticationGraph::Condition_NoSuccessfulCandidate_NoContinuanceToken(const FAuthenticationGraphState &State)
{
    int SuccessCount = 0;
    int ContinuanceCount = 0;
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (EOSString_ProductUserId::IsValid(Candidate.ProductUserId))
        {
            SuccessCount++;
        }
        if (!EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
        {
            ContinuanceCount++;
        }
    }

    // No success, no continuation token, EAS enabled/available.
    return SuccessCount == 0 && ContinuanceCount == 0;
}

bool FAuthenticationGraph::Condition_CrossPlatformProvidedContinuanceToken(const FAuthenticationGraphState &State)
{
    for (const auto &Candidate : State.EOSCandidates)
    {
        if (Candidate.Type == EAuthenticationGraphEOSCandidateType::CrossPlatform)
        {
            if (!EOSString_ProductUserId::IsValid(Candidate.ProductUserId) &&
                !EOSString_ContinuanceToken::IsNone(Candidate.ContinuanceToken))
            {
                return true;
            }
        }
    }

    return false;
}

void FAuthenticationGraph::Execute(
    const TSharedRef<FAuthenticationGraphState> &InitialState,
    const FAuthenticationGraphOnDone &OnDone) const
{
    TSharedRef<FAuthenticationGraphNode> StartingNode = this->CreateGraph(InitialState);

    // Track references to all the nodes in the graph, and reference the collector until
    // we get a result. This ensures nodes don't get released until the authentication graph
    // is done.
    TSharedRef<FAuthenticationGraphNodeCollector> Collector = MakeShared<FAuthenticationGraphNodeCollector>();
    Collector->AddNode(StartingNode);

    UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("%p: Authentication graph is starting"), &InitialState.Get());
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph entering node: %s"),
        &InitialState.Get(),
        *StartingNode->GetDebugName());
    StartingNode->Execute(
        InitialState,
        FAuthenticationGraphNodeOnDone::CreateSP(
            this,
            &FAuthenticationGraph::OnExecuteDone,
            InitialState,
            Collector,
            StartingNode,
            OnDone));
}

TSharedRef<FAsyncTaskGraph> FAuthenticationGraph::ConstructExecutor(
    TStatId StatId,
    const TSharedRef<FAuthenticationGraphState> &ResultingState,
    bool bAddLogoutTasks) const
{
    // Create the executor.
    auto Executor = MakeShared<FAsyncTaskGraph>(StatId);

    // If desired, add the logout tasks.
    if (bAddLogoutTasks)
    {
        FAsyncTaskPtr PreviousTask;
        for (const auto &LogoutNode : ResultingState->LogoutNodes)
        {
            PreviousTask = Executor->AddDelegate(
                FAsyncTaskGraph::FOnExecuteTask::CreateSPLambda(
                    this,
                    [this, ResultingState, LogoutNode](const FAsyncTaskGraph::FOnComplete &OnTaskComplete) {
                        UE_LOG(
                            LogRedpointEOSAuth,
                            Verbose,
                            TEXT("%p: Authentication graph is running logout node %s first"),
                            &ResultingState.Get(),
                            *LogoutNode->GetDebugName());
                        UE_LOG(
                            LogRedpointEOSAuth,
                            Verbose,
                            TEXT("%p: Authentication graph entering node: %s"),
                            &ResultingState.Get(),
                            *LogoutNode->GetDebugName());
                        LogoutNode->Execute(
                            ResultingState,
                            FAuthenticationGraphNodeOnDone::CreateSPLambda(
                                this,
                                [ResultingState, LogoutNode, OnTaskComplete](
                                    EAuthenticationGraphNodeResult LogoutNodeResult) {
                                    UE_LOG(
                                        LogRedpointEOSAuth,
                                        Verbose,
                                        TEXT("%p: Authentication graph exiting node: %s"),
                                        &ResultingState.Get(),
                                        *LogoutNode->GetDebugName());

                                    if (LogoutNodeResult == EAuthenticationGraphNodeResult::Error)
                                    {
                                        UE_LOG(
                                            LogRedpointEOSAuth,
                                            Warning,
                                            TEXT("%p: Authentication graph cleanup encountered an error, continuing "
                                                 "cleanup anyway"),
                                            &ResultingState.Get());
                                    }

                                    OnTaskComplete.ExecuteIfBound(Errors::Success());
                                }));
                    }),
                PreviousTask);
        }
    }

    // Return the executor.
    return Executor;
}

void FAuthenticationGraph::OnExecuteDone(
    EAuthenticationGraphNodeResult Result,
    TSharedRef<FAuthenticationGraphState> ResultingState,
    TSharedRef<FAuthenticationGraphNodeCollector> Collector,
    TSharedRef<FAuthenticationGraphNode> StartingNode,
    FAuthenticationGraphOnDone ExecuteOnDone) const
{
    UE_LOG(
        LogRedpointEOSAuth,
        Verbose,
        TEXT("%p: Authentication graph exiting node: %s"),
        &ResultingState.Get(),
        *StartingNode->GetDebugName());

    // We need to unwind state if we errored.
    bool bImmediateLogoutDueToError = Result == EAuthenticationGraphNodeResult::Error;

    // Construct the done executor, which will include logout tasks if errored.
    auto DoneExecutor =
        this->ConstructExecutor(GET_STATID(STAT_RedpointEOS_Auth_Done), ResultingState, bImmediateLogoutDueToError);

    // Finally, we're done.
    DoneExecutor->Finally(
        [ResultingState, ExecuteOnDone, Result, bImmediateLogoutDueToError, This = this->AsShared()](const FError &) {
            UE_LOG(LogRedpointEOSAuth, Verbose, TEXT("%p: Authentication graph is complete."), &ResultingState.Get());
            for (const auto &ErrorMessage : ResultingState->ErrorMessages)
            {
                UE_LOG(
                    LogRedpointEOSAuth,
                    Error,
                    TEXT("%p: Authentication graph has error message: %s"),
                    &ResultingState.Get(),
                    *ErrorMessage);
            }
            ExecuteOnDone.ExecuteIfBound(
                Result,
                FAuthenticationGraphOnLogout::CreateLambda([ResultingState, bImmediateLogoutDueToError, This](
                                                               const FAuthenticationGraphOnLogoutComplete &OnComplete) {
                    UE_LOG(
                        LogRedpointEOSAuth,
                        Verbose,
                        TEXT("%p: Authentication logout graph is starting"),
                        &ResultingState.Get());
                    auto LogoutExecutor = This->ConstructExecutor(
                        GET_STATID(STAT_RedpointEOS_Auth_Logout),
                        ResultingState,
                        !bImmediateLogoutDueToError);
                    ResultingState->LogoutNodes.Empty();
                    LogoutExecutor->Finally([OnComplete, ResultingState, This](const FError &) {
                        // 'This' holds the authentication graph around until logout nodes all finish.
                        (void)This;
                        UE_LOG(
                            LogRedpointEOSAuth,
                            Verbose,
                            TEXT("%p: Authentication logout graph is complete"),
                            &ResultingState.Get());
                        OnComplete.ExecuteIfBound();
                    });
                    LogoutExecutor->Start();
                }));
        });

    // Execute the graph.
    DoneExecutor->Start();
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()