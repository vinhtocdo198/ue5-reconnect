// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSAPI/Templates/Decl/Event.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/P2P/SocketId.h"
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FNotifyPeerConnectionClosed
{
    REDPOINT_EOSSDK_API_CALL_EVENT_BEGIN(P2P, PeerConnectionClosed, EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ProductUserId> LocalUserId;
        ParamHelpers::TRequired<TOptional<FSocketId>> SocketId;
    };

    class Result
    {
    public:
        EOS_ProductUserId LocalUserId;
        EOS_ProductUserId RemoteUserId;
        FSocketId SocketId;
        EOS_EConnectionClosedReason Reason;
    };

    REDPOINT_EOSSDK_API_CALL_EVENT_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()