// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/SkeletalMeshComponent.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "UObject/Interface.h"
#include "UObject/ObjectMacros.h"

#include "RedpointBackpackItemAttachmentInterface.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UINTERFACE(MinimalAPI, Blueprintable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
class URedpointBackpackItemAttachmentInterface : public UInterface
{
    GENERATED_BODY()
};

class REDPOINTEOSFRAMEWORKEXTRA_API IRedpointBackpackItemAttachmentInterface
{
    GENERATED_BODY()

public:
    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Get Attachment Point"))
    FName GetAttachmentPoint();

    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Is in First Person Perspective?"))
    bool IsInFirstPersonPerspective();

    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Get First Person Mesh"))
    USkeletalMeshComponent *GetFirstPersonMesh();

    UFUNCTION(
        BlueprintCallable,
        BlueprintNativeEvent,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        Meta = (DisplayName = "Get Third Person Mesh"))
    USkeletalMeshComponent *GetThirdPersonMesh();
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()