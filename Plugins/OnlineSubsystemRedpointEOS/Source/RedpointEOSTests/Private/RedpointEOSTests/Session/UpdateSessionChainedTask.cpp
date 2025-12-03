// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/UpdateSessionChainedTask.h"

#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3329562986, Redpoint::EOS::Tests::Session)
{
using namespace ::Redpoint::EOS::Tests::Session;

FUpdateSessionChainedTask::FUpdateSessionChainedTask(
    int32 InInstanceId,
    const FName &InSessionName,
    const FConfigureSessionSettings &InConfigureSessionSettings)
    : InstanceId(InInstanceId)
    , SessionName(InSessionName)
    , ConfigureSessionSettings(InConfigureSessionSettings)
    , OnUpdateSessionCompleteHandle()
{
}

void FUpdateSessionChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->InstanceId).GetSessionInterface();

    // Register callback we're interested in.
    this->OnUpdateSessionCompleteHandle =
        SessionInterface->AddOnUpdateSessionCompleteDelegate_Handle(FOnUpdateSessionCompleteDelegate::CreateSP(
            this,
            &FUpdateSessionChainedTask::OnUpdateSessionComplete,
            Context,
            OnDone));

    // Get the existing session settings.
    auto *ExistingSession = SessionInterface->GetNamedSession(this->SessionName);
    if (!Context->Assert().TestNotNull(TEXT("Expected session to exist."), ExistingSession))
    {
        OnDone.ExecuteIfBound(false);
        return;
    }

    // Make a copy of the existing settings.
    FOnlineSessionSettings SessionSettings = ExistingSession->SessionSettings;

    // Configure on top of the copy.
    this->ConfigureSessionSettings.ExecuteIfBound(SessionSettings);

    // Start the UpdateSession operation.
    if (!Context->Assert().TestTrue(
            TEXT("UpdateSession operation started successfully."),
            SessionInterface->UpdateSession(this->SessionName, SessionSettings, true)))
    {
        // Failed to start.
        SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(this->OnUpdateSessionCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FUpdateSessionChainedTask::OnUpdateSessionComplete(
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
    SessionInterface->ClearOnUpdateSessionCompleteDelegate_Handle(this->OnUpdateSessionCompleteHandle);

    // Test that we updated the session successfully and return.
    Context->Assert().TestTrue(TEXT("Expected session to be updated successfully."), bCallbackWasSuccessful);
    OnDone.ExecuteIfBound(bCallbackWasSuccessful);
}

}

REDPOINT_EOS_CODE_GUARD_END()