// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Containers/Array.h"
#include "Interfaces/OnlineStatsInterface.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

ONLINESUBSYSTEMREDPOINTEOS_API TArray<uint8> SerializeStats(const FOnlineStatsUserUpdatedStats &Stats);
ONLINESUBSYSTEMREDPOINTEOS_API FOnlineStatsUserUpdatedStats DeserializeStats(const TArray<uint8> &Stats);