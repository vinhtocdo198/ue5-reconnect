// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

#include "RedpointStarterCharacter.generated.h"

/**
 * This is a starter character, which is configured with a basic sphere and camera so that other players can be seen in
 * the world.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointStarterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Redpoint EOS")
    TObjectPtr<UStaticMeshComponent> Sphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Redpoint EOS")
    TObjectPtr<UCameraComponent> Camera;

    ARedpointStarterCharacter();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()