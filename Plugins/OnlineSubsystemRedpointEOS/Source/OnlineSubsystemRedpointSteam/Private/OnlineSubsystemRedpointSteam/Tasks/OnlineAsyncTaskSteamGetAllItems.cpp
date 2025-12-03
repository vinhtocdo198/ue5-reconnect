// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetAllItems.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1294003128, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineAsyncTaskSteamGetAllItems::FOnlineAsyncTaskSteamGetAllItems(FSteamGetAllItemsComplete InDelegate)
    : bInit(false)
    , bInventoryServiceDisabled(false)
    , Result()
    , Delegate(MoveTemp(InDelegate))
    , FailureContext()
    , ResultItems()
    , Warnings()
{
}

void FOnlineAsyncTaskSteamGetAllItems::AttemptFinalizeOnTickThread()
{
    // Get the raw items from the result.
    uint32 ItemCount = 0;
    if (!SteamInventory()->GetResultItems(this->Result, nullptr, &ItemCount))
    {
        this->FailureContext = TEXT("GetResultItems failed (count retrieval).");
        this->bWasSuccessful = false;
        return;
    }
    if (ItemCount == 0)
    {
        this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), TArray<Inventory::FSteamItemRaw>());
        return;
    }
    SteamItemDetails_t *Items = (SteamItemDetails_t *)FMemory::Malloc(sizeof(SteamItemDetails_t) * ItemCount);
    if (!SteamInventory()->GetResultItems(this->Result, Items, &ItemCount))
    {
        this->FailureContext = TEXT("GetResultItems failed (fill operation).");
        this->bWasSuccessful = false;
        return;
    }

    // Return the raw item list.
    auto Results = TArray<Inventory::FSteamItemRaw>();
    for (uint32 i = 0; i < ItemCount; i++)
    {
        auto Entry = Inventory::FSteamItemRaw();
        Entry.SteamItemId = Items[i].m_itemId;
        Entry.SteamItemDefinitionId = Items[i].m_iDefinition;
        Entry.Quantity = Items[i].m_unQuantity;
        Entry.Flags = Items[i].m_unFlags;
        Results.Add(Entry);
    }
    this->ResultItems = Results;
}

void FOnlineAsyncTaskSteamGetAllItems::Tick()
{
    if (!this->bInit)
    {
        if (!SteamInventory()->GetAllItems(&this->Result))
        {
            this->bIsComplete = true;
            this->bWasSuccessful = false;
            this->FailureContext = TEXT("GetAllItems call failed to start.");
            return;
        }

        this->bInit = true;
    }

    EResult SteamResult = SteamInventory()->GetResultStatus(this->Result);
    switch (SteamResult)
    {
    case k_EResultPending:
        // We are waiting on the operation to complete.
        return;
    case k_EResultOK:
    case k_EResultExpired:
        // This is complete.
        this->bIsComplete = true;
        this->bWasSuccessful = true;
        break;
    case k_EResultInvalidParam:
        this->FailureContext = TEXT("GetAllItems failed (invalid param)");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        break;
    case k_EResultServiceUnavailable:
        this->FailureContext = TEXT("GetAllItems failed (unavailable)");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        break;
    case k_EResultLimitExceeded:
        this->FailureContext = TEXT("GetAllItems failed (limit exceeded)");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        break;
    case k_EResultFail:
        this->FailureContext = TEXT("GetAllItems failed (fail)");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        this->bInventoryServiceDisabled = true;
        break;
    default:
        this->FailureContext = TEXT("GetAllItems failed (unknown)");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
        break;
    }

    // If this operation is complete, finalise the result.
    if (this->bIsComplete)
    {
        if (this->bWasSuccessful)
        {
            // This function will set bWasSuccessful to false if it fails.
            this->AttemptFinalizeOnTickThread();
        }

        // Release Steam inventory result.
        SteamInventory()->DestroyResult(this->Result);
    }
}

void FOnlineAsyncTaskSteamGetAllItems::Finalize()
{
    for (const auto &Warning : this->Warnings)
    {
        UE_LOG(LogRedpointSteam, Warning, TEXT("%s"), *Warning);
    }

    if (!this->bWasSuccessful)
    {
        if (this->bInventoryServiceDisabled)
        {
            UE_LOG(
                LogRedpointSteam,
                Warning,
                TEXT("Unable to load player inventory: Steam Inventory Service is probably turned off. DLC offers will "
                     "be returned but items will be skipped."));
            this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), TArray<Inventory::FSteamItemRaw>());
        }
        else
        {
            this->Delegate.ExecuteIfBound(
                OnlineRedpointEOS::Errors::UnexpectedError(
                    TEXT("FOnlineAsyncTaskSteamGetAllItems::Finalize"),
                    *this->FailureContext),
                TArray<Inventory::FSteamItemRaw>());
        }
    }
    else
    {
        this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), this->ResultItems);
    }
}

}

#endif