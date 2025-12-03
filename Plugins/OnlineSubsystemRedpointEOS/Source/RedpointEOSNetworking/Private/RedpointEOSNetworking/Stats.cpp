// Copyright June Rhodes. All Rights Reserved.

#include "RedpointEOSNetworking/Stats.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Networking
{

REDPOINT_EOS_DEFINE_STAT(NetDriver, TickDispatch);
REDPOINT_EOS_DEFINE_STAT(NetDriver, BaseTickDispatch);
REDPOINT_EOS_DEFINE_STAT(NetDriver, OnIncomingConnection);
REDPOINT_EOS_DEFINE_STAT(NetDriver, OnConnectionAccepted);
REDPOINT_EOS_DEFINE_STAT(NetDriver, OnConnectionClosed);
REDPOINT_EOS_DEFINE_STAT(NetDriver, InitConnect);
REDPOINT_EOS_DEFINE_STAT(NetDriver, InitListen);

REDPOINT_EOS_DEFINE_STAT(Socket, RecvFrom);
REDPOINT_EOS_DEFINE_STAT(Socket, HasPendingData);
REDPOINT_EOS_DEFINE_STAT(Socket, SendTo);

REDPOINT_EOS_DEFINE_STAT(P2P, ReceivedLoopIters);
REDPOINT_EOS_DEFINE_STAT(P2P, ReceivedPackets);
REDPOINT_EOS_DEFINE_STAT(P2P, ReceivedBytes);
REDPOINT_EOS_DEFINE_STAT(P2P, SentPackets);
REDPOINT_EOS_DEFINE_STAT(P2P, SentBytes);

REDPOINT_EOS_DEFINE_INT_COUNTER(P2P, TEXT("ReceivedLoopIters"), ReceivedLoopIters);
REDPOINT_EOS_DEFINE_INT_COUNTER(P2P, TEXT("ReceivedPackets"), ReceivedPackets);
REDPOINT_EOS_DEFINE_INT_COUNTER(P2P, TEXT("ReceivedBytes"), ReceivedBytes);
REDPOINT_EOS_DEFINE_INT_COUNTER(P2P, TEXT("SentPackets"), SentPackets);
REDPOINT_EOS_DEFINE_INT_COUNTER(P2P, TEXT("SentBytes"), SentBytes);

}

REDPOINT_EOS_CODE_GUARD_END()