// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/Tasks/OnlineAsyncTaskSteamGetAvailableDLC.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryConstants.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(743794569, Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineAsyncTaskSteamGetAvailableDLC::FOnlineAsyncTaskSteamGetAvailableDLC(FSteamOffersFetched InDelegate)
    : bInit(false)
    , Delegate(MoveTemp(InDelegate))
    , FailureContext()
    , Result()
{
}

void FOnlineAsyncTaskSteamGetAvailableDLC::Tick()
{
    // Convert DLC apps into offers.
    TMap<FUniqueOfferId, TSharedPtr<FOnlineStoreOffer>> Offers;
    int DlcCount = SteamApps()->GetDLCCount();
    for (int i = 0; i < DlcCount; i++)
    {
        AppId_t DlcId = 0;
        bool bAvailable = false;
        char NameBuffer[2049];
        FMemory::Memzero(NameBuffer);
        if (SteamApps()->BGetDLCDataByIndex(i, &DlcId, &bAvailable, NameBuffer, 2048))
        {
            // We only add available DLC to the list. Only limited information is
            // available for DLC items.
#if defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING
            if (bAvailable)
#endif // #if defined(UE_BUILD_SHIPPING) && UE_BUILD_SHIPPING
            {
                TSharedRef<FOnlineStoreOffer> Offer = MakeShared<FOnlineStoreOffer>();
                Offer->OfferId = FString::Printf(TEXT("%s:%u"), Inventory::SteamTypeDlc, DlcId);

                Offer->Title = FText::FromString(UTF8_TO_TCHAR(NameBuffer));
                Offer->Description = FText::GetEmpty();
                Offer->LongDescription = FText::GetEmpty();

                Offer->RegularPriceText = NSLOCTEXT("OnlineSubsystemRedpointSteam", "DLCNoPriceAvailable", "N/A");
                Offer->RegularPrice = -1;

                Offer->PriceText = NSLOCTEXT("OnlineSubsystemRedpointSteam", "DLCNoPriceAvailable", "N/A");
                Offer->NumericPrice = -1;

                Offer->CurrencyCode = TEXT("");

                Offer->ReleaseDate = FDateTime::MinValue();
                Offer->ExpirationDate = FDateTime::MaxValue();
                Offer->DiscountType = EOnlineStoreOfferDiscountType::NotOnSale;

                Offer->DynamicFields = TMap<FString, FString>();

                Offers.Add(Offer->OfferId, Offer);
            }
        }
    }
    this->Result = Offers;

    this->bIsComplete = true;
    this->bWasSuccessful = true;
}

void FOnlineAsyncTaskSteamGetAvailableDLC::Finalize()
{
    this->Delegate.ExecuteIfBound(OnlineRedpointEOS::Errors::Success(), this->Result);
}

}

#endif