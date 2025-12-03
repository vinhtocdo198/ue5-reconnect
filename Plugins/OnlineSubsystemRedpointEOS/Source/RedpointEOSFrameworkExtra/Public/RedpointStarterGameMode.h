// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "GameFramework/GameModeBase.h"

#include "RedpointStarterGameMode.generated.h"

/**
 * This is a starter game mode, pre-configured to use the Redpoint EOS starter player controller and starter character,
 * which are pre-configured for really basic multiplayer games.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointStarterGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ARedpointStarterGameMode();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()