// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1294003128, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;

class FOnlineAsyncTaskSteamGetAllItems : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
private:
    bool bInit;
    bool bInventoryServiceDisabled;
    SteamInventoryResult_t Result;
    FSteamGetAllItemsComplete Delegate;
    FString FailureContext;
    TArray<FSteamItemRaw> ResultItems;
    TArray<FString> Warnings;

    void AttemptFinalizeOnTickThread();

public:
    FOnlineAsyncTaskSteamGetAllItems(FSteamGetAllItemsComplete InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamGetAllItems);
    virtual ~FOnlineAsyncTaskSteamGetAllItems() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamGetAllItems");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    1294003128,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamGetAllItems)
}

#endif