// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointEOSBuild/BuildEnvironment.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"

#include "RedpointStarterPlayerController.generated.h"

/**
 * This is a starter player controller, which is configured to use Enhanced Input actions to implement basic WASD
 * third-person controller. It expects the controlled pawn to either be or inherit from ACharacter. It's recommended to
 * use ARedpointEOSStarterCharacter, as this comes pre-configured with a sphere and camera suitable for debugging
 * multiplayer games.
 */
UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointStarterPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    /**
     * The Enhanced Input action that will be used for movement input. Defaults to URedpointStarterMoveAction.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Redpoint EOS")
    TSoftObjectPtr<UInputAction> MoveAction;

    /**
     * The Enhanced Input action that will be used for movement input. Defaults to URedpointStarterJumpAction.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Redpoint EOS")
    TSoftObjectPtr<UInputAction> JumpAction;

    /**
     * The Enhanced Input mapping context that will map inputs to the above actions. Defaults to
     * URedpointStarterInputMappingContext.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Redpoint EOS")
    TSoftObjectPtr<UInputMappingContext> InputMappingContext;

    /**
     * If set, the InputMappingContext will not be registered automatically on BeginPlay. Your derived implementation of
     * this class must register an input mapping context for the MoveAction and JumpAction during BeginPlay, or the
     * controlled pawn will not respond to inputs.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Redpoint EOS")
    bool bSkipInputMappingContext;

    ARedpointStarterPlayerController();

    virtual void BeginPlay() override;

    virtual void SetupInputComponent() override;

private:
    void OnMoveAction(const FInputActionValue &);

    void OnJumpAction();
};

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()