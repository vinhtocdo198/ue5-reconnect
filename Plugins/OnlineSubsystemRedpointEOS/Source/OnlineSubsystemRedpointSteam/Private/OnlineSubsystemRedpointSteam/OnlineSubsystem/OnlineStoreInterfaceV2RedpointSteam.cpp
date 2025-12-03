// Copyright June Rhodes. All Rights Reserved.

#include "OnlineStoreInterfaceV2RedpointSteam.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3176367084, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{

void FOnlineStoreInterfaceV2RedpointSteam::OnQueryOffersByFilterComplete(
    const FOnlineError &Error,
    const FSteamOfferMap &Offers,
    FOnQueryOnlineStoreOffersComplete OnComplete)
{
    if (!Error.bSucceeded)
    {
        OnComplete.ExecuteIfBound(false, TArray<FUniqueOfferId>(), Error.ToLogString());
        return;
    }

    TArray<FUniqueOfferId> OfferIds;
    for (const auto &OfferKV : Offers)
    {
        OfferIds.Add(OfferKV.Key);
    }
    this->CachedOffers = Offers;

    OnComplete.ExecuteIfBound(true, OfferIds, TEXT(""));
}

FOnlineStoreInterfaceV2RedpointSteam::FOnlineStoreInterfaceV2RedpointSteam(TSharedRef<FSteamOperation> InSteamOperation)
    : SteamOperation(InSteamOperation)
{
}

void FOnlineStoreInterfaceV2RedpointSteam::QueryCategories(
    const FUniqueNetId &UserId,
    const FOnQueryOnlineStoreCategoriesComplete &Delegate)
{
    // Steam e-commerce doesn't support categories, so we just return true. There
    // are no categories for offers.
    Delegate.ExecuteIfBound(true, TEXT(""));
}

void FOnlineStoreInterfaceV2RedpointSteam::GetCategories(TArray<FOnlineStoreCategory> &OutCategories) const
{
    // Steam e-commerce doesn't support categories, so we just return true. There
    // are no categories for offers.
    OutCategories.Reset();
}

void FOnlineStoreInterfaceV2RedpointSteam::QueryOffersByFilter(
    const FUniqueNetId &UserId,
    const FOnlineStoreFilter &Filter,
    const FOnQueryOnlineStoreOffersComplete &Delegate)
{
    this->SteamOperation->QueryOffersByFilter(FSteamOperation::FOnQueryOffersByFilterComplete::CreateSP(
        this,
        &FOnlineStoreInterfaceV2RedpointSteam::OnQueryOffersByFilterComplete,
        Delegate));
}

void FOnlineStoreInterfaceV2RedpointSteam::QueryOffersById(
    const FUniqueNetId &UserId,
    const TArray<FUniqueOfferId> &OfferIds,
    const FOnQueryOnlineStoreOffersComplete &Delegate)
{
    Delegate.ExecuteIfBound(
        false,
        TArray<FUniqueOfferId>(),
        OnlineRedpointEOS::Errors::NotImplemented(
            TEXT("FOnlineStoreInterfaceV2RedpointSteam::QueryOffersById"),
            TEXT("Steam e-commerce does not support querying offers by ID; call QueryOffersByFilter instead "
                 "(which will fetch all offers)."))
            .ToLogString());
}

void FOnlineStoreInterfaceV2RedpointSteam::GetOffers(TArray<FOnlineStoreOfferRef> &OutOffers) const
{
    for (const auto &KV : this->CachedOffers)
    {
        OutOffers.Add(KV.Value.ToSharedRef());
    }
}

TSharedPtr<FOnlineStoreOffer> FOnlineStoreInterfaceV2RedpointSteam::GetOffer(const FUniqueOfferId &OfferId) const
{
    if (this->CachedOffers.Contains(OfferId))
    {
        return this->CachedOffers[OfferId];
    }

    return nullptr;
}

}

#endif