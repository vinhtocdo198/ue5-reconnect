// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Delegates/Delegate.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "OnlineError.h"
#include "OnlineSubsystemRedpointSteam/Operation/SteamOperation.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Templates/SharedPointer.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Tasks
{

typedef TDelegate<void(const FOnlineError &Error, const Inventory::FSteamItemDefinitionMap &ItemDefinitions)>
    FSteamLoadItemDefinitionsComplete;

typedef TDelegate<void(const FOnlineError &Error, const TArray<Inventory::FSteamItemRaw> &RawItems)>
    FSteamGetAllItemsComplete;

typedef TDelegate<void(
    const FOnlineError &Error,
    const FString &CurrencyCode,
    const TArray<Inventory::FSteamItemDefinitionPriceRaw> &RawItemDefinitionPrices)>
    FSteamRequestPricesComplete;

typedef TDelegate<void(const FOnlineError &, const Inventory::FSteamOfferMap &)> FSteamOffersFetched;

typedef TDelegate<void(const FOnlineError &, const Inventory::FSteamEntitlementMap &)> FSteamEntitlementsFetched;

typedef TDelegate<void(const FOnlineError &)> FSteamUserInformationFetched;

typedef TDelegate<void(const FOnlineError &, uint32 Width, uint32 Height, const TArray<uint8> &RGBABuffer)>
    FSteamAvatarDataFetched;

}

#endif