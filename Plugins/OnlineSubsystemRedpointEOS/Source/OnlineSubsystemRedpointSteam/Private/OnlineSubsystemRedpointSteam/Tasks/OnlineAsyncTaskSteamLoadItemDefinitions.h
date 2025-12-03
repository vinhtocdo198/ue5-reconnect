// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3496575725, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;

class FOnlineAsyncTaskSteamLoadItemDefinitions;

class FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler
{
private:
    FOnlineAsyncTaskSteamLoadItemDefinitions *Owner;
    STEAM_CALLBACK(
        FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler,
        OnSteamInventoryDefinitionUpdate,
        SteamInventoryDefinitionUpdate_t);

public:
    FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler(class FOnlineAsyncTaskSteamLoadItemDefinitions *InOwner)
        : Owner(InOwner){};
};

class FOnlineAsyncTaskSteamLoadItemDefinitions : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
    friend FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler;

private:
    bool bInit;
    FSteamLoadItemDefinitionsComplete Delegate;
    FString FailureContext;
    TSharedPtr<FOnlineAsyncTaskSteamLoadItemDefinitionsCallbackHandler, ESPMode::ThreadSafe> CallbackHandler;
    TMap<int32, FSteamItemDefinition> ResultDefinitions;
    TArray<FSteamItemDefinitionPriceRaw> LoadedPrices;
    TArray<TTuple<ELogVerbosity::Type, FString>> ParseItemDefinitionsLogs;

    void NotifyComplete();

public:
    FOnlineAsyncTaskSteamLoadItemDefinitions(
        FSteamLoadItemDefinitionsComplete InDelegate,
        TArray<FSteamItemDefinitionPriceRaw> InPrices);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamLoadItemDefinitions);
    virtual ~FOnlineAsyncTaskSteamLoadItemDefinitions() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamLoadItemDefinitions");
    }

    virtual void Tick() override;

private:
    TArray<FSteamItemDefinition> ParseItemDefinitions(
        TArray<FSteamItemDefinitionPriceRaw> DefinitionsOptionallyWithPrices);

public:
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3496575725,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamLoadItemDefinitions)
}

#endif