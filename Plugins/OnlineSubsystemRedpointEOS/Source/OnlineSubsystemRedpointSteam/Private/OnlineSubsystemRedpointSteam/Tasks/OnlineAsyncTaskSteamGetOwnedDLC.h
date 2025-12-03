// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1697498403, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

class FOnlineAsyncTaskSteamGetOwnedDLC : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
private:
    bool bInit;
    FSteamEntitlementsFetched Delegate;
    FString FailureContext;
    TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> Result;

public:
    FOnlineAsyncTaskSteamGetOwnedDLC(FSteamEntitlementsFetched InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamGetOwnedDLC);
    virtual ~FOnlineAsyncTaskSteamGetOwnedDLC() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamGetOwnedDLC");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1697498403,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamGetOwnedDLC)
}

#endif