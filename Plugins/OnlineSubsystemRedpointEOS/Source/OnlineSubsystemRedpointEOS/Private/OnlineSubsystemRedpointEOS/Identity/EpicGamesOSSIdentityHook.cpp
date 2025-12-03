// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointEOS/Identity/EpicGamesOSSIdentityHook.h"

#include "OnlineSubsystemRedpointEOS/Logging.h"
#include "OnlineSubsystemRedpointEOS/Shared/EOSCommon.h"
#include "OnlineSubsystemRedpointEOS/Shared/EpicGames/OnlineSubsystemRedpointEAS.h"
#include "OnlineSubsystemRedpointEOS/Shared/OnlineSubsystemRedpointEOS.h"
#include "OnlineSubsystemUtils.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAuth/CrossPlatform/CrossPlatformAccountId.h"
#include "RedpointEOSAuth/CrossPlatform/EpicGamesCrossPlatformAccountId.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4052375527, Redpoint::EOS::OnlineSubsystemRedpointEOS::Identity)
{
using namespace ::Redpoint::EOS::Identity;

void FEpicGamesOSSIdentityHook::OnStartSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStartSystemComplete &InOnComplete)
{
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::API;
    using namespace ::Redpoint::EOS::Auth::CrossPlatform;

    if (!InUser->GetCrossPlatformAccountId().IsValid() ||
        InUser->GetCrossPlatformAccountId()->GetType() != EPIC_GAMES_ACCOUNT_ID)
    {
        InOnComplete.ExecuteIfBound(Errors::Success());
        return;
    }

    auto OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"))
                   .GetExistingSubsystem(InContext.Environment->InstanceName);
    if (!OSS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EOS OSS to inform Epic Games account subsystem of signed in user.")));
        return;
    }

    auto OSSEAS = OSS->SubsystemEAS;
    if (!OSSEAS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EAS OSS to inform Epic Games account subsystem of signed in user.")));
        return;
    }

    auto IdentityEAS = OSSEAS->IdentityImpl;
    if (!IdentityEAS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EAS identity to inform Epic Games account subsystem of signed in user.")));
        return;
    }

    IdentityEAS->UserSignedInWithEpicId(
        InUser->GetUserSlot(),
        StaticCastSharedPtr<const FEpicGamesCrossPlatformAccountId>(InUser->GetCrossPlatformAccountId()));
    InOnComplete.ExecuteIfBound(Errors::Success());
}

void FEpicGamesOSSIdentityHook::OnStopSystem(
    const FIdentityHookContext &InContext,
    const FIdentityUserRef &InUser,
    const FOnStopSystemComplete &InOnComplete)
{
    using namespace ::Redpoint::EOS::Core::Utils;
    using namespace ::Redpoint::EOS::API;

    auto OSS = FModuleManager::GetModuleChecked<FOnlineSubsystemRedpointEOSModule>(TEXT("OnlineSubsystemRedpointEOS"))
                   .GetExistingSubsystem(InContext.Environment->InstanceName);
    if (!OSS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EOS OSS to inform Epic Games account subsystem of signed out user.")));
        return;
    }

    auto OSSEAS = OSS->SubsystemEAS;
    if (!OSSEAS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EAS OSS to inform Epic Games account subsystem of signed out user.")));
        return;
    }

    auto IdentityEAS = OSSEAS->IdentityImpl;
    if (!IdentityEAS.IsValid())
    {
        InOnComplete.ExecuteIfBound(Errors::UnexpectedError(
            ANSI_TO_TCHAR(__FUNCTION__),
            TEXT("Unable to locate EAS identity to inform Epic Games account subsystem of signed out user.")));
        return;
    }

    IdentityEAS->UserSignedOut(InUser->GetUserSlot());
    InOnComplete.ExecuteIfBound(Errors::Success());
}

}

#endif