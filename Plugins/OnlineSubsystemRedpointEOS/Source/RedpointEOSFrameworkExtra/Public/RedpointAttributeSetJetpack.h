// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetJetpack.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetJetpack : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The number of seconds the jetpack will last.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_JetpackLengthTime,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Jetpack Length Time (seconds)"))
    FGameplayAttributeData JetpackLengthTime;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetJetpack, JetpackLengthTime)

    /**
     * The strength of the jetpack.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_JetpackStrength,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Jetpack Strength (upward velocity)"))
    FGameplayAttributeData JetpackStrength;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetJetpack, JetpackStrength)

protected:
    UFUNCTION()
    virtual void OnRep_JetpackLengthTime(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_JetpackStrength(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()