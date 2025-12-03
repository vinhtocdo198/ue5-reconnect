// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/JoinSessionChainedTask.h"

#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(938180516, Redpoint::EOS::Tests::Session)
{

FJoinSessionChainedTask::FJoinSessionChainedTask(
    int32 InJoiningInstanceId,
    const FName &InSessionName,
    const TSharedRef<FOnlineSessionSearchResult> &InFoundSearchResult)
    : JoiningInstanceId(InJoiningInstanceId)
    , SessionName(InSessionName)
    , FoundSearchResult(InFoundSearchResult)
    , OnJoinSessionCompleteHandle()
{
}

void FJoinSessionChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->JoiningInstanceId).GetSessionInterface();

    // Register callback we're interested in.
    this->OnJoinSessionCompleteHandle =
        SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateSP(
            this,
            &FJoinSessionChainedTask::OnJoinSessionComplete,
            Context,
            OnDone));

    // Join the JoinSession operation.
    if (!Context->Assert().TestTrue(
            TEXT("JoinSession operation joined successfully."),
            SessionInterface
                ->JoinSession(Context->User(this->JoiningInstanceId), this->SessionName, *this->FoundSearchResult)))
    {
        // Failed to join.
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(this->OnJoinSessionCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FJoinSessionChainedTask::OnJoinSessionComplete(
    FName CallbackSessionName,
    EOnJoinSessionCompleteResult::Type CallbackJoinResult,
    TestUtilities::FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!CallbackSessionName.IsEqual(this->SessionName))
    {
        // Not our event.
        return;
    }

    // Get the session interface and unregister our event handler.
    auto SessionInterface = Context->Instance(this->JoiningInstanceId).GetSessionInterface();
    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(this->OnJoinSessionCompleteHandle);

    // Test that we joined the session successfully and return.
    Context->Assert().TestTrue(
        TEXT("Expected session to be joined successfully."),
        CallbackJoinResult == EOnJoinSessionCompleteResult::Success);
    OnDone.ExecuteIfBound(CallbackJoinResult == EOnJoinSessionCompleteResult::Success);
}

}

REDPOINT_EOS_CODE_GUARD_END()