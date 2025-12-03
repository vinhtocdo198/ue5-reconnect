// Copyright June Rhodes. All Rights Reserved.

#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineSubsystemRedpointSteam.h"

#include "Misc/ConfigCacheIni.h"
#include "OnlineSubsystemRedpointSteam/Logging.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineAvatarInterfaceRedpointSteam.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineEntitlementsInterfaceRedpointSteam.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlinePurchaseInterfaceRedpointSteam.h"
#include "OnlineSubsystemRedpointSteam/OnlineSubsystem/OnlineStoreInterfaceV2RedpointSteam.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace REDPOINT_EOS_FILE_NS_ID(3906369459, Redpoint::EOS::OnlineSubsystemRedpointSteam::OnlineSubsystem)
{

FOnlineSubsystemRedpointSteam::FOnlineSubsystemRedpointSteam(FName InInstanceName)
    : FOnlineSubsystemImplBase(SubsystemName, InInstanceName)
{
}

FOnlineSubsystemRedpointSteam::~FOnlineSubsystemRedpointSteam()
{
}

bool FOnlineSubsystemRedpointSteam::IsEnabled() const
{
    return true;
}

IOnlineEntitlementsPtr FOnlineSubsystemRedpointSteam::GetEntitlementsInterface() const
{
    return this->EntitlementsImpl;
}

IOnlineStoreV2Ptr FOnlineSubsystemRedpointSteam::GetStoreV2Interface() const
{
    return this->StoreV2Impl;
}

IOnlinePurchasePtr FOnlineSubsystemRedpointSteam::GetPurchaseInterface() const
{
    return this->PurchaseImpl;
}

class UObject *FOnlineSubsystemRedpointSteam::GetNamedInterface(FName InterfaceName)
{
    if (InterfaceName.IsEqual(ONLINE_AVATAR_INTERFACE_NAME))
    {
        return (class UObject *)(void *)&this->AvatarImpl;
    }

    return nullptr;
}

bool FOnlineSubsystemRedpointSteam::Init()
{
    auto Operation = MakeShared<FSteamOperation>();
    this->OperationImpl = Operation;

    this->AvatarImpl = MakeShared<FOnlineAvatarInterfaceRedpointSteam, ESPMode::ThreadSafe>(Operation);
    this->EntitlementsImpl = MakeShared<FOnlineEntitlementsInterfaceRedpointSteam, ESPMode::ThreadSafe>(Operation);
    this->StoreV2Impl = MakeShared<FOnlineStoreInterfaceV2RedpointSteam, ESPMode::ThreadSafe>(Operation);
    this->PurchaseImpl = MakeShared<FOnlinePurchaseInterfaceRedpointSteam, ESPMode::ThreadSafe>(Operation);

    GConfig->GetString(TEXT("OnlineSubsystemSteam"), TEXT("WebApiKey"), WebApiKey, GEngineIni);

    return true;
}

template <typename T, ESPMode Mode> void DestructInterface(TSharedPtr<T, Mode> &Ref, const TCHAR *Name)
{
    if (Ref.IsValid())
    {
        ensureMsgf(
            Ref.IsUnique(),
            TEXT(
                "Interface is not unique during shutdown of Steam subsystem: %s. "
                "This indicates you have a TSharedPtr<> or IOnline... in your code that is holding a reference open to "
                "the interface longer than the lifetime of the online subsystem. You should use TWeakPtr<> "
                "to hold references to interfaces in class fields to prevent lifetime issues"),
            Name);
        Ref = nullptr;
    }
}

bool FOnlineSubsystemRedpointSteam::Shutdown()
{
    DestructInterface(this->PurchaseImpl, TEXT("IOnlinePurchase"));
    DestructInterface(this->StoreV2Impl, TEXT("IOnlineStoreV2"));
    DestructInterface(this->EntitlementsImpl, TEXT("IOnlineEntitlements"));
    DestructInterface(this->AvatarImpl, TEXT("IOnlineAvatar"));

    this->OperationImpl.Reset();

    return true;
}

FString FOnlineSubsystemRedpointSteam::GetAppId() const
{
    return TEXT("");
}

FText FOnlineSubsystemRedpointSteam::GetOnlineServiceName() const
{
    return FText::GetEmpty();
}

FString FOnlineSubsystemRedpointSteam::GetWebApiKey() const
{
    return WebApiKey;
}

}

#endif