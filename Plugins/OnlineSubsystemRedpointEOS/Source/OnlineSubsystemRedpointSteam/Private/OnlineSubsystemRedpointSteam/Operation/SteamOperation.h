// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Interfaces/OnlinePurchaseInterface.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/Inventory/SteamInventoryTypes.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/SDK.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(2897439011, Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;

class FSteamOperation : public TSharedFromThis<FSteamOperation>
{
public:
    typedef TDelegate<
        void(const FOnlineError &Error, const FSteamEntitlementMap &Items, const FSteamEntitlementMap &Dlcs)>
        FOnLoadAllItemsAndDlcComplete;
    typedef TDelegate<void(const FOnlineError &Error, const FSteamOfferMap &Offers)> FOnQueryOffersByFilterComplete;
    typedef TDelegate<void(const FOnlineError &Error, uint32 Width, uint32 Height, const TArray<uint8> &RGBABuffer)>
        FOnFetchAvatarComplete;

private:
    typedef TDelegate<void()> FOperation;

    TArray<TTuple<FString, FOperation>> TaskQueue;
    bool bTaskRunning;

    void TaskEnded(FString OperationName);
    void ExecuteOrSchedule(FString OperationName, FOperation Operation);

public:
    FSteamOperation() = default;
    UE_NONCOPYABLE(FSteamOperation);
    virtual ~FSteamOperation() = default;

private:
    void LoadAllItemsAndDlc_Start(FOnLoadAllItemsAndDlcComplete OnComplete);
    void LoadAllItemsAndDlc_OnGetAllItemsComplete(
        const FOnlineError &Error,
        const TArray<FSteamItemRaw> &RawItems,
        FOnLoadAllItemsAndDlcComplete OnComplete);
    void LoadAllItemsAndDlc_OnLoadItemDefinitionsComplete(
        const FOnlineError &Error,
        const FSteamItemDefinitionMap &ItemDefinitions,
        TArray<FSteamItemRaw> RawItems,
        FOnLoadAllItemsAndDlcComplete OnComplete);
    void LoadAllItemsAndDlc_OnDlcReceived(
        const FOnlineError &Error,
        const FSteamEntitlementMap &DlcEntitlements,
        FSteamEntitlementMap Items,
        FOnLoadAllItemsAndDlcComplete OnComplete);

public:
    void LoadAllItemsAndDlc(const FOnLoadAllItemsAndDlcComplete &OnComplete);

private:
    void StartPurchase_Start(FPurchaseCheckoutRequest PurchaseRequest, FOnPurchaseCheckoutComplete OnComplete);

public:
    void StartPurchase(const FPurchaseCheckoutRequest &PurchaseRequest, const FOnPurchaseCheckoutComplete &OnComplete);

private:
    void QueryOffersByFilter_Start(FOnQueryOffersByFilterComplete OnComplete);
    void QueryOffersByFilter_OnRequestPricesComplete(
        const FOnlineError &Error,
        const FString &CurrencyCode,
        const TArray<FSteamItemDefinitionPriceRaw> &RawItemDefinitionPrices,
        FOnQueryOffersByFilterComplete OnComplete);
    void QueryOffersByFilter_OnLoadItemDefinitionsComplete(
        const FOnlineError &Error,
        const FSteamItemDefinitionMap &ItemDefinitions,
        FString CurrencyCode,
        FOnQueryOffersByFilterComplete OnComplete);
    void QueryOffersByFilter_OnDlcReceived(
        const FOnlineError &Error,
        const FSteamOfferMap &DlcOffers,
        FSteamOfferMap ItemOffers,
        FOnQueryOffersByFilterComplete OnComplete);

public:
    void QueryOffersByFilter(FOnQueryOffersByFilterComplete OnComplete);

private:
    void FetchAvatar_Start(uint64 SteamID, FOnFetchAvatarComplete OnComplete);
    void FetchAvatar_OnUserInformationFetched(
        const FOnlineError &Error,
        uint64 SteamID,
        FOnFetchAvatarComplete OnComplete);
    void FetchAvatar_OnAvatarDataFetched(
        const FOnlineError &,
        uint32 Width,
        uint32 Height,
        const TArray<uint8> &RGBABuffer,
        FOnFetchAvatarComplete OnComplete);

public:
    void FetchAvatar(uint64 SteamID, FOnFetchAvatarComplete OnComplete);
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation
{
REDPOINT_EOS_FILE_NS_EXPORT(2897439011, Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation, FSteamOperation)
}

#endif