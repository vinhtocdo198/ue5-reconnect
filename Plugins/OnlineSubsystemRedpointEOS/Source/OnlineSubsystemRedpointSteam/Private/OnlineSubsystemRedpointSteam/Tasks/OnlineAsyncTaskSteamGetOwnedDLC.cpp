// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetOwnedDLC.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1697498403, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineAsyncTaskSteamGetOwnedDLC::FOnlineAsyncTaskSteamGetOwnedDLC(FSteamEntitlementsFetched InDelegate)
    : bInit(false)
    , Delegate(MoveTemp(InDelegate))
    , FailureContext()
    , Result()
{
}

void FOnlineAsyncTaskSteamGetOwnedDLC::Tick()
{
    // Convert owned DLC apps into entitlements.
    TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> Entitlements;
    int DlcCount = SteamApps()->GetDLCCount();
    for (int i = 0; i < DlcCount; i++)
    {
        AppId_t DlcId = 0;
        bool bAvailable = false;
        char NameBuffer[2049];
        FMemory::Memzero(NameBuffer);
        if (SteamApps()->BGetDLCDataByIndex(i, &DlcId, &bAvailable, NameBuffer, 2048))
        {
            // We ignore bAvailable for owned DLC, because that represents whether
            // the DLC is available in the Steam store, and the user might
            // own DLC that is no longer available for sale.
            if (SteamApps()->BIsSubscribedApp(DlcId) || SteamApps()->BIsDlcInstalled(DlcId))
            {
                TSharedRef<FOnlineEntitlement> Entitlement = MakeShared<FOnlineEntitlement>();
                Entitlement->Id = FString::Printf(TEXT("%s:%u"), Inventory::SteamTypeDlc, DlcId);

                Entitlement->Name = UTF8_TO_TCHAR(NameBuffer);
                Entitlement->ItemId = FString::Printf(TEXT("%s:%u"), Inventory::SteamTypeDlc, DlcId);
                Entitlement->Namespace = Inventory::SteamTypeDlc;

                Entitlement->bIsConsumable = false;
                Entitlement->RemainingCount = 1;
                Entitlement->ConsumedCount = 0;

                Entitlement->StartDate = TEXT("");
                Entitlement->EndDate = TEXT("");
                Entitlement->Status = SteamApps()->BIsDlcInstalled(DlcId) ? TEXT("Installed") : TEXT("Owned");

                Entitlements.Add(Entitlement->Id, Entitlement);
            }
        }
    }
    this->Result = Entitlements;

    this->bIsComplete = true;
    this->bWasSuccessful = true;
}

void FOnlineAsyncTaskSteamGetOwnedDLC::Finalize()
{
    this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), this->Result);
}

}

#endif