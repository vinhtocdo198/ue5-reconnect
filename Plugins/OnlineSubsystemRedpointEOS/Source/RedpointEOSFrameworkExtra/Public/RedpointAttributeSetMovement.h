// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetMovement.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetMovement : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The movement speed of the actor.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_MovementSpeed,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Movement Speed"))
    FGameplayAttributeData MovementSpeed;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetMovement, MovementSpeed)

protected:
    UFUNCTION()
    virtual void OnRep_MovementSpeed(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()