// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Hooks/EditorDefaultInstanceCheckIdentityHook.h"

#include "Engine/Engine.h"
#include "RedpointEOSIdentity/Logging.h"
#if WITH_EDITOR
#include "RedpointEOSCore/Editor/EditorSignalling.h"
#endif // #if WITH_EDITOR

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(3723025591, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity::Hooks;
using namespace ::Redpoint::EOS::API;

void FEditorDefaultInstanceCheckIdentityHook::OnPreLogin(
    const FIdentityHookContext &InContext,
    const FIdentityUserSlot &InUserSlot,
    const FOnPreLoginComplete &InOnComplete)
{
#if WITH_EDITOR
    using namespace ::Redpoint::EOS::Core::Editor;

    bool bInEditor = false;
    if (GEngine != nullptr && InContext.Environment->InstanceName == TEXT("DefaultInstance"))
    {
        for (const auto &WorldContext : GEngine->GetWorldContexts())
        {
            if (WorldContext.WorldType == EWorldType::PIE || WorldContext.WorldType == EWorldType::Editor)
            {
                bInEditor = true;
                break;
            }
        }
    }
    if (bInEditor)
    {
        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Detected that AutoLogin was called on the DefaultInstance OSS instead of the play-in-editor OSS. "
                 "Something is calling IOnlineSubsystem::Get() instead of Online::GetSubsystem(this->GetWorld()). "
                 "Please fix your code to use the Online::GetSubsystem(this->GetWorld()) API, which is aware of "
                 "play-in-editor windows."))
        FEditorSignalling::OnEditorCustomSignal().Broadcast(
            TEXT("Authentication"),
            TEXT("WrongOnlineContextForPlayInEditor"));
        InOnComplete.ExecuteIfBound(Errors::InvalidParameters(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Something is calling IOnlineSubsystem::Get() in the editor to sign in instead of "
                 "Online::GetSubsystem(this->GetWorld()). Please change your code to use "
                 "Online::GetSubsystem(this->GetWorld()) which is aware of play-in-editor windows.")));
        return;
    }
#endif // #if WITH_EDITOR

    InOnComplete.ExecuteIfBound(Errors::Success());
}

}

REDPOINT_EOS_CODE_GUARD_END()