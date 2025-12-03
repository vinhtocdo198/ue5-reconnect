// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamPurchase.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(1721094416, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineAsyncTaskSteamPurchase::FOnlineAsyncTaskSteamPurchase(
    const FPurchaseCheckoutRequest &InCheckoutRequest,
    const FOnPurchaseCheckoutComplete &InDelegate)
    : bInit(false)
    , CheckoutRequest(InCheckoutRequest)
    , Delegate(InDelegate)
    , CallbackHandle()
    , CallbackResults()
    , FailureContext()
    , OrderId()
    , TransactionId()
{
}

void FOnlineAsyncTaskSteamPurchase::Tick()
{
    ISteamUtils *SteamUtilsPtr = SteamUtils();
    check(SteamUtilsPtr);

    if (!this->bInit)
    {
        TArray<FPurchaseCheckoutRequest::FPurchaseOfferEntry> ItemOffers;
        for (const auto &Entry : this->CheckoutRequest.PurchaseOffers)
        {
            if (Entry.OfferId.StartsWith(FString::Printf(TEXT("%s:"), Inventory::SteamTypeItemDefinition)))
            {
                if (Entry.Quantity >= 1)
                {
                    ItemOffers.Add(Entry);
                }
            }
        }

        SteamItemDef_t *Items = (SteamItemDef_t *)FMemory::MallocZeroed(sizeof(SteamItemDef_t) * ItemOffers.Num());
        uint32 *Quantities = (uint32 *)FMemory::MallocZeroed(sizeof(uint32) * ItemOffers.Num());

        for (int i = 0; i < ItemOffers.Num(); i++)
        {
            Items[i] =
                FCString::Atoi(*ItemOffers[i].OfferId.Mid(FString(Inventory::SteamTypeItemDefinition).Len() + 1));
            Quantities[i] = FMath::Max(1, ItemOffers[i].Quantity);
        }

        this->CallbackHandle = SteamInventory()->StartPurchase(Items, Quantities, ItemOffers.Num());
        this->bInit = true;

        FMemory::Free(Items);
        FMemory::Free(Quantities);
    }

    if (this->CallbackHandle != k_uAPICallInvalid)
    {
        bool bFailedCall = false;
        this->bIsComplete = SteamUtilsPtr->IsAPICallCompleted(this->CallbackHandle, &bFailedCall) ? true : false;
        if (this->bIsComplete)
        {
            bool bFailedResult;
            // Retrieve the callback data from the request
            bool bSuccessCallResult = SteamUtilsPtr->GetAPICallResult(
                this->CallbackHandle,
                &this->CallbackResults,
                sizeof(this->CallbackResults),
                this->CallbackResults.k_iCallback,
                &bFailedResult);
            this->bWasSuccessful = (bSuccessCallResult ? true : false) && (!bFailedCall ? true : false) &&
                                   (!bFailedResult ? true : false) &&
                                   ((this->CallbackResults.m_result == k_EResultOK) ? true : false);
            if (!this->bWasSuccessful)
            {
                this->FailureContext = FString::Printf(
                    TEXT("StartPurchase failed (%d, %d)"),
                    SteamUtils()->GetAPICallFailureReason(this->CallbackHandle),
                    this->CallbackResults.m_result);
            }
            else
            {
                this->OrderId = this->CallbackResults.m_ulOrderID;
                this->TransactionId = this->CallbackResults.m_ulTransID;
            }
        }
    }
    else
    {
        // Invalid API call
        this->FailureContext = TEXT("StartPurchase API call invalid");
        this->bIsComplete = true;
        this->bWasSuccessful = false;
    }
}

void FOnlineAsyncTaskSteamPurchase::Finalize()
{
    if (!this->bWasSuccessful)
    {
        this->Delegate.ExecuteIfBound(
            OnlineRedpointEOS::Errors::UnexpectedError(
                ANSI_TO_TCHAR(__FUNCTION__),
                FString::Printf(
                    TEXT("Failed to retrieve item definitions from Steam for e-commerce (%s)."),
                    *this->FailureContext)),
            MakeShared<FPurchaseReceipt>());
    }
    else
    {
        auto Receipt = MakeShared<FPurchaseReceipt>();
        Receipt->TransactionId = FString::Printf(TEXT("%llu"), this->TransactionId);
        // Steam doesn't actually tell us when
        // this particular purchase is complete!
        Receipt->TransactionState = EPurchaseTransactionState::Processing;
        for (const auto &Entry : this->CheckoutRequest.PurchaseOffers)
        {
            if (Entry.Quantity >= 1)
            {
                Receipt->ReceiptOffers.Add(
                    FPurchaseReceipt::FReceiptOfferEntry(Entry.OfferNamespace, Entry.OfferId, Entry.Quantity));
            }
        }
        this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), Receipt);
    }
}

}

#endif