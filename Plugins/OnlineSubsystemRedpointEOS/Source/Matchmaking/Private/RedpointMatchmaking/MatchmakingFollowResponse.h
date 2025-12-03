// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "OnlineKeyValuePair.h"
#include "RedpointEOSInterfaces/Interfaces/OnlineLobbyInterface.h"
#include "Templates/SharedPointer.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

class FMatchmakingFollowResponse
{
public:
    /**
     * Serializes a follow response into an FVariantData suitable for a lobby attribute.
     */
    static FVariantData SerializeFollowResponse(int32 InRequestId, bool InAccepted);

    /**
     * Try to deserialize the follow response into it's lobby ID and request ID.
     */
    static bool TryDeserializeFollowResponse(
        const TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
        const FVariantData &InFollowResponse,
        int32 &OutRequestId,
        bool &OutAccepted);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()