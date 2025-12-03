// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineEntitlementsInterface.h"
#include "OnlineError.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3157619689, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Operation;
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory;

class FOnlineEntitlementsInterfaceRedpointSteam
    : public IOnlineEntitlements,
      public TSharedFromThis<FOnlineEntitlementsInterfaceRedpointSteam, ESPMode::ThreadSafe>
{
private:
    TSharedRef<FSteamOperation> SteamOperation;
    bool bQueryInProgress;
    TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> CachedItems;
    TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> CachedDlcs;

    void OnAllItemsAndDlcLoaded(
        const FOnlineError &Error,
        const FSteamEntitlementMap &Items,
        const FSteamEntitlementMap &Dlcs,
        FUniqueNetIdRef UserId,
        FString Namespace);

public:
    FOnlineEntitlementsInterfaceRedpointSteam(TSharedRef<FSteamOperation> InSteamOperation);
    UE_NONCOPYABLE(FOnlineEntitlementsInterfaceRedpointSteam);
    virtual ~FOnlineEntitlementsInterfaceRedpointSteam() = default;

    virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(
        const FUniqueNetId &UserId,
        const FUniqueEntitlementId &EntitlementId) override;
    virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId &UserId, const FString &ItemId)
        override;
    virtual void GetAllEntitlements(
        const FUniqueNetId &UserId,
        const FString &Namespace,
        TArray<TSharedRef<FOnlineEntitlement>> &OutUserEntitlements) override;
    virtual bool QueryEntitlements(const FUniqueNetId &UserId, const FString &Namespace, const FPagedQuery &Page)
        override;
};

}

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem
{
REDPOINT_EOS_FILE_NS_EXPORT(
    3157619689,
    Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem,
    FOnlineEntitlementsInterfaceRedpointSteam)
}

#endif