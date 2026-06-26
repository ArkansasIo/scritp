#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/MMOItemBase.h"
#include "Items/MMOItemContainer.h"
#include "Items/MMOEquipmentItem.h"
#include "MMOInventoryComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnEquipmentChanged, EMMOEquipmentSlot, Slot, const FItemInstance&, Item, bool, bEquipped);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCurrencyChanged, int32, NewGold, int32, NewSilver);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWeightChanged);

class UMMOItemContainer;

UCLASS(ClassGroup=(MMO), meta=(BlueprintSpawnableComponent))
class MMOINVENTORY_API UMMOInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOInventoryComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	UMMOItemContainer* InventoryContainer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	UMMOItemContainer* BankContainer;

	UPROPERTY(ReplicatedUsing = OnRep_Equipment, EditAnywhere, BlueprintReadOnly, Category = "Equipment")
	TMap<EMMOEquipmentSlot, FItemInstance> EquipmentSlots;

	UPROPERTY(ReplicatedUsing = OnRep_Currency, EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int32 Gold;

	UPROPERTY(ReplicatedUsing = OnRep_Currency, EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int32 Silver;

	UPROPERTY(ReplicatedUsing = OnRep_Currency, EditAnywhere, BlueprintReadWrite, Category = "Currency")
	int32 Copper;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 MaxInventorySlots;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	float MaxInventoryWeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Inventory")
	int32 MaxBankSlots;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnCurrencyChanged OnCurrencyChanged;

	UPROPERTY(BlueprintAssignable, Category = "Inventory|Events")
	FOnWeightChanged OnWeightChanged;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool AddItem(const FItemInstance& Item, bool bToBank = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool RemoveItem(int32 SlotIndex, int32 Quantity = 1, bool bFromBank = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool MoveItem(int32 FromIndex, int32 ToIndex, bool bFromBank = false, bool bToBank = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool UseItem(int32 SlotIndex, bool bFromBank = false);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	bool SplitItem(int32 SlotIndex, int32 SplitQuantity, int32 TargetIndex, bool bFromBank = false);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool EquipItem(int32 SlotIndex, bool bFromBank = false);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UnequipItem(EMMOEquipmentSlot Slot);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	FItemInstance GetEquippedItem(EMMOEquipmentSlot Slot) const;

	UFUNCTION(BlueprintCallable, Category = "Currency")
	void AddCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool RemoveCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount);

	UFUNCTION(BlueprintCallable, Category = "Currency")
	bool HasEnoughCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount) const;

	UFUNCTION(BlueprintCallable, Category = "Weight")
	float GetTotalWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Weight")
	float GetRemainingWeight() const;

	UFUNCTION(BlueprintCallable, Category = "Weight")
	bool WouldExceedWeight(const FItemInstance& Item) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 FindItemInInventory(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	int32 GetItemCount(FName ItemId) const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void AutoSort();

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void OpenLootContainer(UMMOItemContainer* LootContainer);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void LootItem(int32 ContainerSlotIndex, UMMOItemContainer* LootContainer);

	UFUNCTION(BlueprintCallable, Category = "Loot")
	void LootAll(UMMOItemContainer* LootContainer);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|RPC")
	void Server_AddItem(const FItemInstance& Item, bool bToBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|RPC")
	void Server_RemoveItem(int32 SlotIndex, int32 Quantity, bool bFromBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|RPC")
	void Server_MoveItem(int32 FromIndex, int32 ToIndex, bool bFromBank, bool bToBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|RPC")
	void Server_UseItem(int32 SlotIndex, bool bFromBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Inventory|RPC")
	void Server_SplitItem(int32 SlotIndex, int32 SplitQuantity, int32 TargetIndex, bool bFromBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Equipment|RPC")
	void Server_EquipItem(int32 SlotIndex, bool bFromBank);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Equipment|RPC")
	void Server_UnequipItem(EMMOEquipmentSlot Slot);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Currency|RPC")
	void Server_AddCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount);

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category = "Currency|RPC")
	void Server_RemoveCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Inventory|RPC")
	void Client_UpdateInventoryItem(int32 SlotIndex, const FItemInstance& Item);

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Inventory|RPC")
	void Client_RefreshInventory();

protected:
	UFUNCTION()
	void OnRep_Equipment();

	UFUNCTION()
	void OnRep_Currency();

	void AttachEquipmentMesh(const FItemInstance& Item, EMMOEquipmentSlot Slot);
	void DetachEquipmentMesh(EMMOEquipmentSlot Slot);

	UPROPERTY(Replicated)
	TArray<FItemInstance> ReplicatedInventoryItems;

	UPROPERTY(Replicated)
	TArray<FItemInstance> ReplicatedBankItems;

	void SyncInventoryFromReplicated();
	void SyncReplicatedFromInventory();

	TMap<EMMOEquipmentSlot, UMeshComponent*> EquippedMeshes;
};
