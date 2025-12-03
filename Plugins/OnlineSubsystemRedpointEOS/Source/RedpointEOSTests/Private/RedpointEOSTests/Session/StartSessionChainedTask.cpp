// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/StartSessionChainedTask.h"

#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3102801617, Redpoint::EOS::Tests::Session)
{

FStartSessionChainedTask::FStartSessionChainedTask(int32 InHostingInstanceId, const FName &InSessionName)
    : HostingInstanceId(InHostingInstanceId)
    , SessionName(InSessionName)
    , OnStartSessionCompleteHandle()
{
}

void FStartSessionChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->HostingInstanceId).GetSessionInterface();

    // Register callback we're interested in.
    this->OnStartSessionCompleteHandle =
        SessionInterface->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateSP(
            this,
            &FStartSessionChainedTask::OnStartSessionComplete,
            Context,
            OnDone));

    // Start the StartSession operation.
    if (!Context->Assert().TestTrue(
            TEXT("StartSession operation started successfully."),
            SessionInterface->StartSession(this->SessionName)))
    {
        // Failed to start.
        SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(this->OnStartSessionCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FStartSessionChainedTask::OnStartSessionComplete(
    FName CallbackSessionName,
    bool bCallbackWasSuccessful,
    TestUtilities::FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!CallbackSessionName.IsEqual(this->SessionName))
    {
        // Not our event.
        return;
    }

    // Get the session interface and unregister our event handler.
    auto SessionInterface = Context->Instance(this->HostingInstanceId).GetSessionInterface();
    SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(this->OnStartSessionCompleteHandle);

    // Test that we started the session successfully and return.
    Context->Assert().TestTrue(TEXT("Expected session to be started successfully."), bCallbackWasSuccessful);
    OnDone.ExecuteIfBound(bCallbackWasSuccessful);
}

}

REDPOINT_EOS_CODE_GUARD_END()