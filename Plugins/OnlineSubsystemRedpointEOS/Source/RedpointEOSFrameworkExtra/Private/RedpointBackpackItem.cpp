// Copyright June Rhodes. All Rights Reserved.

#include "RedpointBackpackItem.h"

#include "AbilitySystemInterface.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "LogRedpointEOSFrameworkExtra.h"
#include "Net/UnrealNetwork.h"
#include "RedpointAbilityWithInputIdInterface.h"
#include "RedpointAbilityWithLevelInterface.h"
#include "RedpointBackpackItemAttachmentInterface.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

ARedpointBackpackItem::ARedpointBackpackItem()
{
    // Enable correct replication settings for items.
    this->bReplicates = true;
    this->bNetUseOwnerRelevancy = true;
#if REDPOINT_EOS_UE_5_5_OR_LATER
    this->SetNetUpdateFrequency(100.0f);
#else
    this->NetUpdateFrequency = 100.0f;
#endif

    this->CollisionComp = CreateDefaultSubobject<UCapsuleComponent>(FName("CollisionComponent"));
    this->CollisionComp->InitCapsuleSize(40.0f, 50.0f);
    this->CollisionComp->SetCollisionObjectType(ECC_GameTraceChannel4 /* wut? */);
    this->CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Manually enable when in pickup mode
    this->CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
    this->CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    this->RootComponent = this->CollisionComp;

    this->ItemMesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh1P"));
    this->ItemMesh1P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    this->ItemMesh1P->CastShadow = false;
    this->ItemMesh1P->SetVisibility(false, true);
    this->ItemMesh1P->SetupAttachment(this->CollisionComp);
    this->ItemMesh1P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;

    this->ItemMesh3PickupRelativeLocation = FVector(0.0, -25.0, 0.0);

    this->ItemMesh3P = CreateDefaultSubobject<USkeletalMeshComponent>(FName("WeaponMesh3P"));
    this->ItemMesh3P->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    this->ItemMesh3P->SetupAttachment(this->CollisionComp);
    this->ItemMesh3P->SetRelativeLocation(this->ItemMesh3PickupRelativeLocation);
    this->ItemMesh3P->CastShadow = true;
    this->ItemMesh3P->SetVisibility(true, true);
    this->ItemMesh3P->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPose;
}

void ARedpointBackpackItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME_CONDITION(ARedpointBackpackItem, OwningActor, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ARedpointBackpackItem, WeaponCurrentClipAmmo, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ARedpointBackpackItem, WeaponMaximumClipAmmo, COND_OwnerOnly);
}

void ARedpointBackpackItem::PreReplication(IRepChangedPropertyTracker &ChangedPropertyTracker)
{
    Super::PreReplication(ChangedPropertyTracker);

    REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_BEGIN()

    DOREPLIFETIME_ACTIVE_OVERRIDE(
        ARedpointBackpackItem,
        WeaponCurrentClipAmmo,
        (this->bIsWeapon && IsValid(this->AbilitySystemComponent) &&
         !this->AbilitySystemComponent->HasMatchingGameplayTag(this->WeaponFiringTag)));
    DOREPLIFETIME_ACTIVE_OVERRIDE(ARedpointBackpackItem, WeaponMaximumClipAmmo, this->bIsWeapon);

    REDPOINT_EOS_INTEGER_CONVERSION_PERMIT_END()
}

USkeletalMeshComponent *ARedpointBackpackItem::GetItemMesh1P() const
{
    return ItemMesh1P;
}

USkeletalMeshComponent *ARedpointBackpackItem::GetItemMesh3P() const
{
    return ItemMesh3P;
}

void ARedpointBackpackItem::SetOwningActor(AActor *InOwner)
{
    this->OwningActor = InOwner;
    if (this->OwningActor)
    {
        this->AbilitySystemComponent = nullptr;
        if (this->OwningActor->Implements<UAbilitySystemInterface>())
        {
            IAbilitySystemInterface *AbilitySystem = Cast<IAbilitySystemInterface>(GetOwner());
            if (AbilitySystem != nullptr)
            {
                this->AbilitySystemComponent = AbilitySystem->GetAbilitySystemComponent();
            }
        }

        this->SetOwner(InOwner);

        this->AttachToComponent(
            this->OwningActor->GetRootComponent(),
            FAttachmentTransformRules::KeepRelativeTransform);
    }
    else
    {
        this->AbilitySystemComponent = nullptr;
        this->SetOwner(nullptr);
        this->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    }
}

void ARedpointBackpackItem::Equip(bool &bOutAttachmentDeferred)
{
    bOutAttachmentDeferred = false;

    if (this->OwningActor == nullptr)
    {
        UE_LOG(
            LogRedpointEOSFrameworkExtra,
            Error,
            TEXT("ARedpointBackpackItem::Equip: Called when the OwningActor is nullptr; the item will not equip "
                 "properly."),
            *AActor::GetDebugName(this));
        return;
    }

    bOutAttachmentDeferred = !this->TryCompleteAttachment();
}

void ARedpointBackpackItem::Unequip()
{
    if (this->OwningActor == nullptr)
    {
        return;
    }

    if (this->ItemMesh1P != nullptr)
    {
        this->ItemMesh1P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
        this->ItemMesh1P->SetVisibility(false, true);
    }

    if (this->ItemMesh3P != nullptr)
    {
        this->ItemMesh3P->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
        this->ItemMesh3P->CastShadow = false;
        this->ItemMesh3P->bCastHiddenShadow = false;
        this->ItemMesh3P->SetVisibility(true, true); // Without this, the unequipped weapon's 3p shadow hangs around
        this->ItemMesh3P->SetVisibility(false, true);
    }
}

void ARedpointBackpackItem::AddAbilities()
{
    if (!IsValid(this->OwningActor) || !this->OwningActor->Implements<UAbilitySystemInterface>() ||
        Cast<IAbilitySystemInterface>(this->OwningActor)->GetAbilitySystemComponent() == nullptr)
    {
        return;
    }

    UAbilitySystemComponent *ASC = Cast<IAbilitySystemInterface>(this->OwningActor)->GetAbilitySystemComponent();
    if (ASC == nullptr)
    {
        const TCHAR *NetRoleString = TEXT("Unknown");
        switch (this->OwningActor->GetLocalRole())
        {
        case ENetRole::ROLE_None:
            NetRoleString = TEXT("None");
            break;
        case ENetRole::ROLE_SimulatedProxy:
            NetRoleString = TEXT("SimulatedProxy");
            break;
        case ENetRole::ROLE_AutonomousProxy:
            NetRoleString = TEXT("AutonomousProxy");
            break;
        case ENetRole::ROLE_Authority:
            NetRoleString = TEXT("Authority");
            break;
        case ENetRole::ROLE_MAX:
        default:
            NetRoleString = TEXT("Unknown");
            break;
        }
        UE_LOG(
            LogRedpointEOSFrameworkExtra,
            Error,
            TEXT("%s %s Role: %s ASC is null"),
            *FString(__FUNCTION__),
            *GetName(),
            NetRoleString);
        return;
    }

    // Only add abilities on server.
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    // Spawn the handles based on the abilities configured for this item.
    for (TSubclassOf<UGameplayAbility> &Ability : this->Abilities)
    {
        auto *AbilityCDO = Ability.GetDefaultObject();
        if (!IsValid(AbilityCDO))
        {
            continue;
        }

        int32 AbilityLevel = 0;
        int32 AbilityInputId = INDEX_NONE;
        if (AbilityCDO->Implements<URedpointAbilityWithLevelInterface>())
        {
            AbilityLevel = IRedpointAbilityWithLevelInterface::Execute_GetAbilityLevel(AbilityCDO, this);
        }
        if (AbilityCDO->Implements<URedpointAbilityWithInputIdInterface>())
        {
            AbilityInputId = IRedpointAbilityWithInputIdInterface::Execute_GetAbilityInputId(AbilityCDO, this);
        }

        this->AbilitySpecHandles.Add(
            ASC->GiveAbility(FGameplayAbilitySpec(Ability, AbilityLevel, AbilityInputId, this)));
    }
}

void ARedpointBackpackItem::RemoveAbilities()
{
    if (!IsValid(this->OwningActor) || !this->OwningActor->Implements<UAbilitySystemInterface>() ||
        Cast<IAbilitySystemInterface>(this->OwningActor)->GetAbilitySystemComponent() == nullptr)
    {
        return;
    }

    UAbilitySystemComponent *ASC = Cast<IAbilitySystemInterface>(this->OwningActor)->GetAbilitySystemComponent();
    if (ASC == nullptr)
    {
        return;
    }

    // Only remove abilities on server.
    if (GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    for (FGameplayAbilitySpecHandle &SpecHandle : this->AbilitySpecHandles)
    {
        ASC->ClearAbility(SpecHandle);
    }
    this->AbilitySpecHandles.Empty();
}

bool ARedpointBackpackItem::TryCompleteAttachment()
{
    if (this->OwningActor == nullptr)
    {
        return true;
    }

    if (this->OwningActor->Implements<URedpointBackpackItemAttachmentInterface>())
    {
        if (this->ItemMesh1P != nullptr)
        {
            auto Mesh = IRedpointBackpackItemAttachmentInterface::Execute_GetFirstPersonMesh(this->OwningActor);
            if (!IsValid(Mesh))
            {
                return false;
            }

            this->ItemMesh1P->AttachToComponent(
                Mesh,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                IRedpointBackpackItemAttachmentInterface::Execute_GetAttachmentPoint(this->OwningActor));
            this->ItemMesh1P->SetRelativeLocation(this->ItemMesh1PEquippedRelativeLocation);
            this->ItemMesh1P->SetRelativeRotation(this->ItemMesh1PEquippedRelativeRotation);

            if (IRedpointBackpackItemAttachmentInterface::Execute_IsInFirstPersonPerspective(this->OwningActor))
            {
                this->ItemMesh1P->SetVisibility(true, true);
            }
            else
            {
                this->ItemMesh1P->SetVisibility(false, true);
            }
        }

        if (this->ItemMesh3P != nullptr)
        {
            auto Mesh = IRedpointBackpackItemAttachmentInterface::Execute_GetThirdPersonMesh(this->OwningActor);
            if (!IsValid(Mesh))
            {
                return false;
            }

            this->ItemMesh3P->AttachToComponent(
                Mesh,
                FAttachmentTransformRules::SnapToTargetIncludingScale,
                IRedpointBackpackItemAttachmentInterface::Execute_GetAttachmentPoint(this->OwningActor));
            this->ItemMesh3P->SetRelativeLocation(this->ItemMesh3PEquippedRelativeLocation);
            this->ItemMesh3P->SetRelativeRotation(this->ItemMesh3PEquippedRelativeRotation);
            this->ItemMesh3P->CastShadow = true;
            this->ItemMesh3P->bCastHiddenShadow = true;

            if (IRedpointBackpackItemAttachmentInterface::Execute_IsInFirstPersonPerspective(this->OwningActor))
            {
                this->ItemMesh3P->SetVisibility(true, true); // Without this, the weapon's 3p shadow doesn't show
                this->ItemMesh3P->SetVisibility(false, true);
            }
            else
            {
                this->ItemMesh3P->SetVisibility(true, true);
            }
        }
    }

    return true;
}

void ARedpointBackpackItem::OnRep_WeaponCurrentClipAmmo(int32 OldValue)
{
}

void ARedpointBackpackItem::OnRep_WeaponMaximumClipAmmo(int32 OldValue)
{
}

int32 ARedpointBackpackItem::GetWeaponCurrentClipAmmo() const
{
    return this->WeaponCurrentClipAmmo;
}

int32 ARedpointBackpackItem::GetWeaponMaximumClipAmmo() const
{
    return this->WeaponMaximumClipAmmo;
}

void ARedpointBackpackItem::SetWeaponCurrentClipAmmo(int32 CurrentAmmo)
{
    this->WeaponCurrentClipAmmo = CurrentAmmo;
}

void ARedpointBackpackItem::SetWeaponMaximumClipAmmo(int32 MaximumAmmo)
{
    this->WeaponMaximumClipAmmo = MaximumAmmo;
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()