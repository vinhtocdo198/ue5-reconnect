// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "RedpointAttributeSetBase.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointAttributeSetDash.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attribute Sets")
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointAttributeSetDash : public URedpointAttributeSetBase
{
    GENERATED_BODY()

public:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    /**
     * The number of seconds the dash will last.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_DashLengthTime,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Dash Length Time (seconds)"))
    FGameplayAttributeData DashLengthTime;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDash, DashLengthTime)

    /**
     * The strength of the dash.
     */
    UPROPERTY(
        BlueprintReadOnly,
        ReplicatedUsing = OnRep_DashStrength,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Attributes",
        Meta = (DisplayName = "Dash Strength (forward velocity)"))
    FGameplayAttributeData DashStrength;
    REDPOINT_FRAMEWORK_ATTRIBUTE_ACCESSORS(URedpointAttributeSetDash, DashStrength)

protected:
    UFUNCTION()
    virtual void OnRep_DashLengthTime(const FGameplayAttributeData &OldValue);

    UFUNCTION()
    virtual void OnRep_DashStrength(const FGameplayAttributeData &OldValue);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()