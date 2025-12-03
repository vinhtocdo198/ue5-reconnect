// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "OnlineError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryTypes.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3176367084, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation;

class FOnlineStoreInterfaceV2RedpointSteam
    : public IOnlineStoreV2,
      public TSharedFromThis<FOnlineStoreInterfaceV2RedpointSteam, ESPMode::ThreadSafe>
{
private:
    TSharedRef<FSteamOperation> SteamOperation;
    TMap<FUniqueOfferId, TSharedPtr<FOnlineStoreOffer>> CachedOffers;

    void OnQueryOffersByFilterComplete(
        const FOnlineError &Error,
        const FSteamOfferMap &Offers,
        FOnQueryOnlineStoreOffersComplete OnComplete);

public:
    FOnlineStoreInterfaceV2RedpointSteam(TSharedRef<FSteamOperation> InSteamOperation);
    UE_NONCOPYABLE(FOnlineStoreInterfaceV2RedpointSteam);
    virtual ~FOnlineStoreInterfaceV2RedpointSteam() override = default;

    virtual void QueryCategories(
        const FUniqueNetId &UserId,
        const FOnQueryOnlineStoreCategoriesComplete &Delegate = FOnQueryOnlineStoreCategoriesComplete()) override;
    virtual void GetCategories(TArray<FOnlineStoreCategory> &OutCategories) const override;
    virtual void QueryOffersByFilter(
        const FUniqueNetId &UserId,
        const FOnlineStoreFilter &Filter,
        const FOnQueryOnlineStoreOffersComplete &Delegate = FOnQueryOnlineStoreOffersComplete()) override;
    virtual void QueryOffersById(
        const FUniqueNetId &UserId,
        const TArray<FUniqueOfferId> &OfferIds,
        const FOnQueryOnlineStoreOffersComplete &Delegate = FOnQueryOnlineStoreOffersComplete()) override;
    virtual void GetOffers(TArray<FOnlineStoreOfferRef> &OutOffers) const override;
    virtual TSharedPtr<FOnlineStoreOffer> GetOffer(const FUniqueOfferId &OfferId) const override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3176367084,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem,
    FOnlineStoreInterfaceV2RedpointSteam)
}

#endif