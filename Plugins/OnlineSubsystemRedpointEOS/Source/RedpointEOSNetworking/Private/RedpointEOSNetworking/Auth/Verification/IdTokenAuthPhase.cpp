// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Verification/IdTokenAuthPhase.h"

#include "RedpointEOSAPI/Connect/CopyIdToken.h"
#include "RedpointEOSAPI/Connect/VerifyIdToken.h"
#include "RedpointEOSAPI/Error.h"
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSIdentity/IdentitySystem.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseRoute.h"
#include "RedpointEOSNetworking/Logging.h"
#include "RedpointEOSNetworking/NetDriverRole.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(366962924, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::API;
using namespace ::Redpoint::EOS::API::Connect;
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Identity;
using namespace ::Redpoint::EOS::Networking;
using namespace ::Redpoint::EOS::Networking::Auth;

const FName AuthPhaseIdTokenAuth = FName(TEXT("IdTokenAuth"));

FName FIdTokenAuthPhase::GetName() const
{
    return AuthPhaseIdTokenAuth;
}

void FIdTokenAuthPhase::RegisterRoutes(URedpointEOSControlChannel *CC)
{
    CC->AddRoute(
        NMT_EOS_RequestIdToken,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            FUniqueNetIdRepl TargetUserId;
            if (FNetControlMessage<NMT_EOS_RequestIdToken>::Receive(Bunch, TargetUserId))
            {
                TSharedPtr<FAuthVerificationPhaseContext> Context =
                    ControlChannel->GetAuthVerificationPhaseContext(TargetUserId);
                if (Context)
                {
                    TSharedPtr<FIdTokenAuthPhase> Phase = Context->GetPhase<FIdTokenAuthPhase>(AuthPhaseIdTokenAuth);
                    if (Phase)
                    {
                        Phase->On_NMT_EOS_RequestIdToken(Context.ToSharedRef());
                        return true;
                    }
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_DeliverIdToken,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            FUniqueNetIdRepl TargetUserId;
            FString ClientToken;
            if (FNetControlMessage<NMT_EOS_DeliverIdToken>::Receive(Bunch, TargetUserId, ClientToken))
            {
                TSharedPtr<FAuthVerificationPhaseContext> Context =
                    ControlChannel->GetAuthVerificationPhaseContext(TargetUserId);
                if (Context)
                {
                    TSharedPtr<FIdTokenAuthPhase> Phase = Context->GetPhase<FIdTokenAuthPhase>(AuthPhaseIdTokenAuth);
                    if (Phase)
                    {
                        Phase->On_NMT_EOS_DeliverIdToken(Context.ToSharedRef(), ClientToken);
                        return true;
                    }
                }
            }
            return false;
        }));
}

void FIdTokenAuthPhase::Start(const TSharedRef<FAuthVerificationPhaseContext> &Context)
{
    auto Info = Context->GetInfo();

    if (!IsValid(Info.Connection) || !Info.bIsTrustedOnClient.IsSet())
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }
    if (!*Info.bIsTrustedOnClient)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Warning,
            TEXT("Skipping verification of connecting user %s because this connection is not trusted. To verify "
                 "users, turn on trusted dedicated servers."),
            *GetUserIdString(Context->GetUserId()));
        Info.Connection->PlayerId = GetUniqueNetIdRepl(Context->GetUserId());
        Context->Finish(EAuthPhaseFailureCode::Success);
        return;
    }

    // Ask the client to provide us the ID token for the given user.
    FUniqueNetIdRepl UserIdRepl = GetUniqueNetIdRepl(Context->GetUserId());
    FNetControlMessage<NMT_EOS_RequestIdToken>::Send(Info.Connection, UserIdRepl);
}

void FIdTokenAuthPhase::On_NMT_EOS_RequestIdToken(const TSharedRef<FAuthVerificationPhaseContext> &Context)
{
#if !REDPOINT_EOS_IS_DEDICATED_SERVER
    auto Info = Context->GetInfo();

    if (!Info.Role.IsSet() || !IsValid(Info.Connection) || !Info.bIsTrustedOnClient.IsSet() ||
        !Info.PlatformHandle.IsValid())
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    if (*Info.Role != ENetDriverRole::ClientConnectedToDedicatedServer)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting from remote host because NMT_EOS_RequestIdToken was received when it was not "
                 "expected."));
        Context->Finish(EAuthPhaseFailureCode::Msg_RequestIdToken_UnexpectedIncorrectRole);
        return;
    }
    if (!Info.Connection->IsEncryptionEnabled())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting from remote host; can not send the ID token for %s because this connection is not "
                 "encrypted."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Msg_RequestIdToken_ConnectionNotEncrypted);
        return;
    }
    if (!*Info.bIsTrustedOnClient)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting from remote host; can not send the ID token for %s because this connection is not "
                 "trusted."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Msg_RequestIdToken_ConnectionNotTrusted);
        return;
    }

    auto IdentitySystem = Info.PlatformHandle->GetSystem<IIdentitySystem>();
    auto IdentityUser = IdentitySystem->GetUser(Context->GetUserId());

    if (!IdentityUser.IsValid())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting from remote host because we could not access the identity interface or the supplied "
                 "user ID was invalid."));
        Context->Finish(EAuthPhaseFailureCode::Msg_RequestIdToken_InvalidUserId);
        return;
    }

    EOS_EResult ResultCode;
    FCopyIdToken::Result Result;
    FCopyIdToken::Execute(
        Info.PlatformHandle.ToSharedRef(),
        FCopyIdToken::Options{GetProductUserId(Context->GetUserId())},
        ResultCode,
        Result);
    if (ResultCode != EOS_EResult::EOS_Success || Result.JsonWebToken.IsEmpty())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting from remote host because we could not obtain the ID token for %s, got error: %s"),
            *GetUserIdString(Context->GetUserId()),
            *ConvertError(ResultCode).ToLogString());
        Context->Finish(EAuthPhaseFailureCode::Msg_RequestIdToken_CanNotRetrieveIdToken);
        return;
    }

    FUniqueNetIdRepl TargetUserId = GetUniqueNetIdRepl(Context->GetUserId());
    FString Token = Result.JsonWebToken;
    FNetControlMessage<NMT_EOS_DeliverIdToken>::Send(Info.Connection, TargetUserId, Token);
#else
    Context->Finish(EAuthPhaseFailureCode::All_InvalidMessageType);
#endif
}

void FIdTokenAuthPhase::On_NMT_EOS_DeliverIdToken(
    const TSharedRef<FAuthVerificationPhaseContext> &Context,
    const FString &ClientToken)
{
    auto Info = Context->GetInfo();

    if (!IsValid(Info.Connection) || !Info.Role.IsSet() || !Info.bIsTrustedOnClient || !Info.PlatformHandle.IsValid())
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    if (*Info.Role != ENetDriverRole::DedicatedServer)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting remote host because NMT_EOS_DeliverIdToken was received when it was not "
                 "expected."));
        Context->Finish(EAuthPhaseFailureCode::Msg_DeliverIdToken_UnexpectedIncorrectRole);
        return;
    }
    if (!Info.Connection->IsEncryptionEnabled())
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting client; can not receive the ID token for %s because this connection is not "
                 "encrypted."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Msg_DeliverIdToken_ConnectionNotEncrypted);
        return;
    }
    if (!Info.bIsTrustedOnClient)
    {
        UE_LOG(
            LogRedpointEOSNetworkAuth,
            Error,
            TEXT("Disconnecting client; can not receive the ID token for %s because this connection is not "
                 "trusted."),
            *GetUserIdString(Context->GetUserId()));
        Context->Finish(EAuthPhaseFailureCode::Msg_DeliverIdToken_ConnectionNotTrusted);
        return;
    }

    FVerifyIdToken::Execute(
        Info.PlatformHandle.ToSharedRef(),
        FVerifyIdToken::Options{GetProductUserId(Context->GetUserId()), ClientToken},
        FVerifyIdToken::CompletionDelegate::CreateSPLambda(
            this,
            [Context](const FVerifyIdToken::Result &Data) {
                UNetConnection *Connection;
                if (!Context->GetConnection(Connection))
                {
                    Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
                    return;
                }

                if (Data.ResultCode == EOS_EResult::EOS_Success)
                {
                    // The token was valid and the user was successfully authenticated. Go to next step.
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Verbose,
                        TEXT("Server authentication: %s: Successfully authenticated user with their ID token."),
                        *GetUserIdString(Context->GetUserId()));

                    Connection->PlayerId = GetUniqueNetIdRepl(Context->GetUserId());
                    Context->Finish(EAuthPhaseFailureCode::Success);
                }
                else
                {
                    UE_LOG(
                        LogRedpointEOSNetworkAuth,
                        Error,
                        TEXT("Server authentication: %s: Failed to verify user, got result code %s on server"),
                        *GetUserIdString(Context->GetUserId()),
                        *ConvertError(Data.ResultCode).ToLogString());
                    Context->Finish(EAuthPhaseFailureCode::Msg_DeliverIdToken_AuthenticationFailed);
                    return;
                }
            }));
}

}

REDPOINT_EOS_CODE_GUARD_END()