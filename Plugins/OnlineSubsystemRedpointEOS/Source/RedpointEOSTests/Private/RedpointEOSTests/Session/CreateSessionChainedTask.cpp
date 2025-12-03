// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/CreateSessionChainedTask.h"

#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(882746230, Redpoint::EOS::Tests::Session)
{

FCreateSessionChainedTask::FCreateSessionChainedTask(
    int32 InInstanceId,
    const FName &InSessionName,
    const FConfigureSessionSettings &InConfigureSessionSettings)
    : InstanceId(InInstanceId)
    , SessionName(InSessionName)
    , ConfigureSessionSettings(InConfigureSessionSettings)
    , OnCreateSessionCompleteHandle()
{
}

void FCreateSessionChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->InstanceId).GetSessionInterface();

    // Register callback we're interested in.
    this->OnCreateSessionCompleteHandle =
        SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateSP(
            this,
            &FCreateSessionChainedTask::OnCreateSessionComplete,
            Context,
            OnDone));

    // Set up the desired settings.
    FOnlineSessionSettings SessionSettings;
    this->ConfigureSessionSettings.ExecuteIfBound(SessionSettings);

    // Start the CreateSession operation.
    if (!Context->Assert().TestTrue(
            TEXT("CreateSession operation started successfully."),
            SessionInterface->CreateSession(0, this->SessionName, SessionSettings)))
    {
        // Failed to start.
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(this->OnCreateSessionCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FCreateSessionChainedTask::OnCreateSessionComplete(
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
    auto SessionInterface = Context->Instance(this->InstanceId).GetSessionInterface();
    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(this->OnCreateSessionCompleteHandle);

    // Test that we created the session successfully and return.
    Context->Assert().TestTrue(TEXT("Expected session to be created successfully."), bCallbackWasSuccessful);
    OnDone.ExecuteIfBound(bCallbackWasSuccessful);
}

}

REDPOINT_EOS_CODE_GUARD_END()