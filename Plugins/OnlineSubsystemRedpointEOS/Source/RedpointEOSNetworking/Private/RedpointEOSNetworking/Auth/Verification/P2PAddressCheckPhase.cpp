// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Verification/P2PAddressCheckPhase.h"

#if !REDPOINT_EOS_IS_DEDICATED_SERVER
#include "RedpointEOSCore/Id/Id.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseFailureCode.h"
#include "RedpointEOSNetworking/Socket/InternetAddrRedpointEOS.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_DEDICATED_SERVER

namespace REDPOINT_EOS_FILE_NS_ID(4036443453, Redpoint::EOS::Networking::Auth::Verification)
{
using namespace ::Redpoint::EOS::Core::Id;
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Socket;

const FName AuthPhaseP2PAddressCheck = FName(TEXT("P2PAddressCheck"));

FName FP2PAddressCheckPhase::GetName() const
{
    return AuthPhaseP2PAddressCheck;
}

void FP2PAddressCheckPhase::RegisterRoutes(URedpointEOSControlChannel *ControlChannel)
{
}

void FP2PAddressCheckPhase::Start(const TSharedRef<FAuthVerificationPhaseContext> &Context)
{
    UNetConnection *Connection;
    if (!Context->GetConnection(Connection))
    {
        Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
        return;
    }

    auto RemoteAddress = Connection->LowLevelGetRemoteAddress();

    if (RemoteAddress.Contains(TEXT(".eosp2p")))
    {
        ISocketSubsystem *SocketSubsystem;
        if (!Context->GetSocketSubsystem(SocketSubsystem))
        {
            Context->Finish(EAuthPhaseFailureCode::All_ConnectionHasGoneAway);
            return;
        }
        if (SocketSubsystem == nullptr)
        {
            Context->Finish(EAuthPhaseFailureCode::All_CanNotAccessSocketSubsystem);
            return;
        }

        auto InternetAddr =
            StaticCastSharedPtr<FInternetAddrRedpointEOS>(SocketSubsystem->GetAddressFromString(RemoteAddress));
        if (!InternetAddr.IsValid() || !IsProductUserId(InternetAddr->GetUserId()))
        {
            Context->Finish(EAuthPhaseFailureCode::Phase_P2PAddressCheck_InvalidSourceAddress);
            return;
        }

        if (InternetAddr->GetUserId() != Context->GetUserId())
        {
            Context->Finish(EAuthPhaseFailureCode::Phase_P2PAddressCheck_UserIdDoesNotMatchSource);
            return;
        }
    }

    // Either the P2P address matches, or we're not hosting over P2P (in which case we
    // just permit clients to connect to IP-based listen servers with no authentication).
    Connection->PlayerId = GetUniqueNetIdRepl(Context->GetUserId());
    Context->Finish(EAuthPhaseFailureCode::Success);
}

}

#endif

REDPOINT_EOS_CODE_GUARD_END()