// Copyright June Rhodes. All Rights Reserved.

#include "RedpointStarterGameMode.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

#include "RedpointStarterCharacter.h"
#include "RedpointStarterPlayerController.h"

ARedpointStarterGameMode::ARedpointStarterGameMode()
{
    this->PlayerControllerClass = ARedpointStarterPlayerController::StaticClass();
    this->DefaultPawnClass = ARedpointStarterCharacter::StaticClass();
    this->bUseSeamlessTravel = true;
}

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()