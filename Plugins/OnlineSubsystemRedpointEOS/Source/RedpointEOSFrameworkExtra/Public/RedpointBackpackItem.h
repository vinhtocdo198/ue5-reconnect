// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Abilities/GameplayAbility.h"
#include "AbilitySystemComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "RedpointEOSBuild/BuildEnvironment.h"
#include "Sound/SoundCue.h"

#include "RedpointBackpackItem.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

UCLASS(BlueprintType)
class REDPOINTEOSFRAMEWORKEXTRA_API ARedpointBackpackItem : public AActor
{
    GENERATED_BODY()

public:
    ARedpointBackpackItem();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    virtual void PreReplication(IRepChangedPropertyTracker &ChangedPropertyTracker) override;

    /**
     * This tag is primarily used by the first person animation blueprint to determine which animations to play.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    FGameplayTag ItemTag;

    /**
     * The icon to display on the UI for the item.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        meta = (DisplayName = "User Interface Icon"))
    TObjectPtr<UTexture2D> Icon;

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|First-Person View",
        meta = (DisplayName = "Get Skeletal Mesh Component for First-Person View"))
    virtual USkeletalMeshComponent *GetItemMesh1P() const;

    UFUNCTION(
        BlueprintCallable,
        BlueprintPure,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Get Skeletal Mesh Component for Third-Person View"))
    virtual USkeletalMeshComponent *GetItemMesh3P() const;

    void SetOwningActor(AActor *InOwner);

    virtual void Equip(bool &bOutAttachmentDeferred);

    virtual void Unequip();

    virtual void AddAbilities();

    virtual void RemoveAbilities();

    /**
     * When items are initially replicated to clients, the mesh components for first and third-person attachment may not
     * yet be available. In this scenario, Equip sets bOutAttachmentDeferred and the backpack component calls this
     * function on subsequent frames to try and complete mesh attachment.
     */
    bool TryCompleteAttachment();

protected:
    UPROPERTY()
    TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

    // Collision capsule for when weapon is in pickup mode
    UPROPERTY(
        VisibleAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        meta = (DisplayName = "Collision Component"))
    TObjectPtr<UCapsuleComponent> CollisionComp;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|First-Person View",
        meta = (DisplayName = "Skeletal Mesh Component for First-Person View"))
    TObjectPtr<USkeletalMeshComponent> ItemMesh1P;

    UPROPERTY(
        VisibleAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Skeletal Mesh Component for Third-Person View"))
    TObjectPtr<USkeletalMeshComponent> ItemMesh3P;

    /**
     * Relative Location of weapon 3P Mesh when in pickup mode. 1P weapon mesh is invisible so it doesn't need one.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Skeletal Mesh Relative Pickup Location for Third-Person View"))
    FVector ItemMesh3PickupRelativeLocation;

    /**
     * Relative Location of weapon 1P Mesh when equipped.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|First-Person View",
        meta = (DisplayName = "Skeletal Mesh Relative Equipped Location for First-Person View"))
    FVector ItemMesh1PEquippedRelativeLocation;

    /**
     * Relative rotation of weapon 1P Mesh when equipped.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|First-Person View",
        meta = (DisplayName = "Skeletal Mesh Relative Equipped Rotation for First-Person View"))
    FRotator ItemMesh1PEquippedRelativeRotation;

    /**
     * Relative Location of weapon 3P Mesh when equipped.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Skeletal Mesh Relative Equipped Location for Third-Person View"))
    FVector ItemMesh3PEquippedRelativeLocation;

    /**
     * Relative rotation of weapon 3P Mesh when equipped.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Skeletal Mesh Relative Equipped Rotation for Third-Person View"))
    FRotator ItemMesh3PEquippedRelativeRotation;

    UPROPERTY(
        BlueprintReadOnly,
        Replicated,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TObjectPtr<AActor> OwningActor;

    /**
     * The abilities that this item grants when in an backpack.
     */
    UPROPERTY(
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        meta = (DisplayName = "Gameplay Abilities Granted by Item"))
    TArray<TSubclassOf<UGameplayAbility>> Abilities;

    /**
     * Handles for the abilities.
     */
    UPROPERTY(BlueprintReadOnly, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TArray<FGameplayAbilitySpecHandle> AbilitySpecHandles;

    /**
     * The animation that plays when this item is equipped in first person view.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|First-Person View",
        meta = (DisplayName = "Equip Montage to Play in First-Person View"))
    TObjectPtr<UAnimMontage> Equip1PMontage;

    /**
     * The animation that plays when this item is equipped in third person view.
     */
    UPROPERTY(
        BlueprintReadonly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Third-Person View",
        meta = (DisplayName = "Equip Montage to Play in Third-Person View"))
    TObjectPtr<UAnimMontage> Equip3PMontage;

    /**
     * The sound played when this item is picked up.
     */
    UPROPERTY(
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
        meta = (DisplayName = "Sound to Play when Picked Up"))
    TObjectPtr<USoundCue> PickupSound;

public:
    /**
     * If true, this item represents a weapon with ammo. Enabling this option causes ammo information to replicate.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons")
    bool bIsWeapon;

    /**
     * This tag will be checked to determine if the weapon is firing. When the weapon is being fired by the local
     * player, we turn off replication to prevent the server from clobbering client-side prediction.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditDefaultsOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons",
        meta = (EditCondition = "bIsWeapon"))
    FGameplayTag WeaponFiringTag;

    /**
     * The icon to display on the UI for the ammo clip.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons",
        meta = (EditCondition = "bIsWeapon"))
    TObjectPtr<UTexture2D> WeaponClipIcon;

    /**
     * The ammo currently in the clip of this weapon.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        ReplicatedUsing = OnRep_WeaponCurrentClipAmmo,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons",
        meta = (EditCondition = "bIsWeapon"))
    int32 WeaponCurrentClipAmmo;

    /**
     * The maximum amount of ammo that can be held in the clip of this weapon.
     */
    UPROPERTY(
        BlueprintReadOnly,
        EditAnywhere,
        ReplicatedUsing = OnRep_WeaponMaximumClipAmmo,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons",
        meta = (EditCondition = "bIsWeapon"))
    int32 WeaponMaximumClipAmmo;

    UFUNCTION()
    virtual void OnRep_WeaponCurrentClipAmmo(int32 OldValue);

    UFUNCTION()
    virtual void OnRep_WeaponMaximumClipAmmo(int32 OldValue);

    /**
     * Returns the current ammo in the weapon's clip.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons")
    int32 GetWeaponCurrentClipAmmo() const;

    /**
     * Returns the maximum amount of ammo this weapon can hold in it's clip.
     */
    UFUNCTION(BlueprintPure, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons")
    int32 GetWeaponMaximumClipAmmo() const;

    /**
     * Set the current ammo in the weapon's clip.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons")
    void SetWeaponCurrentClipAmmo(int32 CurrentAmmo);

    /**
     * Set the maximum amount of ammo this weapon can hold in it's clip.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack|Weapons")
    void SetWeaponMaximumClipAmmo(int32 MaximumAmmo);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()