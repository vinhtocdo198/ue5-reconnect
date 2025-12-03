// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Auth/Connection/AuthConnectionPhaseContext.h"

#include "RedpointEOSControlChannel.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace REDPOINT_EOS_FILE_NS_ID(1581436555, Redpoint::EOS::Networking::Auth::Connection)
{

FAuthConnectionPhaseContext::FAuthConnectionPhaseContext(URedpointEOSControlChannel *InControlChannel)
    : FAuthPhaseContext(InControlChannel)
{
}

FString FAuthConnectionPhaseContext::GetIdentifier() const
{
    UNetConnection *NetConnection;
    if (this->GetConnection(NetConnection))
    {
        return NetConnection->LowLevelGetRemoteAddress();
    }
    return TEXT("(connection closed)");
}

FString FAuthConnectionPhaseContext::GetPhaseGroup() const
{
    return TEXT("connection");
}

void FAuthConnectionPhaseContext::MarkConnectionAsTrustedOnClient()
{
    URedpointEOSControlChannel *ControlChannelTemp;
    if (GetControlChannel(ControlChannelTemp))
    {
        ControlChannelTemp->bClientTrustsServer = true;
    }
}

}

REDPOINT_EOS_CODE_GUARD_END()