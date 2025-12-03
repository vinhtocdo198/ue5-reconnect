// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/FindSessionByIdChainedTask.h"

#include "Containers/Ticker.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(673342039, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests::Session;

FFindSessionByIdChainedTask::FFindSessionByIdChainedTask(
    int32 InSearchingInstanceId,
    int32 InOwningInstanceId,
    FName InOwningSessionName,
    const TSharedRef<FOnlineSessionSearchResult> &InFoundSearchResult)
    : SearchingInstanceId(InSearchingInstanceId)
    , OwningInstanceId(InOwningInstanceId)
    , OwningSessionName(InOwningSessionName)
    , FoundSearchResult(InFoundSearchResult)
    , bSearchedOnce(false)
{
}

void FFindSessionByIdChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SearchingSessionInterface = Context->Instance(this->SearchingInstanceId).GetSessionInterface();
    auto OwningSessionInterface = Context->Instance(this->OwningInstanceId).GetSessionInterface();

    // Start the search.
    if (!Context->Assert().TestTrue(
            TEXT("FindSessionById operation should start successfully."),
            SearchingSessionInterface->FindSessionById(
                Context->User(this->SearchingInstanceId),
                OwningSessionInterface->GetNamedSession(this->OwningSessionName)->SessionInfo->GetSessionId(),
                Context->User(this->SearchingInstanceId),
                FOnSingleSessionResultCompleteDelegate::CreateSP(
                    this,
                    &FFindSessionByIdChainedTask::OnFindSessionByIdComplete,
                    Context,
                    OnDone))))
    {
        // Failed to start.
        OnDone.ExecuteIfBound(false);
    }
}

void FFindSessionByIdChainedTask::OnFindSessionByIdComplete(
    int32 LocalUserId,
    bool bWasSuccessful,
    const FOnlineSessionSearchResult &SearchResult,
    TestUtilities::FChainedTaskContextRef Context,
    FOnComplete OnDone)
{

    // Get the session interface and unregister our event handler.
    auto SessionInterface = Context->Instance(this->SearchingInstanceId).GetSessionInterface();

    // If it wasn't successful and this is the first time searching, delay and then search again. This helps mitigate
    // eventual consistency issues.
    if (!bWasSuccessful && !this->bSearchedOnce)
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
        // Return the search result.
        if (Context->Assert().TestTrue(TEXT("Expected find by session ID to be successful."), bWasSuccessful))
        {
            if (Context->Assert().TestTrue(
                    TEXT("Expected search result to be valid if successful."),
                    SearchResult.IsValid() && SearchResult.IsSessionInfoValid()))
            {
                *this->FoundSearchResult = SearchResult;
            }
        }
        OnDone.ExecuteIfBound(bWasSuccessful && SearchResult.IsValid() && SearchResult.IsSessionInfoValid());
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()