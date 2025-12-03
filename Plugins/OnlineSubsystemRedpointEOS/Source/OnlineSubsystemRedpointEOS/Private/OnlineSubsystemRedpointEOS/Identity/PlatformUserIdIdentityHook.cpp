// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Identity/PlatformUserIdIdentityHook.h"

#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/PlatformUserIdManager.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "OnlineSubsystemRedpointEOS/VoiceChat/EOSVoiceManager.h"
#include "OnlineSubsystemRedpointEOSModule.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Id/Id.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1255016888, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem;

void FPlatformUserIdIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    auto OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"))
                   .GetExistingSubsystem(InContext.Environment->InstanceName);
    if (!OSS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate world to inform voice manager of signed in user.")));
        return;
    }

    FPlatformUserIdManager::AllocatedIds.Add(
        InUser->GetProcessGlobalUserHandle(),
        FPlatformUserIdManager::FValue(
            OSS,
            StaticCastSharedRef<const FUniqueNetIdEOS>(GetUniqueNetIdRef(InUser->GetUserId()))));
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void FPlatformUserIdIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::Core::Id;
    using namespace ::Redpoint::EOS::API;

    auto OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"))
                   .GetExistingSubsystem(InContext.Environment->InstanceName);
    if (!OSS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate world to inform voice manager of signed out user.")));
        return;
    }

    FPlatformUserIdManager::AllocatedIds.Remove(InUser->GetProcessGlobalUserHandle());
    InOnComplete.ExecuteIfBound(Errors::Success());
}

}

#endif