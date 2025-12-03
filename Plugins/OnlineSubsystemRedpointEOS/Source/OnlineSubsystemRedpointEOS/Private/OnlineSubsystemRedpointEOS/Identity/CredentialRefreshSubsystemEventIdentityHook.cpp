// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Identity/CredentialRefreshSubsystemEventIdentityHook.h"

#include "EOSSubsystem.h"
#include "RedpointEOSCore/Utils/WorldResolution.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(1533336711, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

void FCredentialRefreshSubsystemEventIdentityHook::OnPostCredentialRefresh(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    bool bWasSuccessful)
{
    TSoftObjectPtr<UWorld> World =
        Redpoint::EOS::Core::Utils::FWorldResolution::GetWorld(InContext.Environment->InstanceName, true);
    if (World.IsValid())
    {
        UEOSSubsystem *GlobalSubsystem = UEOSSubsystem::GetSubsystem(World.Get());
        if (IsValid(GlobalSubsystem))
        {
            GlobalSubsystem->OnCredentialRefreshComplete.Broadcast(bWasSuccessful);
        }
    }
}

}

#endif