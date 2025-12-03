// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

#include "OnlineAsyncTaskManager.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamDelegates.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(2768286484, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

class FOnlineAsyncTaskSteamRequestUserInformation;

class FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler
{
private:
    FOnlineAsyncTaskSteamRequestUserInformation *Owner;
    uint64 SteamID;
    STEAM_CALLBACK(
        FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler,
        OnPersonaStateChange,
        PersonaStateChange_t);

public:
    FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler(
        class FOnlineAsyncTaskSteamRequestUserInformation *InOwner,
        uint64 InSteamID)
        : Owner(InOwner)
        , SteamID(InSteamID){};
};

class FOnlineAsyncTaskSteamRequestUserInformation : public FOnlineAsyncTaskBasic<class FOnlineSubsystemSteam>
{
    friend FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler;

private:
    bool bInit;
    uint64 SteamID;
    FSteamUserInformationFetched Delegate;
    FString FailureContext;
    TSharedPtr<FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler, ESPMode::ThreadSafe> CallbackHandler;
    void NotifyComplete();

public:
    FOnlineAsyncTaskSteamRequestUserInformation(uint64 InSteamID, FSteamUserInformationFetched InDelegate);
    UE_NONCOPYABLE(FOnlineAsyncTaskSteamRequestUserInformation);
    virtual ~FOnlineAsyncTaskSteamRequestUserInformation() = default;

    virtual FString ToString() const override
    {
        return TEXT("FOnlineAsyncTaskSteamRequestUserInformation");
    }

    virtual void Tick() override;
    virtual void Finalize() override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{
REDPOINT_EOS_FILE_NS_EXPORT(
    2768286484,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks,
    FOnlineAsyncTaskSteamRequestUserInformation)
}

#endif