// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Interfaces/OnlineEntitlementsInterface.h"
#include "OnlineSubsystemRedpointEOS/OnlineSubsystem/Identity/OnlineIdentityInterfaceEOS.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

EOS_ENABLE_STRICT_WARNINGS

class ONLINESUBSYSTEMREDPOINTEOS_API FOnlineEntitlementsInterfaceSynthetic
    : public IOnlineEntitlements,
      public TSharedFromThis<FOnlineEntitlementsInterfaceSynthetic, ESPMode::ThreadSafe>
{
private:
    Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSRef IdentityEOS;
    TMap<FString, FDelegateHandle> EventHandlers;

    void OnQueryEntitlementsComplete(
        bool bWasSuccessful,
        const FUniqueNetId &UserId,
        const FString &Namespace,
        const FString &Error,
        FString RequestedNamespace,
        TSharedRef<const FUniqueNetId> RequestedUserId,
        TSharedRef<const FUniqueNetId> OriginalNativeUserId);

public:
    FOnlineEntitlementsInterfaceSynthetic(
        Redpoint::EOS::OnlineSubsystemRedpointEOS::OnlineSubsystem::Identity::FOnlineIdentityInterfaceEOSRef
            InIdentityEOS)
        : IdentityEOS(MoveTemp(InIdentityEOS))
        , EventHandlers(){};
    UE_NONCOPYABLE(FOnlineEntitlementsInterfaceSynthetic);
    virtual ~FOnlineEntitlementsInterfaceSynthetic() override = default;

    virtual TSharedPtr<FOnlineEntitlement> GetEntitlement(
        const FUniqueNetId &UserId,
        const FUniqueEntitlementId &EntitlementId) override;
    virtual TSharedPtr<FOnlineEntitlement> GetItemEntitlement(const FUniqueNetId &UserId, const FString &ItemId)
        override;
    virtual void GetAllEntitlements(
        const FUniqueNetId &UserId,
        const FString &Namespace,
        TArray<TSharedRef<FOnlineEntitlement>> &OutUserEntitlements) override;
    virtual bool QueryEntitlements(
        const FUniqueNetId &UserId,
        const FString &Namespace,
        const FPagedQuery &Page = FPagedQuery()) override;
};

EOS_DISABLE_STRICT_WARNINGS