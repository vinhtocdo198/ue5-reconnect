// Copyright June Rhodes. All Rights Reserved.

#include "OnlineEntitlementsInterfaceRedpointSteam.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemRedpointEOS/Public/EOSError.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3157619689, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{
using namespace ::Redpoint::EOS::OnlineSubsystemRedpointSteam;

FOnlineEntitlementsInterfaceRedpointSteam::FOnlineEntitlementsInterfaceRedpointSteam(
    TSharedRef<FSteamOperation> InSteamOperation)
    : SteamOperation(InSteamOperation)
    , bQueryInProgress(false)
    , CachedItems()
    , CachedDlcs()
{
}

void FOnlineEntitlementsInterfaceRedpointSteam::OnAllItemsAndDlcLoaded(
    const FOnlineError &Error,
    const FSteamEntitlementMap &Items,
    const FSteamEntitlementMap &Dlcs,
    FUniqueNetIdRef UserId,
    FString Namespace)
{
    if (Error.bSucceeded)
    {
        this->CachedItems = Items;
        this->CachedDlcs = Dlcs;
    }

    this->bQueryInProgress = false;
    this->TriggerOnQueryEntitlementsCompleteDelegates(Error.bSucceeded, *UserId, Namespace, Error.ToLogString());
}

TSharedPtr<FOnlineEntitlement> FOnlineEntitlementsInterfaceRedpointSteam::GetEntitlement(
    const FUniqueNetId &UserId,
    const FUniqueEntitlementId &EntitlementId)
{
    if (this->CachedDlcs.Contains(EntitlementId))
    {
        return this->CachedDlcs[EntitlementId];
    }
    if (this->CachedItems.Contains(EntitlementId))
    {
        return this->CachedItems[EntitlementId];
    }

    return nullptr;
}

TSharedPtr<FOnlineEntitlement> FOnlineEntitlementsInterfaceRedpointSteam::GetItemEntitlement(
    const FUniqueNetId &UserId,
    const FString &ItemId)
{
    UE_LOG(
        LogRedpointSteam,
        Warning,
        TEXT("IOnlineEntitlement::GetItemEntitlement should not be used on Steam, as there may be multiple "
             "entitlements (inventory items) with the same item ID (the same item type)."));

    for (const auto &Entitlement : this->CachedDlcs)
    {
        if (Entitlement.Value->ItemId == ItemId)
        {
            return Entitlement.Value;
        }
    }
    for (const auto &Entitlement : this->CachedItems)
    {
        if (Entitlement.Value->ItemId == ItemId)
        {
            return Entitlement.Value;
        }
    }

    return nullptr;
}

void FOnlineEntitlementsInterfaceRedpointSteam::GetAllEntitlements(
    const FUniqueNetId &UserId,
    const FString &Namespace,
    TArray<TSharedRef<FOnlineEntitlement>> &OutUserEntitlements)
{
    for (const auto &KV : this->CachedDlcs)
    {
        // Include if the requested namespace is empty string (global), or if
        // the namespace matches.
        if (Namespace.IsEmpty() || Namespace == KV.Value->Namespace)
        {
            OutUserEntitlements.Add(KV.Value.ToSharedRef());
        }
    }
    for (const auto &KV : this->CachedItems)
    {
        // Include if the requested namespace is empty string (global), or if
        // the namespace matches.
        if (Namespace.IsEmpty() || Namespace == KV.Value->Namespace)
        {
            OutUserEntitlements.Add(KV.Value.ToSharedRef());
        }
    }
}

bool FOnlineEntitlementsInterfaceRedpointSteam::QueryEntitlements(
    const FUniqueNetId &UserId,
    const FString &Namespace,
    const FPagedQuery &Page)
{
    if (this->bQueryInProgress)
    {
        UE_LOG(
            LogRedpointSteam,
            Error,
            TEXT("Another FOnlineEntitlementsInterfaceRedpointSteam::QueryEntitlements operation is in progress. You "
                 "can not concurrently query entitlements in Steam."));
        return false;
    }
    this->bQueryInProgress = true;

    this->SteamOperation->LoadAllItemsAndDlc(FSteamOperation::FOnLoadAllItemsAndDlcComplete::CreateSP(
        this,
        &FOnlineEntitlementsInterfaceRedpointSteam::OnAllItemsAndDlcLoaded,
        UserId.AsShared(),
        Namespace));
    return true;
}

}

#endif