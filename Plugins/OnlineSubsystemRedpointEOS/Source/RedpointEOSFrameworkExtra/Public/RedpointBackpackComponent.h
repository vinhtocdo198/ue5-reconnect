// Copyright June Rhodes. All Rights Reserved.

#pragma once

#include "Components/ActorComponent.h"
#include "Delegates/DelegateCombinations.h"
#include "RedpointBackpackItem.h"
#include "RedpointBackpackLoadoutEntry.h"
#include "RedpointEOSBuild/BuildEnvironment.h"

#include "RedpointBackpackComponent.generated.h"

REDPOINT_EOS_CODE_GUARD_BEGIN_UOBJECT()

/**
 * The event delegate used when a backpack wants to drop an item.
 */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(bool, FRedpointBackpackOnDropItemRequested, ARedpointBackpackItem *, Item);

/**
 * A component which manages a "backpack" of items that a character or actor is holding. Keeps track of the current item
 * selected, replicating it to the game server in multiplayer games and applying the necessary gameplay abilities and
 * tags when the item is active.
 *
 * In some games this is referred to as an "inventory". However, some services refer to the permanent collection of
 * items on a player's account as "inventory" (e.g. the Steam Inventory service). To ensure consistent terminology,
 * we refer to the items held by a player during a game to be their "backpack" and the collection of items they have in
 * their account as their "inventory".
 */
UCLASS(
    BlueprintType,
    Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack",
    HideCategories = (LOD, Lighting, Transform, Sockets, TextureStreaming),
    Meta = (BlueprintSpawnableComponent))
class REDPOINTEOSFRAMEWORKEXTRA_API URedpointBackpackComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    URedpointBackpackComponent();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

    virtual void BeginPlay() override;

    FGameplayTag CurrentItemTag;

    bool bAttachmentDeferred;

    /**
     * The current item this actor has selected. This can only be None if the actor has no items in their backpack,
     * otherwise the backpack will always have an item selected.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    ARedpointBackpackItem *GetCurrentItem();

    /**
     * Returns the item in this backpack in the specified slot (index). This will be None if there is no item in that
     * slot.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    ARedpointBackpackItem *GetItemInSlot(int InSlot);

    /**
     * Called on the game server when a player picks up an item that is currently spawned in the world. Backpack items
     * are actors which render their associated mesh only when they're not in an inventory, which allows a backpack item
     * to maintain state across being picked up and dropped by a player.
     *
     * Returns true if the item was successfully picked up. In order for an item to be successfully picked up, one of
     * the following scenarios must be true:
     *
     * - There must be an empty slot in the backpack, or
     * - The "OnDropItemRequested" event is bound and returns true, allowing the currently selected item to be dropped.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    bool PickupItem(ARedpointBackpackItem *Item);

    /**
     * Called on the game server when a player drops an item currently in their backpack. The provided item must be in
     * the backpack (typically retrieved via GetItemInSlot).
     *
     * For items to be dropped, OnDropItemRequested must be bound and return true.
     *
     * If the current item is dropped, the backpack automatically switches to the next item.
     *
     * Returns true if the item was dropped successfully.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    bool DropItem(ARedpointBackpackItem *Item = nullptr);

    /**
     * Called on the game server when a player should drop all items in their backpack.
     *
     * For items to be dropped, OnDropItemRequested must be bound and return true.
     *
     * After this executes, the player will have no current item.
     */
    UFUNCTION(
        BlueprintCallable,
        BlueprintAuthorityOnly,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    void DropAllItems(bool bForce);

    /**
     * Switch to the next item in the backpack. This should be called on the client so that the client can immediately
     * predict the item switch. The backpack component will automatically handle replicating the item switch to the
     * server, and performing any necessary unwind if the client's state is inconsistent with the server.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    virtual void SelectNextItem();

    /**
     * Switch to the previous item in the backpack. This should be called on the client so that the client can
     * immediately predict the item switch. The backpack component will automatically handle replicating the item switch
     * to the server, and performing any necessary unwind if the client's state is inconsistent with the server.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    virtual void SelectPreviousItem();

    /**
     * Returns the total number of slots in this backpack.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    int GetSlotCount();

    /**
     * Returns the number of filled slots in this backpack.
     */
    UFUNCTION(BlueprintCallable, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    int GetItemCount();

    /*
     * The loadout is the list of items that this backpack will start with when it is first created. Each of the entries
     * here will result in the item being spawned and immediately placed in the backpack.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TArray<FRedpointBackpackLoadoutEntry> Loadout;

    /**
     * The number of slots this backpack will have.
     */
    UPROPERTY(EditDefaultsOnly, Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    int SlotCount;

    /**
     * The read-only list of items that are currently in this backpack.
     */
    UPROPERTY(
        VisibleAnywhere,
        ReplicatedUsing = OnRep_Items,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TArray<TObjectPtr<ARedpointBackpackItem>> Items;

    /**
     * The read-only item that is currently selected item in this backpack.
     */
    UPROPERTY(
        VisibleAnywhere,
        ReplicatedUsing = OnRep_CurrentItem,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    TObjectPtr<ARedpointBackpackItem> CurrentItem;

    /**
     * The event delegate that is fired when this backpack wants to drop an item, either in response to an item being
     * picked up with no empty space, or DropItem being called.
     *
     * Upon this event firing, you will typically reset the position of the item to the current player's position, apply
     * any necessary motion (for drop effects) and return true to indicate that the item was dropped.
     *
     * If you want to prevent an item being dropped, you can return false from the event.
     */
    UPROPERTY(
        BlueprintReadWrite,
        VisibleAnywhere,
        Category = "Redpoint EOS Online Framework|Gameplay Ability System|Backpack")
    FRedpointBackpackOnDropItemRequested OnDropItemRequested;

protected:
    UFUNCTION()
    int GetItemIndex(ARedpointBackpackItem *Item);

    void SetCurrentItem(ARedpointBackpackItem *NewItem, ARedpointBackpackItem *PreviousItem);

    UFUNCTION()
    void OnRep_Items();

    UFUNCTION()
    void OnRep_CurrentItem(ARedpointBackpackItem *PreviousItem);

    /**
     * Set to true when we change the item predictively and flip it to false when the Server replicates to confirm. We
     * use this if the Server refused a item change ability's activation to ask the Server to sync the client back up
     * with the correct CurrentItem.
     */
    bool bChangedItemLocally;

private:
    FGameplayTag NoItemTag;
    FGameplayTag AbilityCancelOnItemChangeTag;

    // Server spawns default backpack
    void SpawnDefaultBackpack();

    // Try to perform deferred attachment.
    void TryResolveDeferredAttachment();

    void EquipItem(ARedpointBackpackItem *ItemToEquip);

    UFUNCTION(Server, Reliable)
    void RequestItemEquipOnServer(ARedpointBackpackItem *NewItem);
    void RequestItemEquipOnServer_Implementation(ARedpointBackpackItem *NewItem);
    bool RequestItemEquipOnServer_Validate(ARedpointBackpackItem *NewItem);

    void UnequipItem(ARedpointBackpackItem *ItemToUnequip);

    void ItemAddedToBackpack(ARedpointBackpackItem *Item);

    void ItemRemovedFromBackpack(ARedpointBackpackItem *Item);

    UAbilitySystemComponent *GetAbilitySystemComponent();

    void OnAbilityActivationFailed(const UGameplayAbility *FailedAbility, const FGameplayTagContainer &FailTags);

    /**
     * The CurrentItem is only automatically replicated to simulated clients. The autonomous client can use this to
     * request the proper CurrentItem from the server when it knows it may be out of sync with it from predictive
     * client-side changes.
     */
    UFUNCTION(Server, Reliable)
    void SyncCurrentItemToServer();
    void SyncCurrentItemToServer_Implementation();
    bool SyncCurrentItemToServer_Validate();

    /**
     * The CurrentItem is only automatically replicated to simulated clients. Use this function to manually sync the
     * autonomous client's CurrentItem when we're ready to. This allows us to predict item changes (changing items
     * fast multiple times in a row so that the server doesn't replicate and clobber our CurrentItem).
     */
    UFUNCTION(Client, Reliable)
    void SyncCurrentItemToClient(ARedpointBackpackItem *InItem);
    void SyncCurrentItemToClient_Implementation(ARedpointBackpackItem *InItem);
    bool SyncCurrentItemToClient_Validate(ARedpointBackpackItem *InItem);
};

REDPOINT_EOS_CODE_GUARD_END_UOBJECT()