// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Login/AntiCheatIntegrityPhase.h"

#include "RedpointEOSAntiCheat/AntiCheatActionReason.h"
#include "RedpointEOSControlChannel.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseFailureCode.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverRole.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1089039527, Redpoint::EOS::Networking::Auth::Login)
{
using namespace ::Redpoint::EOS::AntiCheat;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Auth;

const FName AuthPhaseAntiCheatIntegrity = FName(TEXT("AntiCheatIntegrity"));

FAntiCheatIntegrityPhase::FAntiCheatIntegrityPhase()
    : bIsStarted(false)
    , bIsVerified(false)
{
}

FName FAntiCheatIntegrityPhase::GetName() const
{
    return AuthPhaseAntiCheatIntegrity;
}

void FAntiCheatIntegrityPhase::RegisterRoutes(URedpointEOSControlChannel *ControlChannel)
{
}

void FAntiCheatIntegrityPhase::Start(const TSharedRef<FAuthLoginPhaseContext> &Context)
{
    UNetConnection *Connection;
    if (!Context->GetConnection(Connection))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    EUserVerificationStatus VerificationStatus = EUserVerificationStatus::NotStarted;
    if (Context->GetVerificationStatus(VerificationStatus) && VerificationStatus == EUserVerificationStatus::Verified)
    {
        // This client is a trusted client that is bypassing Anti-Cheat integrity. Skip
        // waiting for verification.
        Context->Finish(EAuthPhaseFailureCode::Success);
        return;
    }

    checkf(
        Connection->PlayerId.IsValid(),
        TEXT("Connection player ID must have been set by verification phase before Anti-Cheat phases can begin."));

    // NOTE: If this is a reconnection by the same player, where we skipped the registration
    // in AntiCheatProofPhase because of their existing registration, then we won't receive
    // the event and we need to proceed immediately.

    if (bIsVerified)
    {
        Context->Finish(EAuthPhaseFailureCode::Success);
    }
    else
    {
        bIsStarted = true;
    }
}

void FAntiCheatIntegrityPhase::OnAntiCheatPlayerAuthStatusChanged(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    EOS_EAntiCheatCommonClientAuthStatus NewAuthStatus)
{
    ENetDriverRole Role;
    if (!Context->GetRole(Role))
    {
        // If the role is not known, ignore this notification.
        return;
    }
    if (Role == ENetDriverRole::ClientConnectedToDedicatedServer ||
        Role == ENetDriverRole::ClientConnectedToListenServer)
    {
        // If we are a client, we handle our auth notifications in AntiCheatProofPhase instead.
        return;
    }

    FString StatusStr = TEXT("Unknown");
    if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_Invalid)
    {
        StatusStr = TEXT("Invalid");
    }
    else if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_LocalAuthComplete)
    {
        StatusStr = TEXT("LocalAuthComplete");
    }
    else if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete)
    {
        StatusStr = TEXT("RemoteAuthComplete");
    }

    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Anti-Cheat verification status is now '%s'."),
        *GetUserIdString(Context->GetUserId()),
        *StatusStr);

    if (NewAuthStatus == EOS_EAntiCheatCommonClientAuthStatus::EOS_ACCCAS_RemoteAuthComplete)
    {
        if (bIsStarted)
        {
            Context->Finish(EAuthPhaseFailureCode::Success);
        }
        else
        {
            bIsVerified = true;
        }
    }
}

void FAntiCheatIntegrityPhase::OnAntiCheatPlayerActionRequired(
    const TSharedRef<FAuthLoginPhaseContext> &Context,
    EOS_EAntiCheatCommonClientAction ClientAction,
    EOS_EAntiCheatCommonClientActionReason ActionReasonCode,
    const FString &ActionReasonDetailsString)
{
    FString DetailsString = FString::Printf(
        TEXT("Disconnected from server due to Anti-Cheat error. Reason: '%s'. Details: '%s'."),
        *EOS_EAntiCheatCommonClientActionReason_ToString(ActionReasonCode),
        *ActionReasonDetailsString);
    UE_LOG(
        LogRedpointEOSNetworking,
        Warning,
        TEXT("A network connection will close due to an Anti-Cheat error. Reason: '%s'. Details: '%s'."),
        *EOS_EAntiCheatCommonClientActionReason_ToString(ActionReasonCode),
        *ActionReasonDetailsString);
    Context->Finish(EAuthPhaseFailureCode::Phase_AntiCheatIntegrity_KickedDueToEACFailure);
}

}

REDPOINT_EOS_CODE_GUARD_END()