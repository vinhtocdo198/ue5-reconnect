// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamRequestUserInformation.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(2768286484, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{

FOnlineAsyncTaskSteamRequestUserInformation::FOnlineAsyncTaskSteamRequestUserInformation(
    uint64 InSteamID,
    FSteamUserInformationFetched InDelegate)
    : bInit(false)
    , SteamID(InSteamID)
    , Delegate(MoveTemp(InDelegate))
    , CallbackHandler(MakeShareable(new FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler(this, InSteamID)))
{
}

void FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler::OnPersonaStateChange(PersonaStateChange_t *InEv)
{
    if (InEv && InEv->m_ulSteamID == this->SteamID)
    {
        this->Owner->NotifyComplete();
    }
}

void FOnlineAsyncTaskSteamRequestUserInformation::NotifyComplete()
{
    this->bIsComplete = true;
    this->bWasSuccessful = true;
}

void FOnlineAsyncTaskSteamRequestUserInformation::Tick()
{
    if (!this->bInit)
    {
        if (!SteamFriends()->RequestUserInformation(this->SteamID, false))
        {
            this->bIsComplete = true;
            this->bWasSuccessful = true;
            return;
        }

        this->bInit = true;
    }

    // Otherwise wait for our FOnlineAsyncTaskSteamRequestUserInformationCallbackHandler
    // to send us a notification that the work has finished.
}

void FOnlineAsyncTaskSteamRequestUserInformation::Finalize()
{
    checkf(this->bWasSuccessful, TEXT("FOnlineAsyncTaskSteamRequestUserInformation has no failure path."));

    this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success());
}

}

#endif