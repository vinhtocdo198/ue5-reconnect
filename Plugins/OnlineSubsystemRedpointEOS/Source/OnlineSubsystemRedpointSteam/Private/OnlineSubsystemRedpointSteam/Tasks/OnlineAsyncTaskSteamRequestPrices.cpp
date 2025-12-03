// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamRequestPrices.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3665562259, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{

FOnlineAsyncTaskSteamRequestPrices::FOnlineAsyncTaskSteamRequestPrices(FSteamRequestPricesComplete InDelegate)
    : bInit(false)
    , PriceCallbackHandle()
    , PriceCallbackResults()
    , Delegate(MoveTemp(InDelegate))
    , FailureContext()
    , ResultCurrencyCode()
    , ResultPrices()
{
}

void FOnlineAsyncTaskSteamRequestPrices::Tick()
{
    ISteamUtils *SteamUtilsPtr = SteamUtils();
    check(SteamUtilsPtr);

    if (!this->bInit)
    {
        this->PriceCallbackHandle = SteamInventory()->RequestPrices();
        this->bInit = true;
    }

    if (this->PriceCallbackHandle != k_uAPICallInvalid)
    {
        bool bFailedCall = false;
        this->bIsComplete = SteamUtilsPtr->IsAPICallCompleted(this->PriceCallbackHandle, &bFailedCall) ? true : false;
        if (this->bIsComplete)
        {
            bool bFailedResult;
            // Retrieve the callback data from the request
            bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(
                this->PriceCallbackHandle,
                &this->PriceCallbackResults,
                sizeof(this->PriceCallbackResults),
                this->PriceCallbackResults.k_iCallback,
                &bFailedResult);
            this->bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) &&
                                   (!bFailedResult ? true : false) &&
                                   ((this->PriceCallbackResults.m_result == k_EResultOK) ? true : false);
            if (!this->bWasSuccessful)
            {
                this->FailureContext = FString::Printf(
                    TEXT("RequestPrices failed (%d, %d)"),
                    SteamUtils()->GetAPICallFailureReason(this->PriceCallbackHandle),
                    this->PriceCallbackResults.m_result);
            }
            else
            {
                this->ResultCurrencyCode = ANSI_TO_TCHAR(this->PriceCallbackResults.m_rgchCurrency);

                uint32 NumItems = SteamInventory()->GetNumItemsWithPrices();
                SteamItemDef_t *Items = (SteamItemDef_t *)FMemory::Malloc(sizeof(SteamItemDef_t) * NumItems);
                uint64 *CurrentPrices = (uint64 *)FMemory::Malloc(sizeof(uint64) * NumItems);
                uint64 *BasePrices = (uint64 *)FMemory::Malloc(sizeof(uint64) * NumItems);
                if (!SteamInventory()->GetItemsWithPrices(Items, CurrentPrices, BasePrices, NumItems))
                {
                    FMemory::Free(Items);
                    FMemory::Free(CurrentPrices);
                    FMemory::Free(BasePrices);
                    this->FailureContext = TEXT("GetItemsWithPrices call failed.");
                    this->bWasSuccessful = false;
                    return;
                }

                auto Prices = TArray<FSteamItemDefinitionPriceRaw>();
                for (uint32 i = 0; i < NumItems; i++)
                {
                    auto Price = FSteamItemDefinitionPriceRaw();
                    Price.SteamItemDefinitionId = Items[i];
                    Price.CurrentPrice = CurrentPrices[i];
                    Price.BasePrice = BasePrices[i];
                    Prices.Add(Price);
                }
                this->ResultPrices = Prices;

                FMemory::Free(Items);
                FMemory::Free(CurrentPrices);
                FMemory::Free(BasePrices);
            }
        }
    }
    else
    {
        // Invalid API call
        this->FailureContext = TEXT("RequestPrices API call invalid");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
    }
}

void FOnlineAsyncTaskSteamRequestPrices::Finalize()
{
    if (!this->bWasSuccessful)
    {
        this->Delegate.ExecuteIfBound(
            OnlineRedpointEOS::Errors::UnexpectedError(
                TEXT("FOnlineAsyncTaskSteamRequestPrices::Finalize"),
                FString::Printf(
                    TEXT("Failed to retrieve item definitions from Steam for e-commerce (%s)."),
                    *this->FailureContext)),
            TEXT(""),
            TArray<FSteamItemDefinitionPriceRaw>());
        return;
    }

    this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), this->ResultCurrencyCode, this->ResultPrices);
}

}

#endif