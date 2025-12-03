// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingFollowResponse.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

FVariantData FMatchmakingFollowResponse::SerializeFollowResponse(int32 InRequestId, bool InAccepted)
{
    check(InRequestId > 0);

    return FVariantData(FString::Printf(
        TEXT("%d|%s"),
        InRequestId,
        InAccepted ? MatchmakingFollowResponseAccepted : MatchmakingFollowResponseRejected));
}

bool FMatchmakingFollowResponse::TryDeserializeFollowResponse(
    const TSharedPtr<IOnlineLobby, ESPMode::ThreadSafe> &InOnlineLobby,
    const FVariantData &InFollowResponse,
    int32 &OutRequestId,
    bool &OutAccepted)
{
    if (InFollowResponse.GetType() != EOnlineKeyValuePairDataType::String)
    {
        return false;
    }

    FString FollowResponseStr;
    InFollowResponse.GetValue(FollowResponseStr);

    FString SerializedRequestId;
    FString ResponseValue;
    int32 SeperatorIndex = FollowResponseStr.Find(TEXT("|"));
    if (SeperatorIndex == -1)
    {
        return false;
    }

    SerializedRequestId = FollowResponseStr.Mid(0, SeperatorIndex);
    ResponseValue = FollowResponseStr.Mid(SeperatorIndex + 1);

    OutRequestId = FCString::Atoi(*SerializedRequestId);
    OutAccepted = ResponseValue == MatchmakingFollowResponseAccepted;

    return OutRequestId > 0 &&
           (ResponseValue == MatchmakingFollowResponseAccepted || ResponseValue == MatchmakingFollowResponseRejected);
}

#endif

REDPOINT_EOS_CODE_GUARD_END()