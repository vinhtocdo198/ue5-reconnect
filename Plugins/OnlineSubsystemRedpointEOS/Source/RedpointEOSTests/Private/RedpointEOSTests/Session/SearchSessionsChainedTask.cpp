// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/SearchSessionsChainedTask.h"

#include "Containers/Ticker.h"
#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1721909571, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests::Session;

FSearchSessionsChainedTask::FSearchSessionsChainedTask(
    int32 InSearchingInstanceId,
    const FConfigureSessionSearch &InConfigureSessionSearch,
    const FEvaluateSessionSearch &InEvaluateSessionSearch)
    : SearchingInstanceId(InSearchingInstanceId)
    , ConfigureSessionSearch(InConfigureSessionSearch)
    , EvaluateSessionSearch(InEvaluateSessionSearch)
    , OnFindSessionsCompleteHandle()
    , bSearchedOnce(false)
{
}

void FSearchSessionsChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->SearchingInstanceId).GetSessionInterface();

    // Create the search object.
    auto Search = MakeShared<FOnlineSessionSearch>();

    // Clear default search parameters.
    Search->QuerySettings.SearchParams.Empty();

    // Allow us to find non-listening sessions, since we aren't listening for our test sessions.
    Search->QuerySettings.SearchParams.Add(
        FName(TEXT("__EOS_bListening")),
        FOnlineSessionSearchParam(false, EOnlineComparisonOp::Equals));

    // Configure the search.
    this->ConfigureSessionSearch.ExecuteIfBound(*Search);

    // Register callback we're interested in.
    this->OnFindSessionsCompleteHandle =
        SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateSP(
            this,
            &FSearchSessionsChainedTask::OnFindSessionsComplete,
            Search,
            Context,
            OnDone));

    // Start the FindSessions operation.
    if (!Context->Assert().TestTrue(
            TEXT("FindSessions operation started successfully."),
            SessionInterface->FindSessions(Context->User(this->SearchingInstanceId), Search)))
    {
        // Failed to start.
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(this->OnFindSessionsCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FSearchSessionsChainedTask::OnFindSessionsComplete(
    bool bCallbackWasSuccessful,
    TSharedRef<FOnlineSessionSearch> Search,
    TestUtilities::FChainedTaskContextRef Context,
    FOnComplete OnDone)
{

    if (Search->SearchState != EOnlineAsyncTaskState::Done && Search->SearchState != EOnlineAsyncTaskState::Failed)
    {
        // Not our event.
        return;
    }

    // Get the session interface and unregister our event handler.
    auto SessionInterface = Context->Instance(this->SearchingInstanceId).GetSessionInterface();
    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(this->OnFindSessionsCompleteHandle);

    // If searching failed, then don't attempt a second search regardless of the chained task state.
    if (!Context->Assert().TestEqual(
            TEXT("Expected search to succeed (even if it is empty)."),
            Search->SearchState,
            EOnlineAsyncTaskState::Done))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // If we're searching for the first time, delay and then search again. This helps mitigate eventual consistency
    // issues.
    if (!this->bSearchedOnce)
    {
        // Schedule second search.
        FTSTicker::GetCoreTicker().AddTicker(
            FTickerDelegate::CreateSPLambda(
                this->AsShared(),
                [this, Context, OnDone](float) -> bool {
                    this->bSearchedOnce = true;
                    this->ExecuteAsync(Context, OnDone);
                    return false;
                }),
            5.0f);
    }
    else
    {
        // Evaluate whether we got the search results we expected.
        OnDone.ExecuteIfBound(
            this->EvaluateSessionSearch.IsBound() ? this->EvaluateSessionSearch.Execute(Context, *Search) : false);
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()