// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Connection/AutomaticEncryptionCommon.h"

#include "RedpointEOSControlMessages.h"
#include "RedpointEOSNetworking/Auth/AuthPhaseRoute.h"
#include "RedpointEOSNetworking/Auth/Connection/AutomaticEncryptionPhase.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(2729212981, Redpoint::EOS::Networking::Auth::Connection)
{
using namespace ::Redpoint::EOS::Networking::Auth;
using namespace ::Redpoint::EOS::Networking::Auth::Connection;

void IAutomaticEncryptionCommon::RegisterRoutes(URedpointEOSControlChannel *CC)
{
    CC->AddRoute(
        NMT_EOS_RequestClientEphemeralKey,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            if (!IsValid(ControlChannel) || !IsValid(ControlChannel->Connection))
            {
                return false;
            }
            TSharedPtr<FAuthConnectionPhaseContext> Context = ControlChannel->GetAuthConnectionPhaseContext();
            if (Context)
            {
                Context->Log(TEXT("Received NMT_EOS_RequestClientEphemeralKey."));
                auto Phase = Context->GetPhase<IAutomaticEncryptionCommon>(AuthPhaseAutomationEncryption);
                if (Phase.IsValid())
                {
                    Phase->On_NMT_EOS_RequestClientEphemeralKey(Context.ToSharedRef(), Bunch);
                    return true;
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_DeliverClientEphemeralKey,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            if (!IsValid(ControlChannel) || !IsValid(ControlChannel->Connection))
            {
                return false;
            }
            TSharedPtr<FAuthConnectionPhaseContext> Context = ControlChannel->GetAuthConnectionPhaseContext();
            if (Context)
            {
                Context->Log(TEXT("Received NMT_EOS_DeliverClientEphemeralKey."));
                auto Phase = Context->GetPhase<IAutomaticEncryptionCommon>(AuthPhaseAutomationEncryption);
                if (Phase.IsValid())
                {
                    Phase->On_NMT_EOS_DeliverClientEphemeralKey(Context.ToSharedRef(), Bunch);
                    return true;
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_SymmetricKeyExchange,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            if (!IsValid(ControlChannel) || !IsValid(ControlChannel->Connection))
            {
                return false;
            }
            TSharedPtr<FAuthConnectionPhaseContext> Context = ControlChannel->GetAuthConnectionPhaseContext();
            if (Context)
            {
                Context->Log(TEXT("Received NMT_EOS_SymmetricKeyExchange."));
                auto Phase = Context->GetPhase<IAutomaticEncryptionCommon>(AuthPhaseAutomationEncryption);
                if (Phase.IsValid())
                {
                    Phase->On_NMT_EOS_SymmetricKeyExchange(Context.ToSharedRef(), Bunch);
                    return true;
                }
            }
            return false;
        }));
    CC->AddRoute(
        NMT_EOS_EnableEncryption,
        FAuthPhaseRoute::CreateLambda([](URedpointEOSControlChannel *ControlChannel, FInBunch &Bunch) {
            if (!IsValid(ControlChannel) || !IsValid(ControlChannel->Connection))
            {
                return false;
            }
            TSharedPtr<FAuthConnectionPhaseContext> Context = ControlChannel->GetAuthConnectionPhaseContext();
            if (Context)
            {
                Context->Log(TEXT("Received NMT_EOS_EnableEncryption."));
                auto Phase = Context->GetPhase<IAutomaticEncryptionCommon>(AuthPhaseAutomationEncryption);
                if (Phase.IsValid())
                {
                    Phase->On_NMT_EOS_EnableEncryption(Context.ToSharedRef(), Bunch);
                    return true;
                }
            }
            return false;
        }));
}

}

REDPOINT_EOS_CODE_GUARD_END()