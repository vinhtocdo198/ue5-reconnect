// Copyright June Rhodes. All Rights Reserved.

#include "RedpointStarterPlayerController.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "RedpointStarterInputMappingContext.h"
#include "RedpointStarterJumpAction.h"
#include "RedpointStarterMoveAction.h"

ARedpointStarterPlayerController::ARedpointStarterPlayerController()
{
    this->MoveAction = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterMoveAction>());
    this->JumpAction = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterJumpAction>());
    this->InputMappingContext = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterInputMappingContext>());
    this->bSkipInputMappingContext = false;
}

void ARedpointStarterPlayerController::BeginPlay()
{
    if (!this->MoveAction.IsValid())
    {
        this->MoveAction = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterMoveAction>());
    }
    if (!this->JumpAction.IsValid())
    {
        this->JumpAction = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterJumpAction>());
    }
    if (!this->InputMappingContext.IsValid())
    {
        this->InputMappingContext = MakeSoftObjectPtr(GetMutableDefault<URedpointStarterInputMappingContext>());
    }
}

void ARedpointStarterPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (!this->bSkipInputMappingContext)
    {
        if (auto *InputSystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
        {
            InputSystem->AddMappingContext(this->InputMappingContext.Get(), 0);
        }

        if (auto *Input = CastChecked<UEnhancedInputComponent>(InputComponent))
        {
            Input->BindAction(
                this->MoveAction.Get(),
                ETriggerEvent::Triggered,
                this,
                &ARedpointStarterPlayerController::OnMoveAction);
            Input->BindAction(
                this->JumpAction.Get(),
                ETriggerEvent::Triggered,
                this,
                &ARedpointStarterPlayerController::OnJumpAction);
        }
    }
}

void ARedpointStarterPlayerController::OnMoveAction(const FInputActionValue &ActionValue)
{
    auto Axis2D = ActionValue.Get<FVector2D>();
    auto WorldDirection = this->GetControlRotation().RotateVector(FVector(Axis2D.X, Axis2D.Y, 0.0));
    if (APawn *ControlledPawn = this->GetPawn())
    {
        ControlledPawn->AddMovementInput(WorldDirection, 50.0f);
    }
}

void ARedpointStarterPlayerController::OnJumpAction()
{
    if (auto *ControlledCharacter = Cast<ACharacter>(this->GetPawn()))
    {
        ControlledCharacter->Jump();
    }
}

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()