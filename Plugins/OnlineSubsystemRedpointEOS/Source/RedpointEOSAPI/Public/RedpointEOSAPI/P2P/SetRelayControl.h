// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if WITH_CLIENT_CODE
#include "RedpointEOSAPI/ParamHelpers/Required.h"
#include "RedpointEOSAPI/Templates/Decl/Sync.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if WITH_CLIENT_CODE

namespace Redpoint::EOS::API::P2P
{

class REDPOINTEOSAPI_API FSetRelayControl
{
    REDPOINT_EOSSDK_API_CALL_SYNC_BEGIN(P2P, SetRelayControl, EOS_P2P_SETRELAYCONTROL_API_LATEST)

    class Options
    {
    public:
        ParamHelpers::TRequired<EOS_ERelayControl> RelayControl;
    };

    REDPOINT_EOSSDK_API_CALL_SYNC_END()
};

}

#endif

REDPOINT_EOS_CODE_GUARD_END()