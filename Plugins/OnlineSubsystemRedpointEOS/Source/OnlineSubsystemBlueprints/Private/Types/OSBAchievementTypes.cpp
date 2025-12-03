// Copyright June Rhodes. All Rights Reserved.

#include "Types/OSBAchievementTypes.h"

FOnlineAchievementBP FOnlineAchievementBP::FromNative(const FOnlineAchievement &Achievement)
{
    FOnlineAchievementBP Result;
    Result.Id = Achievement.Id;
    Result.Progress = Achievement.Progress;
    return Result;
}

FOnlineAchievementDescBP FOnlineAchievementDescBP::FromNative(const FOnlineAchievementDesc &AchievementDescription)
{
    FOnlineAchievementDescBP Result;
    Result.Title = AchievementDescription.Title;
    Result.LockedDesc = AchievementDescription.LockedDesc;
    Result.UnlockedDesc = AchievementDescription.UnlockedDesc;
    Result.bIsHidden = AchievementDescription.bIsHidden;
    Result.UnlockTime = AchievementDescription.UnlockTime;
    return Result;
}

void UOnlineAchievementsWrite::SetAchievementProgress(FName AchievementId, float AchievementProgress)
{
#if REDPOINT_EOS_UE_5_7_OR_LATER
#error The AchievementId parameter type must be changed to FString as of Unreal Engine 5.7.
#elif REDPOINT_EOS_UE_5_5_OR_LATER
    this->AchievementsWrite->SetFloatStat(AchievementId.ToString(), AchievementProgress);
#else
    this->AchievementsWrite->SetFloatStat(AchievementId, AchievementProgress);
#endif
}

FOnlineAchievementsWriteRef &UOnlineAchievementsWrite::GetAchievementsWrite()
{
    return this->AchievementsWrite;
}