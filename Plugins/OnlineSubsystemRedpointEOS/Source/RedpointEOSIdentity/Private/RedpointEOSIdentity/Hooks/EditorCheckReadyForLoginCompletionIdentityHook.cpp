// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Hooks/EditorCheckReadyForLoginCompletionIdentityHook.h"

#include "Engine/Engine.h"
#include "RedpointEOSIdentity/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(89100057, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity::Hooks;
using namespace ::Redpoint::EOS::API;

void FEditorCheckReadyForLoginCompletionIdentityHook::OnPostLoginBeforeEvents(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FIdentityUserPtr &InUserIfLoginSuccessful,
    const FOnPostLoginBeforeEventsComplete &InOnComplete)
{
#if WITH_EDITOR
    // When we are automatically logging in in the editor, we need to ensure that client PIE instances always finish
    // their login process *after* the server PIE instance is listening. This is because UEOSNetDriver needs to fix up
    // the default 127.0.0.1 URL if the server is listening over EOS P2P.
    if (GEngine != nullptr)
    {
        FWorldContext *WorldContext = GEngine->GetWorldContextFromHandle(InContext.Environment->InstanceName);
        if (WorldContext != nullptr)
        {
            if (WorldContext->WorldType == EWorldType::PIE && WorldContext->PIEInstance != 0)
            {
                UE_LOG(
                    LogRedpointEOSIdentity,
                    Verbose,
                    TEXT("PIE client login is being delayed while waiting for the server "
                         "to be ready."));
                FTSTicker::GetCoreTicker().AddTicker(
                    FTickerDelegate::CreateThreadSafeSP(
                        this->AsShared(),
                        &FEditorCheckReadyForLoginCompletionIdentityHook::CheckIfReadyToFinishEditorLogin,
                        InOnComplete),
                    0);
                return;
            }
        }
    }
#endif

    InOnComplete.ExecuteIfBound(Errors::Success());
}

#if WITH_EDITOR

bool FEditorCheckReadyForLoginCompletionIdentityHook::CheckIfReadyToFinishEditorLogin(
    float DeltaSeconds,
    FOnPostLoginBeforeEventsComplete InOnComplete)
{
    if (GEngine == nullptr)
    {
        // Engine no longer valid, finish firing delegates for completeness.
        UE_LOG(
            LogRedpointEOSIdentity,
            Warning,
            TEXT("PIE client login completing early because engine is no longer available"));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return false;
    }

    for (const auto &WorldContext : GEngine->GetWorldContexts())
    {
        if (WorldContext.WorldType == EWorldType::PIE && WorldContext.PIEInstance == 0)
        {
            if (WorldContext.bWaitingOnOnlineSubsystem)
            {
                // We are still waiting for the host to be ready.
                return true;
            }

            // The host has finished being ready, complete our client login.
            UE_LOG(
                LogRedpointEOSIdentity,
                Verbose,
                TEXT("PIE client login completing because the PIE server is now ready"));
            InOnComplete.ExecuteIfBound(Errors::Success());
            return false;
        }
    }

    // The host context has disappeared, finish firing delegates for completeness.
    UE_LOG(
        LogRedpointEOSIdentity,
        Warning,
        TEXT("PIE client login completing early because host context is no longer available"));
    InOnComplete.ExecuteIfBound(Errors::Success());
    return false;
}

#endif

}

REDPOINT_EOS_CODE_GUARD_END()