// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetInteract.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetInteract : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The maximum distance this actor can interact with other actors at.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_InteractDistance,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Maximum Interact Distance"))
    FGameplayAttributeData InteractDistance;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetInteract, InteractDistance)

protected:
    UFUNCTION()
    virtual void OnRep_InteractDistance(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()