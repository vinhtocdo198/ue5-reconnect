// Copyright June Rhodes. All Rights Reserved.

#include "RedpointBackpackComponent.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "LogRedpointEOSFrameworkExtra.h"
#include "Net/UnrealNetwork.h"
#include "RedpointBackpackItemNotificationInterface.h"
#include "TimerManager.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

URedpointBackpackComponent::URedpointBackpackComponent()
{
    this->NoItemTag = FGameplayTag::RequestGameplayTag(FName("Item.Equipped.None"), false);
    this->AbilityCancelOnItemChangeTag = FGameplayTag::RequestGameplayTag(FName("Ability.CancelOnItemChange"), false);

    if ((this->GetFlags() & EObjectFlags::RF_ClassDefaultObject) != 0)
    {
        if (!this->NoItemTag.IsValid())
        {
            UE_LOG(
                LogRedpointEOSFrameworkExtra,
                Warning,
                TEXT("The gameplay tag 'Item.Equipped.None' was not found. This tag is set on ability system "
                     "components when no item is currently selected in a backpack component, instead of the ItemTag of "
                     "the selected item."));
        }
        if (!this->AbilityCancelOnItemChangeTag.IsValid())
        {
            UE_LOG(
                LogRedpointEOSFrameworkExtra,
                Warning,
                TEXT("The gameplay tag 'Ability.CancelOnItemChange' was not found. This tag can be set on abilities to "
                     "ensure they are cancelled when a player changes their current item selection in the backpack "
                     "component."));
        }
    }

    this->bChangedItemLocally = false;

    this->SetIsReplicatedByDefault(true);
    this->bAutoActivate = true;

    this->bAttachmentDeferred = false;
}

void URedpointBackpackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(URedpointBackpackComponent, Items);
    // Only replicate CurrentItem to simulated clients and manually sync CurrentItem with Owner when we're ready.
    // This allows us to predict weapon changing.
    DOREPLIFETIME_CONDITION(URedpointBackpackComponent, CurrentItem, COND_SimulatedOnly);
}

void URedpointBackpackComponent::BeginPlay()
{
    Super::BeginPlay();

    if (this->GetOwner()->GetLocalRole() == ROLE_Authority)
    {
        // Fix up slot count if we have more default items than slots.
        if (this->Loadout.Num() > this->SlotCount)
        {
            this->SlotCount = this->Loadout.Num();
        }

        // Add empty slots.
        for (auto i = 0; i < this->SlotCount; i++)
        {
            this->Items.Add(nullptr);
        }
    }

    UAbilitySystemComponent *ASC = this->GetAbilitySystemComponent();
    if (ASC != nullptr)
    {
        ASC->AbilityFailedCallbacks.AddUObject(this, &URedpointBackpackComponent::OnAbilityActivationFailed);
    }

    this->GetOwner()->GetWorldTimerManager().SetTimerForNextTick(
        this,
        &URedpointBackpackComponent::SpawnDefaultBackpack);
}

ARedpointBackpackItem *URedpointBackpackComponent::GetCurrentItem()
{
    return this->CurrentItem;
}

ARedpointBackpackItem *URedpointBackpackComponent::GetItemInSlot(int InSlot)
{
    if (InSlot < 0 || InSlot > this->SlotCount)
    {
        return nullptr;
    }

    return this->Items[InSlot];
}

bool URedpointBackpackComponent::PickupItem(ARedpointBackpackItem *Item)
{
    if (this->GetOwner()->GetLocalRole() != ROLE_Authority)
    {
        return false;
    }

    // Fill in the empty slot.
    for (auto i = 0; i < this->Items.Num(); i++)
    {
        if (!IsValid(this->Items[i]))
        {
            this->Items[i] = Item;
            this->ItemAddedToBackpack(Item);
            return true;
        }
    }

    // Otherwise, replace the current slot.
    auto CurrentIndex = this->GetItemIndex(this->CurrentItem);
    if (CurrentIndex == -1)
    {
        return false;
    }
    if (this->OnDropItemRequested.IsBound() && this->OnDropItemRequested.Execute(this->CurrentItem))
    {
        this->ItemRemovedFromBackpack(this->CurrentItem);
        this->Items[CurrentIndex] = Item;
        this->ItemAddedToBackpack(Item);
        return true;
    }
    else
    {
        return false;
    }
}

bool URedpointBackpackComponent::DropItem(ARedpointBackpackItem *Item)
{
    if (this->GetOwner()->GetLocalRole() != ROLE_Authority)
    {
        return false;
    }

    auto ItemIndex = this->GetItemIndex(Item);
    if (ItemIndex == -1)
    {
        return false;
    }

    auto NeedsWeaponCycle = ItemIndex == this->GetItemIndex(this->CurrentItem);

    if (this->OnDropItemRequested.IsBound() && this->OnDropItemRequested.Execute(this->CurrentItem))
    {
        this->ItemRemovedFromBackpack(this->Items[ItemIndex]);
        this->Items[ItemIndex] = nullptr;

        if (NeedsWeaponCycle)
        {
            this->SelectNextItem();
        }

        return true;
    }
    else
    {
        return false;
    }
}

void URedpointBackpackComponent::DropAllItems(bool bForce)
{
    if (this->GetOwner()->GetLocalRole() != ROLE_Authority)
    {
        return;
    }

    bool bNeedsWeaponCycle = false;

    for (int ItemIndex = 0; ItemIndex < this->Items.Num(); ItemIndex++)
    {
        if (IsValid(this->Items[ItemIndex]))
        {
            bool bIsCurrentItem = ItemIndex == this->GetItemIndex(this->CurrentItem);
            if ((this->OnDropItemRequested.IsBound() && this->OnDropItemRequested.Execute(this->Items[ItemIndex])) ||
                bForce)
            {
                if (bIsCurrentItem)
                {
                    bNeedsWeaponCycle = true;
                }
                this->ItemRemovedFromBackpack(this->Items[ItemIndex]);
                this->Items[ItemIndex] = nullptr;
            }
        }
    }

    if (bNeedsWeaponCycle)
    {
        if (bForce)
        {
            this->SetCurrentItem(nullptr, this->CurrentItem);
        }
        else
        {
            this->SelectNextItem();
        }
    }
}

void URedpointBackpackComponent::SelectNextItem()
{
    auto CurrentIndex = this->GetItemIndex(this->CurrentItem);
    if (CurrentIndex == -1)
    {
        // Just start from the beginning.
        CurrentIndex = 0;
    }

    for (auto i = 0; i < this->Items.Num(); i++)
    {
        auto RealIndex = (CurrentIndex + i) % this->Items.Num();
        if (IsValid(this->Items[RealIndex]) && this->Items[RealIndex] != this->CurrentItem)
        {
            this->EquipItem(this->Items[RealIndex]);
        }
    }
}

void URedpointBackpackComponent::SelectPreviousItem()
{
    auto CurrentIndex = this->GetItemIndex(this->CurrentItem);
    if (CurrentIndex == -1)
    {
        // Just start from the beginning.
        CurrentIndex = 0;
    }

    for (auto i = 0; i < this->Items.Num(); i++)
    {
        auto RealIndex = (this->Items.Num() + CurrentIndex - i) % this->Items.Num();
        if (IsValid(this->Items[RealIndex]) && this->Items[RealIndex] != this->CurrentItem)
        {
            this->EquipItem(this->Items[RealIndex]);
        }
    }
}

int URedpointBackpackComponent::GetSlotCount()
{
    return this->Items.Num();
}

int URedpointBackpackComponent::GetItemCount()
{
    int Count = 0;
    for (auto i = 0; i < this->Items.Num(); i++)
    {
        if (IsValid(this->Items[i]))
        {
            Count++;
        }
    }

    return Count;
}

int URedpointBackpackComponent::GetItemIndex(ARedpointBackpackItem *Item)
{
    for (auto i = 0; i < this->Items.Num(); i++)
    {
        if (this->Items[i] == Item)
        {
            return i;
        }
    }

    return -1;
}

void URedpointBackpackComponent::SetCurrentItem(ARedpointBackpackItem *NewItem, ARedpointBackpackItem *PreviousItem)
{
    if (NewItem == PreviousItem)
    {
        return;
    }

    UAbilitySystemComponent *AbilitySystemComponent = this->GetAbilitySystemComponent();
    if (AbilitySystemComponent != nullptr)
    {
        // Cancel any abilities that have the 'Ability.CancelOnItemChange' tag.
        FGameplayTagContainer AbilityTagsToCancel = FGameplayTagContainer(AbilityCancelOnItemChangeTag);
        AbilitySystemComponent->CancelAbilities(&AbilityTagsToCancel);
    }

    if (PreviousItem)
    {
        PreviousItem->RemoveAbilities();
    }

    this->UnequipItem(PreviousItem);

    if (NewItem != nullptr)
    {
        if (AbilitySystemComponent != nullptr && this->CurrentItemTag.IsValid())
        {
            AbilitySystemComponent->RemoveLooseGameplayTag(this->CurrentItemTag);
        }

        this->CurrentItem = NewItem;
        this->CurrentItem->SetOwningActor(GetOwner());
        this->bAttachmentDeferred = false;
        this->CurrentItem->Equip(this->bAttachmentDeferred);
        this->CurrentItemTag = this->CurrentItem->ItemTag;

        if (this->bAttachmentDeferred)
        {
            this->GetOwner()->GetWorldTimerManager().SetTimerForNextTick(
                this,
                &URedpointBackpackComponent::TryResolveDeferredAttachment);
        }

        if (AbilitySystemComponent != nullptr && this->CurrentItemTag.IsValid())
        {
            AbilitySystemComponent->AddLooseGameplayTag(this->CurrentItemTag);
        }

        this->CurrentItem->AddAbilities();

        // Notify our owner that the current item has been changed; this can be used to play montages, etc.
        if (GetOwner() != nullptr && GetOwner()->Implements<URedpointBackpackItemNotificationInterface>())
        {
            IRedpointBackpackItemNotificationInterface::Execute_OnItemChanged(GetOwner(), NewItem, PreviousItem);
        }
    }
    else
    {
        if (AbilitySystemComponent != nullptr)
        {
            if (this->CurrentItemTag.IsValid())
            {
                AbilitySystemComponent->RemoveLooseGameplayTag(this->CurrentItemTag);
            }
            if (this->NoItemTag.IsValid())
            {
                this->CurrentItemTag = this->NoItemTag;
                AbilitySystemComponent->AddLooseGameplayTag(this->CurrentItemTag);
            }
        }

        this->UnequipItem(this->CurrentItem);
        this->CurrentItem = nullptr;
    }
}

void URedpointBackpackComponent::OnRep_Items()
{
    if (this->GetOwner()->GetLocalRole() == ROLE_AutonomousProxy && this->Items.Num() > 0 &&
        this->CurrentItem == nullptr)
    {
        // Since we don't replicate the CurrentItem to the owning client, this is a way to ask the Server to sync the
        // CurrentItem after it's been spawned via replication from the Server. The weapon spawning is replicated but
        // the variable CurrentItem is not on the owning client.
        this->SyncCurrentItemToServer();
    }
}

void URedpointBackpackComponent::OnRep_CurrentItem(ARedpointBackpackItem *PreviousItem)
{
    bChangedItemLocally = false;
    this->SetCurrentItem(CurrentItem, PreviousItem);
}

void URedpointBackpackComponent::SpawnDefaultBackpack()
{
    if (this->GetOwner()->GetLocalRole() < ROLE_Authority)
    {
        return;
    }

    auto NeedsSelect = false;
    int32 NumLoadout = this->Loadout.Num();
    for (int32 i = 0; i < NumLoadout; i++)
    {
        if (!Loadout[i].Item)
        {
            // An empty item was added to the Array in blueprint
            continue;
        }

        ARedpointBackpackItem *NewItem = GetWorld()->SpawnActorDeferred<ARedpointBackpackItem>(
            this->Loadout[i].Item,
            FTransform::Identity,
            GetOwner(),
            nullptr,
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
        NewItem->FinishSpawning(FTransform::Identity);

        this->PickupItem(NewItem);
        NeedsSelect = true;
    }

    if (NeedsSelect)
    {
        this->SelectNextItem();
    }
}

void URedpointBackpackComponent::EquipItem(ARedpointBackpackItem *ItemToEquip)
{
    if (this->GetOwner()->GetLocalRole() < ROLE_Authority)
    {
        this->RequestItemEquipOnServer(ItemToEquip);
        this->SetCurrentItem(ItemToEquip, this->CurrentItem);
    }
    else
    {
        this->SetCurrentItem(ItemToEquip, this->CurrentItem);
        this->bChangedItemLocally = true;
    }
}

void URedpointBackpackComponent::RequestItemEquipOnServer_Implementation(ARedpointBackpackItem *NewItem)
{
    this->EquipItem(NewItem);
}

bool URedpointBackpackComponent::RequestItemEquipOnServer_Validate(ARedpointBackpackItem *NewItem)
{
    return true;
}

void URedpointBackpackComponent::UnequipItem(ARedpointBackpackItem *ItemToUnequip)
{
    if (ItemToUnequip)
    {
        ItemToUnequip->Unequip();
    }
}

void URedpointBackpackComponent::ItemAddedToBackpack(ARedpointBackpackItem *Item)
{
    Item->SetOwningActor(this->GetOwner());
}

void URedpointBackpackComponent::ItemRemovedFromBackpack(ARedpointBackpackItem *Item)
{
    Item->SetOwningActor(nullptr);
}

UAbilitySystemComponent *URedpointBackpackComponent::GetAbilitySystemComponent()
{
    if (GetOwner() != nullptr && GetOwner()->Implements<UAbilitySystemInterface>())
    {
        IAbilitySystemInterface *AbilitySystem = Cast<IAbilitySystemInterface>(GetOwner());
        if (AbilitySystem != nullptr)
        {
            return AbilitySystem->GetAbilitySystemComponent();
        }
    }

    return nullptr;
}

void URedpointBackpackComponent::OnAbilityActivationFailed(
    const UGameplayAbility *FailedAbility,
    const FGameplayTagContainer &FailTags)
{
    auto ChangingTag = FGameplayTag::RequestGameplayTag(FName("Ability.Item.IsChanging"), false);

#if REDPOINT_EOS_UE_5_5_OR_LATER
    if (FailedAbility && ChangingTag.IsValid() && FailedAbility->GetAssetTags().HasTagExact(ChangingTag))
#else
    if (FailedAbility && ChangingTag.IsValid() && FailedAbility->AbilityTags.HasTagExact(ChangingTag))
#endif
    {
        if (this->bChangedItemLocally)
        {
            this->SyncCurrentItemToServer();
        }
    }
}

void URedpointBackpackComponent::SyncCurrentItemToServer_Implementation()
{
    this->SyncCurrentItemToClient(this->CurrentItem);
}

bool URedpointBackpackComponent::SyncCurrentItemToServer_Validate()
{
    return true;
}

void URedpointBackpackComponent::SyncCurrentItemToClient_Implementation(ARedpointBackpackItem *InItem)
{
    ARedpointBackpackItem *LastItem = this->CurrentItem;
    this->CurrentItem = InItem;
    OnRep_CurrentItem(LastItem);
}

bool URedpointBackpackComponent::SyncCurrentItemToClient_Validate(ARedpointBackpackItem *InItem)
{
    return true;
}

void URedpointBackpackComponent::TryResolveDeferredAttachment()
{
    if (!this->bAttachmentDeferred)
    {
        return;
    }

    if (!IsValid(this->CurrentItem))
    {
        this->bAttachmentDeferred = false;
        return;
    }

    if (this->CurrentItem->TryCompleteAttachment())
    {
        this->bAttachmentDeferred = false;
        return;
    }
    else
    {
        // We need to attempt on the next frame.
        this->GetOwner()->GetWorldTimerManager().SetTimerForNextTick(
            this,
            &URedpointBackpackComponent::TryResolveDeferredAttachment);
    }
}

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()