// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemSteamProtectedAccessor.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetAllItems.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetAvailableDLC.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetLargeFriendAvatar.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetOwnedDLC.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamLoadItemDefinitions.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamPurchase.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamRequestPrices.h"
#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamRequestUserInformation.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(2897439011, Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

void FSteamOperation::TaskEnded(FString OperationName)
{
    UE_LOG(LogRedpointSteam, Verbose, TEXT("Operation '%s' has now finished."), *OperationName);
    if (this->TaskQueue.Num() > 0)
    {
        auto NextTask = this->TaskQueue[0];
        this->TaskQueue.RemoveAt(0);
        if (NextTask.Value.IsBound())
        {
            UE_LOG(
                LogRedpointSteam,
                Verbose,
                TEXT("Executing scheduled operation '%s' because the previous operation has finished."),
                *NextTask.Key);
            NextTask.Value.Execute();
        }
        else
        {
            UE_LOG(
                LogRedpointSteam,
                Warning,
                TEXT("Ignoring scheduled operation '%s' because the operation delegate is not bound!"),
                *NextTask.Key);
            this->TaskEnded(NextTask.Key);
        }
    }
    else
    {
        UE_LOG(LogRedpointSteam, Verbose, TEXT("The last operation has now finished and the task queue is empty."));
        this->bTaskRunning = false;
    }
}

void FSteamOperation::ExecuteOrSchedule(FString OperationName, FOperation Operation)
{
    if (Operation.IsBound())
    {
        if (this->bTaskRunning)
        {
            UE_LOG(
                LogRedpointSteam,
                Verbose,
                TEXT("Scheduling operation '%s' because another operation is already in progress."),
                *OperationName);
            this->TaskQueue.Add(TTuple<FString, FOperation>(OperationName, Operation));
        }
        else
        {
            UE_LOG(
                LogRedpointSteam,
                Verbose,
                TEXT("Immediately executing operation '%s' because no operation is in progress."),
                *OperationName);
            this->bTaskRunning = true;
            Operation.Execute();
        }
    }
    else
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("Ignoring operation '%s' because the operation delegate is not bound!"),
            *OperationName);
    }
}

void FSteamOperation::LoadAllItemsAndDlc_Start(FOnLoadAllItemsAndDlcComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamEntitlementMap(),
            FSteamEntitlementMap());
        return;
    }

    // Load definitions.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamGetAllItems *ItemsTask =
        new FOnlineAsyncTaskSteamGetAllItems(FSteamGetAllItemsComplete::CreateSP(
            this,
            &FSteamOperation::LoadAllItemsAndDlc_OnGetAllItemsComplete,
            OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(ItemsTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::LoadAllItemsAndDlc_OnGetAllItemsComplete(
    const FOnlineError &Error,
    const TArray<FSteamItemRaw> &RawItems,
    FOnLoadAllItemsAndDlcComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation was successful.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamEntitlementMap(), FSteamEntitlementMap());
        return;
    }

    // If the user has no items, then skip straight to loading DLC as we won't be able to make a call to LoadDefinitions
    // or GetItemDefinitionIDs in the Steamworks API.
    if (RawItems.Num() == 0)
    {
        UE_LOG(
            LogRedpointSteam,
            Verbose,
            TEXT("%s: User has no inventory items, skipping item definition load."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->LoadAllItemsAndDlc_OnLoadItemDefinitionsComplete(
            OnlineRedpointEOS::Errors::Success(),
            FSteamItemDefinitionMap(),
            TArray<FSteamItemRaw>(),
            OnComplete);
        return;
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamEntitlementMap(),
            FSteamEntitlementMap());
        return;
    }

    // Load definitions.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamLoadItemDefinitions *LoadTask = new FOnlineAsyncTaskSteamLoadItemDefinitions(
        FSteamLoadItemDefinitionsComplete::CreateSP(
            this,
            &FSteamOperation::LoadAllItemsAndDlc_OnLoadItemDefinitionsComplete,
            RawItems,
            OnComplete),
        TArray<FSteamItemDefinitionPriceRaw>());
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(LoadTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::LoadAllItemsAndDlc_OnLoadItemDefinitionsComplete(
    const FOnlineError &Error,
    const FSteamItemDefinitionMap &ItemDefinitions,
    TArray<FSteamItemRaw> RawItems,
    FOnLoadAllItemsAndDlcComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation was successful.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamEntitlementMap(), FSteamEntitlementMap());
        return;
    }

    // We must now assemble the raw items and item definitions into the entitlement map. This no longer happens in
    // FOnlineAsyncTaskSteamGetAllItems, since that task happens before the item definitions might be loaded.
    TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> Entitlements;
    for (const auto &Item : RawItems)
    {
        if (!ItemDefinitions.Contains(Item.SteamItemDefinitionId))
        {
            UE_LOG(
                LogRedpointSteam,
                Warning,
                TEXT("%s: Ignoring item with ID '%llu' and definition ID '%d' because we could not fetch metadata "
                     "about this definition from the Steamworks API."),
                ANSI_TO_TCHAR(__FUNCTION__),
                Item.SteamItemId,
                Item.SteamItemDefinitionId);
            continue;
        }

        TSharedRef<FOnlineEntitlement> Entitlement = MakeShared<FOnlineEntitlement>();
        Entitlement->Id = FString::Printf(TEXT("%s:%llu"), Inventory::SteamTypeItem, Item.SteamItemId);

        const FSteamItemDefinition &ItemDef = ItemDefinitions[Item.SteamItemDefinitionId];

        Entitlement->Name = ItemDef.Properties.Contains(TEXT("name")) ? ItemDef.Properties[TEXT("name")] : TEXT("");

        Entitlement->ItemId =
            FString::Printf(TEXT("%s:%d"), Inventory::SteamTypeItemDefinition, Item.SteamItemDefinitionId);

        Entitlement->Namespace = Inventory::SteamTypeItem;

        Entitlement->bIsConsumable = ItemDef.Properties.Contains(TEXT("consumable"))
                                         ? ItemDef.Properties[TEXT("consumable")] == TEXT("true")
                                         : false;
        Entitlement->RemainingCount = Item.Quantity;
        Entitlement->ConsumedCount = 0;

        Entitlement->StartDate = TEXT("");
        Entitlement->EndDate = TEXT("");

        Entitlement->Status = (Item.Flags & k_ESteamItemNoTrade) != 0 ? TEXT("NotTradeable") : TEXT("Owned");

        Entitlements.Add(Entitlement->Id, Entitlement);
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    IOnlineSubsystem *OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamEntitlementMap(),
            FSteamEntitlementMap());
        return;
    }

    // Request owned DLC.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamGetOwnedDLC *DlcTask =
        new FOnlineAsyncTaskSteamGetOwnedDLC(FSteamEntitlementsFetched::CreateSP(
            this,
            &FSteamOperation::LoadAllItemsAndDlc_OnDlcReceived,
            Entitlements,
            OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(DlcTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::LoadAllItemsAndDlc_OnDlcReceived(
    const FOnlineError &Error,
    const FSteamEntitlementMap &DlcEntitlements,
    FSteamEntitlementMap Items,
    FOnLoadAllItemsAndDlcComplete OnComplete)
{
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the DLC load failed.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamEntitlementMap(), FSteamEntitlementMap());
        return;
    }

    OnComplete.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), Items, DlcEntitlements);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::LoadAllItemsAndDlc(const FOnLoadAllItemsAndDlcComplete &OnComplete)
{
    
    FString OperationName = TEXT("LoadAllItemsAndDlc");
    this->ExecuteOrSchedule(
        OperationName,
        FOperation::CreateSP(
            this,
            &FSteamOperation::LoadAllItemsAndDlc_Start,
            FOnLoadAllItemsAndDlcComplete::CreateSPLambda(
                this,
                [this, OnComplete, OperationName](
                    const FOnlineError &Error,
                    const FSteamEntitlementMap &Items,
                    const FSteamEntitlementMap &Dlcs) {
                    this->TaskEnded(OperationName);
                    OnComplete.ExecuteIfBound(Error, Items, Dlcs);
                })));
}

void FSteamOperation::StartPurchase_Start(
    FPurchaseCheckoutRequest PurchaseRequest,
    FOnPurchaseCheckoutComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Start the purchase of the items.
    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    IOnlineSubsystem *OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            MakeShared<FPurchaseReceipt>());
        return;
    }

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamPurchase *NewTask = new FOnlineAsyncTaskSteamPurchase(PurchaseRequest, OnComplete);
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(NewTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::StartPurchase(
    const FPurchaseCheckoutRequest &PurchaseRequest,
    const FOnPurchaseCheckoutComplete &OnComplete)
{
    
    FString OperationName = TEXT("StartPurchase");
    this->ExecuteOrSchedule(
        OperationName,
        FOperation::CreateSP(
            this,
            &FSteamOperation::StartPurchase_Start,
            PurchaseRequest,
            FOnPurchaseCheckoutComplete::CreateSPLambda(
                this,
                [this,
                 OnComplete,
                 OperationName](const FOnlineError &Result, const TSharedRef<FPurchaseReceipt> &Receipt) {
                    this->TaskEnded(OperationName);
                    OnComplete.ExecuteIfBound(Result, Receipt);
                })));
}

void FSteamOperation::QueryOffersByFilter_Start(FOnQueryOffersByFilterComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamOfferMap());
        return;
    }

    // Now request item prices.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamRequestPrices *PricesTask =
        new FOnlineAsyncTaskSteamRequestPrices(FSteamRequestPricesComplete::CreateSP(
            this,
            &FSteamOperation::QueryOffersByFilter_OnRequestPricesComplete,
            OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(PricesTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::QueryOffersByFilter_OnRequestPricesComplete(
    const FOnlineError &Error,
    const FString &CurrencyCode,
    const TArray<FSteamItemDefinitionPriceRaw> &RawItemDefinitionPrices,
    FOnQueryOffersByFilterComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation succeeded.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamOfferMap());
        return;
    }

    // If there are no prices (i.e. no items available for sale), skip calling LoadItemDefinitions and go straight to
    // loading DLC.
    if (RawItemDefinitionPrices.Num() == 0)
    {
        UE_LOG(
            LogRedpointSteam,
            Verbose,
            TEXT("%s: Store has no items available for sale, skipping item definition load."),
            ANSI_TO_TCHAR(__FUNCTION__));
        this->QueryOffersByFilter_OnLoadItemDefinitionsComplete(
            OnlineRedpointEOS::Errors::Success(),
            FSteamItemDefinitionMap(),
            TEXT(""),
            OnComplete);
        return;
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamOfferMap());
        return;
    }

    // Load definitions.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamLoadItemDefinitions *LoadTask = new FOnlineAsyncTaskSteamLoadItemDefinitions(
        FSteamLoadItemDefinitionsComplete::CreateSP(
            this,
            &FSteamOperation::QueryOffersByFilter_OnLoadItemDefinitionsComplete,
            CurrencyCode,
            OnComplete),
        RawItemDefinitionPrices);
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(LoadTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::QueryOffersByFilter_OnLoadItemDefinitionsComplete(
    const FOnlineError &Error,
    const FSteamItemDefinitionMap &ItemDefinitions,
    FString CurrencyCode,
    FOnQueryOffersByFilterComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation succeeded.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamOfferMap());
        return;
    }

    // Convert the loaded item definitions into offers.
    TMap<FUniqueOfferId, TSharedPtr<FOnlineStoreOffer>> Offers;
    for (const auto &ItemKV : ItemDefinitions)
    {
        const auto &ItemDef = ItemKV.Value;

        TSharedRef<FOnlineStoreOffer> Offer = MakeShared<FOnlineStoreOffer>();
        Offer->OfferId =
            FString::Printf(TEXT("%s:%d"), Inventory::SteamTypeItemDefinition, ItemDef.SteamItemDefinitionId);

        Offer->Title = ItemDef.Properties.Contains(TEXT("name")) ? FText::FromString(ItemDef.Properties[TEXT("name")])
                                                                 : FText::GetEmpty();
        Offer->Description = ItemDef.Properties.Contains(TEXT("description"))
                                 ? FText::FromString(ItemDef.Properties[TEXT("description")])
                                 : FText::GetEmpty();
        Offer->LongDescription = ItemDef.Properties.Contains(TEXT("long_description"))
                                     ? FText::FromString(ItemDef.Properties[TEXT("long_description")])
                                     : FText::GetEmpty();

        Offer->RegularPriceText = FText::GetEmpty();
        Offer->RegularPrice = ItemDef.BasePrice;

        Offer->PriceText = FText::GetEmpty();
        Offer->NumericPrice = ItemDef.CurrentPrice;

        Offer->CurrencyCode = CurrencyCode;

        if (ItemDef.Properties.Contains(TEXT("date_created")))
        {
            // Parse the date value which comes in the form "20221008T103044Z".
            FString DateString = ItemDef.Properties[TEXT("date_created")];
            int32 Year = FCString::Atoi(*DateString.Mid(0, 4));
            int32 Month = FCString::Atoi(*DateString.Mid(4, 2));
            int32 Day = FCString::Atoi(*DateString.Mid(6, 2));
            int32 Hour = FCString::Atoi(*DateString.Mid(9, 2));
            int32 Minute = FCString::Atoi(*DateString.Mid(11, 2));
            int32 Second = FCString::Atoi(*DateString.Mid(13, 2));
            Offer->ReleaseDate = FDateTime(Year, Month, Day, Hour, Minute, Second);
        }
        else
        {
            Offer->ReleaseDate = FDateTime::MinValue();
        }
        Offer->ExpirationDate = FDateTime::MaxValue();
        Offer->DiscountType = Offer->RegularPrice != Offer->NumericPrice ? EOnlineStoreOfferDiscountType::Percentage
                                                                         : EOnlineStoreOfferDiscountType::NotOnSale;

        Offer->DynamicFields = ItemDef.Properties;

        Offers.Add(Offer->OfferId, Offer);
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamOfferMap());
        return;
    }

    // Now request DLC items.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamGetAvailableDLC *DlcTask =
        new FOnlineAsyncTaskSteamGetAvailableDLC(FSteamOffersFetched::CreateThreadSafeSP(
            this,
            &FSteamOperation::QueryOffersByFilter_OnDlcReceived,
            Offers,
            OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)->AddToInQueue(DlcTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::QueryOffersByFilter_OnDlcReceived(
    const FOnlineError &Error,
    const FSteamOfferMap &DlcOffers,
    FSteamOfferMap ItemOffers,
    FOnQueryOffersByFilterComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation succeeded.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, FSteamOfferMap());
        return;
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            FSteamOfferMap());
        return;
    }

    // Add the DLC items to the ItemOffers copy, and then return all the offers.
    for (const auto &DlcKV : DlcOffers)
    {
        ItemOffers.Add(DlcKV.Key, DlcKV.Value);
    }
    OnComplete.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), ItemOffers);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::QueryOffersByFilter(FOnQueryOffersByFilterComplete OnComplete)
{
    
    FString OperationName = TEXT("QueryOffersByFilter");
    this->ExecuteOrSchedule(
        OperationName,
        FOperation::CreateSP(
            this,
            &FSteamOperation::QueryOffersByFilter_Start,
            FOnQueryOffersByFilterComplete::CreateSPLambda(
                this,
                [this, OnComplete, OperationName](const FOnlineError &Error, const FSteamOfferMap &Offers) {
                    this->TaskEnded(OperationName);
                    OnComplete.ExecuteIfBound(Error, Offers);
                })));
}

void FSteamOperation::FetchAvatar_Start(uint64 SteamID, FOnFetchAvatarComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            0,
            0,
            TArray<uint8>());
        return;
    }

    // Request user information.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamRequestUserInformation *RequestUserInformationTask =
        new FOnlineAsyncTaskSteamRequestUserInformation(
            SteamID,
            FSteamUserInformationFetched::CreateThreadSafeSP(
                this,
                &FSteamOperation::FetchAvatar_OnUserInformationFetched,
                SteamID,
                OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)
        ->AddToInQueue(RequestUserInformationTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::FetchAvatar_OnUserInformationFetched(
    const FOnlineError &Error,
    uint64 SteamID,
    FOnFetchAvatarComplete OnComplete)
{
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem;
    using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks;

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    // Check if the operation succeeded.
    if (!Error.WasSuccessful())
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Callback from async task operation failed."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(Error, 0, 0, TArray<uint8>());
        return;
    }

    // NOLINTNEXTLINE(unreal-ionlinesubsystem-get)
    FOnlineSubsystemSteam *OSS = (FOnlineSubsystemSteam *)IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
    if (OSS == nullptr)
    {
        UE_LOG(
            LogRedpointSteam,
            Warning,
            TEXT("%s: Steam online subsystem has gone away."),
            ANSI_TO_TCHAR(__FUNCTION__));
        OnComplete.ExecuteIfBound(
            OnlineRedpointEOS::Errors::Missing_Feature(
                ANSI_TO_TCHAR(__FUNCTION__),
                TEXT("The Steam online subsystem is not available.")),
            0,
            0,
            TArray<uint8>());
        return;
    }

    // Now fetch the avatar.
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Pushing async task onto queue."), ANSI_TO_TCHAR(__FUNCTION__));
    FOnlineAsyncTaskSteamGetLargeFriendAvatar *GetLargeFriendAvatarTask = new FOnlineAsyncTaskSteamGetLargeFriendAvatar(
        SteamID,
        FSteamAvatarDataFetched::CreateSP(this, &FSteamOperation::FetchAvatar_OnAvatarDataFetched, OnComplete));
    FOnlineSubsystemSteamProtectedAccessor::GetAsyncTaskRunner((FOnlineSubsystemSteam *)OSS)
        ->AddToInQueue(GetLargeFriendAvatarTask);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::FetchAvatar_OnAvatarDataFetched(
    const FOnlineError &Error,
    uint32 Width,
    uint32 Height,
    const TArray<uint8> &RGBABuffer,
    FOnFetchAvatarComplete OnComplete)
{
    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Started."), ANSI_TO_TCHAR(__FUNCTION__));

    OnComplete.ExecuteIfBound(Error, Width, Height, RGBABuffer);

    UE_LOG(LogRedpointSteam, Verbose, TEXT("%s: Finished."), ANSI_TO_TCHAR(__FUNCTION__));
}

void FSteamOperation::FetchAvatar(uint64 SteamID, FOnFetchAvatarComplete OnComplete)
{
    
    FString OperationName = TEXT("FetchAvatar");
    this->ExecuteOrSchedule(
        OperationName,
        FOperation::CreateSP(
            this,
            &FSteamOperation::FetchAvatar_Start,
            SteamID,
            FOnFetchAvatarComplete::CreateSPLambda(
                this,
                [this, OnComplete, OperationName](
                    const FOnlineError &Error,
                    uint32 Width,
                    uint32 Height,
                    const TArray<uint8> &RGBABuffer) {
                    this->TaskEnded(OperationName);
                    OnComplete.ExecuteIfBound(Error, Width, Height, RGBABuffer);
                })));
}

}

#endif