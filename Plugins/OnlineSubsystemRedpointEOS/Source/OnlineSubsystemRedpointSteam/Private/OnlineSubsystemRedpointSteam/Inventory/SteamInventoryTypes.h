// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Map.h"
#include "Containers/UnrealString.h"
#include "Interfaces/OnlineEntitlementsInterface.h"
#include "Interfaces/OnlineStoreInterfaceV2.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#if defined(REDPOINT_EOS_STEAM_ENABLED)

namespace Redpoint::EOS::OnlineSubsystemRedpointSteam::Inventory
{

typedef TMap<FUniqueOfferId, TSharedPtr<FOnlineStoreOffer>> FSteamOfferMap;
typedef TMap<FUniqueEntitlementId, TSharedPtr<FOnlineEntitlement>> FSteamEntitlementMap;

struct FSteamItemDefinition
{
public:
    int32 SteamItemDefinitionId;
    uint64 CurrentPrice;
    uint64 BasePrice;
    TMap<FString, FString> Properties;
};

typedef TMap<int32, FSteamItemDefinition> FSteamItemDefinitionMap;

struct FSteamItemRaw
{
public:
    uint64 SteamItemId;
    int32 SteamItemDefinitionId;
    uint16 Quantity;
    uint16 Flags;
};

struct FSteamItemDefinitionPriceRaw
{
public:
    int32 SteamItemDefinitionId;
    uint64 CurrentPrice;
    uint64 BasePrice;
};

}

#endif