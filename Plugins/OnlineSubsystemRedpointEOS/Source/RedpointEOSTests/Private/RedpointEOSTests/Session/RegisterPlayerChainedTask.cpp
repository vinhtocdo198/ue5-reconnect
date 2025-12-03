// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSTests/Session/RegisterPlayerChainedTask.h"

#include "Interfaces/OnlineSessionInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2225559797, Redpoint::EOS::Tests::Session)
{

FRegisterPlayerChainedTask::FRegisterPlayerChainedTask(
    int32 InHostingInstanceId,
    int32 InTargetInstanceId,
    const FName &InSessionName)
    : HostingInstanceId(InHostingInstanceId)
    , TargetInstanceId(InTargetInstanceId)
    , SessionName(InSessionName)
    , OnRegisterPlayersCompleteHandle()
{
}

void FRegisterPlayerChainedTask::ExecuteAsync(
    const TestUtilities::FChainedTaskContextRef &Context,
    const FOnComplete &OnDone)
{
    auto SessionInterface = Context->Instance(this->HostingInstanceId).GetSessionInterface();

    Context->Assert().AddInfo(FString::Printf(
        TEXT("Registering player %d into session %s..."),
        this->TargetInstanceId,
        *this->SessionName.ToString()));

    // Register callback we're interested in.
    this->OnRegisterPlayersCompleteHandle =
        SessionInterface->AddOnRegisterPlayersCompleteDelegate_Handle(FOnRegisterPlayersCompleteDelegate::CreateSP(
            this,
            &FRegisterPlayerChainedTask::OnRegisterPlayersComplete,
            Context,
            OnDone));

    // Start the RegisterPlayers operation.
    if (!Context->Assert().TestTrue(
            TEXT("RegisterPlayers operation started successfully."),
            SessionInterface->RegisterPlayers(
                this->SessionName,
                TArray<FUniqueNetIdRef>{Context->User(this->TargetInstanceId).AsShared()},
                false)))
    {
        // Failed to start.
        SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(this->OnRegisterPlayersCompleteHandle);
        OnDone.ExecuteIfBound(false);
    }
}

void FRegisterPlayerChainedTask::OnRegisterPlayersComplete(
    FName CallbackSessionName,
    const TArray<FUniqueNetIdRef> &CallbackRegisteredPlayerIds,
    bool bCallbackWasSuccessful,
    TestUtilities::FChainedTaskContextRef Context,
    FOnComplete OnDone)
{
    if (!CallbackSessionName.IsEqual(this->SessionName) || CallbackRegisteredPlayerIds.Num() != 1 ||
        *CallbackRegisteredPlayerIds[0] != Context->User(this->TargetInstanceId))
    {
        // Not our event.
        return;
    }

    // Get the session interface and unregister our event handler.
    auto SessionInterface = Context->Instance(this->HostingInstanceId).GetSessionInterface();
    SessionInterface->ClearOnRegisterPlayersCompleteDelegate_Handle(this->OnRegisterPlayersCompleteHandle);

    // Test that we registered our players successfully and return.
    Context->Assert().TestTrue(TEXT("Expected player to be registered successfully."), bCallbackWasSuccessful);
    OnDone.ExecuteIfBound(bCallbackWasSuccessful);
}

}

REDPOINT_EOS_CODE_GUARD_END()