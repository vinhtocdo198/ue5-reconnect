// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"
#if !REDPOINT_EOS_IS_FREE_EDITION
#include "Interfaces/OnlineStatsInterface.h"
#endif

REDPOINT_EOS_CODE_GUARD_BEGIN()

#if !REDPOINT_EOS_IS_FREE_EDITION

extern bool bAlternateStatsArrangement;

class FOnlineStatsInterfaceTest : public IOnlineStats,
                                  public TSharedFromThis<FOnlineStatsInterfaceTest, ESPMode::ThreadSafe>
{
public:
    virtual void QueryStats(
        const FUniqueNetIdRef LocalUserId,
        const FUniqueNetIdRef StatsUser,
        const FOnlineStatsQueryUserStatsComplete &Delegate) override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::QueryStats, return;);
    virtual void QueryStats(
        const FUniqueNetIdRef LocalUserId,
        const TArray<FUniqueNetIdRef> &StatUsers,
        const TArray<FString> &StatNames,
        const FOnlineStatsQueryUsersStatsComplete &Delegate) override;
    virtual TSharedPtr<const FOnlineStatsUserStats> GetStats(const FUniqueNetIdRef StatsUserId) const override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::GetStats, return nullptr;);
    virtual void UpdateStats(
        const FUniqueNetIdRef LocalUserId,
        const TArray<FOnlineStatsUserUpdatedStats> &UpdatedUserStats,
        const FOnlineStatsUpdateStatsComplete &Delegate) override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::UpdateStats, return;);
#if !UE_BUILD_SHIPPING
    virtual void ResetStats(const FUniqueNetIdRef StatsUserId) override
        PURE_VIRTUAL(FOnlineStatsInterfaceTest::ResetStats, return;);
#endif // !UE_BUILD_SHIPPING
};

#endif

REDPOINT_EOS_CODE_GUARD_END()