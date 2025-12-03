// Copyright June Rhodes. All Rights Reserved.

#include "RedpointStarterCharacter.h"

#include "Components/CapsuleComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_BEGIN()

ARedpointStarterCharacter::ARedpointStarterCharacter()
{
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> BasicShapeMaterial(
        TEXT("/Script/Engine.Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
        TEXT("/Script/Engine.StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));

    this->GetCapsuleComponent()->SetCapsuleHalfHeight(50.f);
    this->GetCapsuleComponent()->SetCapsuleRadius(50.f);

    this->Sphere = this->CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Sphere"));
    this->Sphere->SetStaticMesh(SphereMesh.Object);
    this->Sphere->SetMaterial(0, BasicShapeMaterial.Object);
    this->Sphere->SetupAttachment(this->RootComponent);

    this->Camera = this->CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    this->Camera->SetRelativeLocation(FVector(-575.0, 0.0, 430.0));
    this->Camera->SetRelativeRotation(FRotator(-30.0, 0.0, 0.0));
    this->Camera->SetupAttachment(this->Sphere);
}

REDPOINT_EOS_FLOAT_CONVERSION_PERMIT_END()