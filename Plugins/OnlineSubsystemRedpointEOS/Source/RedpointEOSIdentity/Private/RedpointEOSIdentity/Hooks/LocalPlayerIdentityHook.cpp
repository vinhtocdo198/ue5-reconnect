// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSIdentity/Hooks/LocalPlayerIdentityHook.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(4076165956, Redpoint::EOS::Identity::Hooks)
{
using namespace ::Redpoint::EOS::Identity::Hooks;
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::Core::Id;

void FLocalPlayerIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    if (GEngine == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("GEngine is not available; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    FName WorldContextHandle = (InContext.Environment->InstanceName != NAME_None &&
                                InContext.Environment->InstanceName != FName(TEXT("DefaultInstance")))
                                   ? InContext.Environment->InstanceName
                                   : FName("Context_0");

    auto WorldContext = GEngine->GetWorldContextFromHandle(WorldContextHandle);
    if (WorldContext == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Unable to obtain world context; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    auto World = WorldContext->World();
    if (World == nullptr)
    {
        // This is probably a valid scenario, where the world hasn't been set up yet. Since the world hasn't been set
        // up, we don't need to worry about the ULocalPlayer or APlayerState not being in sync.
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    ULocalPlayer *LocalPlayer = GEngine->GetLocalPlayerFromControllerId(World, InUser->GetUserSlot());
    if (LocalPlayer == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Unable to get local player %d; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."),
            InUser->GetUserSlot());
#endif
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    FUniqueNetIdRepl UniqueNetId = GetUniqueNetIdRepl(InUser->GetUserId());
    LocalPlayer->SetCachedUniqueNetId(UniqueNetId);
    UE_LOG(
        LogRedpointEOSIdentity,
        Verbose,
        TEXT("Updated unique net ID of ULocalPlayer for local player num %d to '%s'."),
        InUser->GetUserSlot(),
        *UniqueNetId.ToString());

    auto PlayerController = LocalPlayer->GetPlayerController(World);
    if (PlayerController != nullptr && PlayerController->PlayerState != nullptr)
    {
        PlayerController->PlayerState->SetUniqueId(UniqueNetId);
        UE_LOG(
            LogRedpointEOSIdentity,
            Verbose,
            TEXT("Updated unique net ID of APlayerState for local player num %d to '%s'."),
            InUser->GetUserSlot(),
            *UniqueNetId.ToString());
    }

    InOnComplete.ExecuteIfBound(Errors::Success());
}

void FLocalPlayerIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    if (GEngine == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("GEngine is not available; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    FName WorldContextHandle = (InContext.Environment->InstanceName != NAME_None &&
                                InContext.Environment->InstanceName != FName(TEXT("DefaultInstance")))
                                   ? InContext.Environment->InstanceName
                                   : FName("Context_0");

    auto WorldContext = GEngine->GetWorldContextFromHandle(WorldContextHandle);
    if (WorldContext == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Unable to obtain world context; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."));
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    auto World = WorldContext->World();
    if (World == nullptr)
    {
        // This is probably a valid scenario, where the world hasn't been set up yet. Since the world hasn't been set
        // up, we don't need to worry about the ULocalPlayer or APlayerState not being in sync.
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    ULocalPlayer *LocalPlayer = GEngine->GetLocalPlayerFromControllerId(World, InUser->GetUserSlot());
    if (LocalPlayer == nullptr)
    {
        if (GIsAutomationTesting)
        {
            InOnComplete.ExecuteIfBound(Errors::Success());
            return;
        }

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
        UE_LOG(
            LogRedpointEOSIdentity,
            Error,
            TEXT("Unable to get local player %d; the unique net ID stored in ULocalPlayer and APlayerState will "
                 "not be correct until the next map load."),
            InUser->GetUserSlot());
#endif
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    LocalPlayer->SetCachedUniqueNetId(FUniqueNetIdRepl());
    UE_LOG(
        LogRedpointEOSIdentity,
        Verbose,
        TEXT("Updated unique net ID of ULocalPlayer for local player num %d for logout."),
        InUser->GetUserSlot());

    auto PlayerController = LocalPlayer->GetPlayerController(World);
    if (PlayerController != nullptr && PlayerController->PlayerState != nullptr)
    {
        PlayerController->PlayerState->SetUniqueId(FUniqueNetIdRepl());
        UE_LOG(
            LogRedpointEOSIdentity,
            Verbose,
            TEXT("Updated unique net ID of APlayerState for local player num %d for logout."),
            InUser->GetUserSlot());
    }

    InOnComplete.ExecuteIfBound(Errors::Success());
}

}

REDPOINT_EOS_CODE_GUARD_END()