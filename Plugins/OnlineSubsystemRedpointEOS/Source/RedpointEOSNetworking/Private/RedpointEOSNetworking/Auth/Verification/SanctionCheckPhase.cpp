// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Verification/SanctionCheckPhase.h"

#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSAPI/Sanctions/QueryActivePlayerSanctions.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseFailureCode.h"
#include "RedpointEOSNetworking/Logging.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(29325491, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Sanctions;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Auth;

const FName AuthPhaseSanctionCheck = FName(TEXT("SanctionCheck"));

FName FSanctionCheckPhase::GetName() const
{
    return AuthPhaseSanctionCheck;
}

void FSanctionCheckPhase::RegisterRoutes(URedpointEOSControlChannel *ControlChannel)
{
}

void FSanctionCheckPhase::Start(const TSharedRef<FAuthVerificationPhaseContext> &Context)
{
    auto Info = Context->GetInfo();
    if (!Info.PlatformHandle.IsValid())
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    Context->SetVerificationStatus(EUserVerificationStatus::CheckingSanctions);

    UE_LOG(
        LogRedpointEOSNetworkAuth,
        Verbose,
        TEXT("Server authentication: %s: Checking to see if user has any BAN sanctions..."),
        *GetUserIdString(Context->GetUserId()));

    auto IdentitySystem = Info.PlatformHandle->GetSystem<IIdentitySystem>();
    TSharedPtr<FIdentityUser> IdentityUser = nullptr;
    for (const auto &User : IdentitySystem->GetUsers())
    {
        IdentityUser = User;
        break;
    }
    if (!IdentityUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Warning,
            TEXT("Server authentication: %s: The server is not signed into a local user, so sanctions can not be "
                 "checked. Sanction checks will be skipped."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Success);
        return;
    }

    FQueryActivePlayerSanctions::Execute(
        Info.PlatformHandle.ToSharedRef(),
        FQueryActivePlayerSanctions::Options{
            GetProductUserId(Context->GetUserId()),
            IsDedicatedServerId(IdentityUser->GetUserId()) ? nullptr : GetProductUserId(IdentityUser->GetUserId())},
        FQueryActivePlayerSanctions::CompletionDelegate::CreateSPLambda(
            this,
            [Context](EOS_EResult ResultCode, const TArray<FQueryActivePlayerSanctions::ResultEntry> &ResultEntries) {
                if (ResultCode == EOS_EResult::EOS_AccessDenied || ResultCode == EOS_EResult::EOS_NotConfigured ||
                    ResultCode == EOS_EResult::EOS_MissingPermissions)
                {
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Warning,
                        TEXT("Server authentication: %s: Server's client policy did not have permission to query "
                             "sanctions. Please update the client policy in the EOS Developer Portal. Bypassing "
                             "sanctions check due to lack of API permissions."),
                        *GetUserIdString(Context->GetUserId()));
                    Context->Finish(EAuthPhaseFailureCode::Success);
                }
                else if (ResultCode == EOS_EResult::EOS_NoConnection)
                {
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Warning,
                        TEXT("Server authentication: %s: Failed to connect to EOS sanctions service. Bypassing sanctions check due to connection issues."),
                        *GetUserIdString(Context->GetUserId()));
                    Context->Finish(EAuthPhaseFailureCode::Success);
                }
                else if (ResultCode == EOS_EResult::EOS_Success)
                {
                    bool bIsDenied = false;
                    for (const auto &Sanction : ResultEntries)
                    {
                        if (Sanction.Action == TEXT("BAN") || Sanction.Action == TEXT("RESTRICT_GAME_ACCESS"))
                        {
                            // User is banned.
                            UE_LOG(
                                LogRedpointEOSNetworkAuth,
                                Error,
                                TEXT("Server authentication: %s: User is banned because they have an active BAN or "
                                     "RESTRICT_GAME_ACCESS sanction."),
                                *GetUserIdString(Context->GetUserId()));
                            Context->Finish(EAuthPhaseFailureCode::Phase_SanctionCheck_AccountBanned);
                            bIsDenied = true;
                            break;
                        }

                        // Otherwise this is some other kind of sanction we don't handle yet.
                    }
                    if (!bIsDenied)
                    {
                        UE_LOG(
                            LogRedpointEOSNetworkAuth,
                            Verbose,
                            TEXT("Server authentication: %s: Did not find any BAN sanctions for user."),
                            *GetUserIdString(Context->GetUserId()));
                        Context->Finish(EAuthPhaseFailureCode::Success);
                    }
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Error,
                        TEXT("Server authentication: %s: Failed to retrieve sanctions for user, got result code %s on "
                             "server."),
                        *GetUserIdString(Context->GetUserId()),
                        *ConvertError(ResultCode).ToLogString());
                    Context->Finish(EAuthPhaseFailureCode::Phase_SanctionCheck_FailedToRetrieveSanctions);
                }
            }));
}

}

REDPOINT_EOS_CODE_GUARD_END()
