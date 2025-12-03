// Copyright June Rhodes. All Rights Reserved.

#pragma once

#if EOS_HAS_AUTHENTICATION

#include "RedpointEOSBuild/BuildEnvironment.h"

EOS_ENABLE_STRICT_WARNINGS

enum class EOnlineStoreOfferItemType : uint8
{
    Durable = 0,
    Consumable = 1,
    Other = 2,
};

EOS_DISABLE_STRICT_WARNINGS

#endif // #if EOS_HAS_AUTHENTICATION