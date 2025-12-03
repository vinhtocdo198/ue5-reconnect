// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(743794569, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

class FOnlineAsyncTaskSteamGetAvailableDLC : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
private:
    bool bInit;
    FSteamOffersFetched Delegate;
    FString FailureContext;
    TMap<FUniqueOfferId, TSharedPtr<FOnlineStoreOffer>> Result;

public:
    FOnlineAsyncTaskSteamGetAvailableDLC(FSteamOffersFetched InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamGetAvailableDLC);
    virtual ~FOnlineAsyncTaskSteamGetAvailableDLC() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamGetAvailableDLC");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    743794569,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamGetAvailableDLC)
}

#endif