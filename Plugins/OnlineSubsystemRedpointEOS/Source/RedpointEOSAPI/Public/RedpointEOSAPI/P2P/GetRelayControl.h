// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/Templates/Decl/SyncGet.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FGetRelayControl
{
    REDPOINT_EOSSDK_API_CALL_SYNC_GET_BEGIN(P2P, GetRelayControl, EOS_P2P_GETRELAYCONTROL_API_LATEST)

    class Options
    {
    public:
    };

    using Result = EOS_ERelayControl;

    REDPOINT_EOSSDK_API_CALL_SYNC_GET_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()