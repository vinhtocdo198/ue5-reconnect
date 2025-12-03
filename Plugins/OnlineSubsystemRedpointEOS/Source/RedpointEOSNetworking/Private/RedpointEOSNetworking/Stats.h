// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Stats.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_CODE_GUARD_BEGIN()

namespace Redpoint::EOS::Networking
{

REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("TickDispatch"), TickDispatch);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("BaseTickDispatch"), BaseTickDispatch);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("OnIncomingConnection"), OnIncomingConnection);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("OnConnectionAccepted"), OnConnectionAccepted);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("OnConnectionClosed"), OnConnectionClosed);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("InitConnect"), InitConnect);
REDPOINT_EOS_DECLARE_CYCLE_STAT(NetDriver, TEXT("InitListen"), InitListen);

REDPOINT_EOS_DECLARE_CYCLE_STAT(Socket, TEXT("RecvFrom"), RecvFrom);
REDPOINT_EOS_DECLARE_CYCLE_STAT(Socket, TEXT("HasPendingData"), HasPendingData);
REDPOINT_EOS_DECLARE_CYCLE_STAT(Socket, TEXT("SendTo"), SendTo);

REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(P2P, TEXT("ReceivedLoopIters"), ReceivedLoopIters);
REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(P2P, TEXT("ReceivedPackets"), ReceivedPackets);
REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(P2P, TEXT("ReceivedBytes"), ReceivedBytes);
REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(P2P, TEXT("SentPackets"), SentPackets);
REDPOINT_EOS_DECLARE_DWORD_COUNTER_STAT(P2P, TEXT("SentBytes"), SentBytes);

REDPOINT_EOS_DECLARE_INT_COUNTER(P2P, ReceivedLoopIters);
REDPOINT_EOS_DECLARE_INT_COUNTER(P2P, ReceivedPackets);
REDPOINT_EOS_DECLARE_INT_COUNTER(P2P, ReceivedBytes);
REDPOINT_EOS_DECLARE_INT_COUNTER(P2P, SentPackets);
REDPOINT_EOS_DECLARE_INT_COUNTER(P2P, SentBytes);

}

REDPOINT_EOS_CODE_GUARD_END()
