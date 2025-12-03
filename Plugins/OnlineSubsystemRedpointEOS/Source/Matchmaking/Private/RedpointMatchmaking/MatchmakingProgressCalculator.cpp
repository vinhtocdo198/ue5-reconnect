// Copyright June Rhodes. All Rights Reserved.

#include "MatchmakingProgressCalculator.h"

#if !REDPOINT_EOS_IS_FREE_EDITION
#include "RedpointMatchmaking/MatchmakingConstants.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

float FMatchmakingProgressCalculator::ComputeProgress(const FMatchmakingEngineTask &InTask)
{
    if (InTask.TeamManager.IsValid())
    {
        return (float)(InTask.TeamManager->GetTotalCapacity() - InTask.TeamManager->GetUnassignedSlotCount()) /
               (float)InTask.TeamManager->GetTotalCapacity();
    }
    else if (InTask.MatchmakingLobbyId.IsValid())
    {
        if (auto Pins = InTask.PinAndContinue())
        {
            int32 MemberCount;
            if (Pins->OnlineLobby->GetMemberCount(*InTask.Request.UserId, *InTask.MatchmakingLobbyId, MemberCount))
            {
                FVariantData TargetCapacity;
                if (Pins->OnlineLobby->GetLobbyMetadataValue(
                        *InTask.Request.UserId,
                        *InTask.MatchmakingLobbyId,
                        MatchmakingMetadataTargetCapacity,
                        TargetCapacity))
                {
                    if (TargetCapacity.GetType() == EOnlineKeyValuePairDataType::Int64)
                    {
                        int64 TargetCapacityValue;
                        TargetCapacity.GetValue(TargetCapacityValue);

                        return (float)MemberCount / (float)TargetCapacityValue;
                    }
                    else
                    {
                        UE_LOG(
                            LogMatchmaking,
                            Warning,
                            TEXT("%sUnable to compute progress value (target capacity attribute is wrong type)"),
                            *FString::Printf(TEXT("Task %s: "), *InTask.TaskId, *InTask.CurrentStep.ToString()));
                    }
                }
                else
                {
                    UE_LOG(
                        LogMatchmaking,
                        Warning,
                        TEXT("%sUnable to compute progress value (target capacity attribute does not exist)"),
                        *FString::Printf(TEXT("Task %s: "), *InTask.TaskId, *InTask.CurrentStep.ToString()));
                }
            }
            else
            {
                UE_LOG(
                    LogMatchmaking,
                    Warning,
                    TEXT("%sUnable to compute progress value (unable to get member count)"),
                    *FString::Printf(TEXT("Task %s: "), *InTask.TaskId, *InTask.CurrentStep.ToString()));
            }
        }
        else
        {
            UE_LOG(
                LogMatchmaking,
                Warning,
                TEXT("%sUnable to compute progress value (unable to pin OSS APIs)"),
                *FString::Printf(TEXT("Task %s: "), *InTask.TaskId, *InTask.CurrentStep.ToString()));
        }

        return 0.0f;
    }
    else
    {
        // Not in a matchmaking lobby yet, so we haven't made any progress.
        return 0.0f;
    }
}

#endif

REDPOINT_EOS_CODE_GUARD_END()