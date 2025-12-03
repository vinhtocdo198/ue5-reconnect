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

class FMatchmakingFollowRequest
{
public:
    /**
     * Serializes a follow request into an FVariantData suitable for a lobby attribute.
     */
    static FVariantData SerializeFollowRequest(
        const FOnlineLobbyId &InLobbyId,
        int32 InRequestId,
        int64 InLobbyCreatedUnixTimestamp,
        const FString &InStateHash);

    /**
     * Try to deserialize the follow request into it's lobby ID and request ID.
     */
    static bool TryDeserializeFollowRequest(
        const TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
        const FVariantData &InFollowRequest,
        TSharedPtr<const FOnlineLobbyId> &OutLobbyId,
        int32 &OutRequestId,
        int64 &OutLobbyCreatedUnixTimestamp,
        FString &OutStateHash);
};

#endif

REDPOINT_EOS_CODE_GUARD_END()