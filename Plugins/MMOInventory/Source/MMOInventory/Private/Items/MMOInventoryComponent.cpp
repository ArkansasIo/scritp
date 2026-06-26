#include "Items/MMOInventoryComponent.h"
#include "Items/MMOEquipmentItem.h"
#include "Items/MMOConsumableItem.h"
#include "UI/MMOInventoryInterface.h"
#include "Engine/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"

UMMOInventoryComponent::UMMOInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxInventorySlots = 50;
	MaxInventoryWeight = 200.0f;
	MaxBankSlots = 100;

	Gold = 0;
	Silver = 0;
	Copper = 0;
}

void UMMOInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMMOInventoryComponent, EquipmentSlots);
	DOREPLIFETIME(UMMOInventoryComponent, Gold);
	DOREPLIFETIME(UMMOInventoryComponent, Silver);
	DOREPLIFETIME(UMMOInventoryComponent, Copper);
	DOREPLIFETIME(UMMOInventoryComponent, ReplicatedInventoryItems);
	DOREPLIFETIME(UMMOInventoryComponent, ReplicatedBankItems);
}

void UMMOInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	if (!InventoryContainer)
	{
		InventoryContainer = NewObject<UMMOItemContainer>(this);
		InventoryContainer->ContainerType = EMMOContainerType::Inventory;
		InventoryContainer->MaxSlots = MaxInventorySlots;
		InventoryContainer->MaxWeight = MaxInventoryWeight;
	}

	if (!BankContainer)
	{
		BankContainer = NewObject<UMMOItemContainer>(this);
		BankContainer->ContainerType = EMMOContainerType::Bank;
		BankContainer->MaxSlots = MaxBankSlots;
		BankContainer->MaxWeight = 0.0f;
	}
}

void UMMOInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UMMOInventoryComponent::AddItem(const FItemInstance& Item, bool bToBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_AddItem(Item, bToBank);
		return true;
	}

	UMMOItemContainer* TargetContainer = bToBank ? BankContainer : InventoryContainer;
	if (!TargetContainer)
	{
		return false;
	}

	int32 Result = TargetContainer->AddItem(Item);
	if (Result == INDEX_NONE)
	{
		return false;
	}

	SyncReplicatedFromInventory();
	return true;
}

bool UMMOInventoryComponent::RemoveItem(int32 SlotIndex, int32 Quantity, bool bFromBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_RemoveItem(SlotIndex, Quantity, bFromBank);
		return true;
	}

	UMMOItemContainer* TargetContainer = bFromBank ? BankContainer : InventoryContainer;
	if (!TargetContainer || !TargetContainer->Items.IsValidIndex(SlotIndex))
	{
		return false;
	}

	bool bResult = TargetContainer->RemoveItem(SlotIndex, Quantity);
	if (bResult)
	{
		SyncReplicatedFromInventory();
	}
	return bResult;
}

bool UMMOInventoryComponent::MoveItem(int32 FromIndex, int32 ToIndex, bool bFromBank, bool bToBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_MoveItem(FromIndex, ToIndex, bFromBank, bToBank);
		return true;
	}

	UMMOItemContainer* SourceContainer = bFromBank ? BankContainer : InventoryContainer;
	UMMOItemContainer* DestContainer = bToBank ? BankContainer : InventoryContainer;

	if (!SourceContainer || !DestContainer)
	{
		return false;
	}

	if (SourceContainer == DestContainer)
	{
		bool bResult = SourceContainer->MoveItem(FromIndex, ToIndex);
		if (bResult)
		{
			SyncReplicatedFromInventory();
		}
		return bResult;
	}

	if (!SourceContainer->Items.IsValidIndex(FromIndex))
	{
		return false;
	}

	FItemInstance ItemToMove = SourceContainer->Items[FromIndex];
	bool bHasSpace = DestContainer->HasSpaceFor(ItemToMove);
	if (!bHasSpace)
	{
		return false;
	}

	SourceContainer->RemoveItem(FromIndex, ItemToMove.Quantity);
	DestContainer->AddItem(ItemToMove);

	SyncReplicatedFromInventory();
	return true;
}

bool UMMOInventoryComponent::UseItem(int32 SlotIndex, bool bFromBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UseItem(SlotIndex, bFromBank);
		return true;
	}

	UMMOItemContainer* TargetContainer = bFromBank ? BankContainer : InventoryContainer;
	if (!TargetContainer || !TargetContainer->Items.IsValidIndex(SlotIndex))
	{
		return false;
	}

	FItemInstance& ItemInstance = TargetContainer->Items[SlotIndex];
	if (!ItemInstance.ItemDefinition.IsValid())
	{
		return false;
	}

	UMMOItemBase* ItemDef = ItemInstance.ItemDefinition.LoadSynchronous();
	if (!ItemDef)
	{
		return false;
	}

	APlayerController* OwnerPC = Cast<APlayerController>(GetOwner());
	if (!OwnerPC)
	{
		return false;
	}

	bool bUsed = ItemDef->OnUse(OwnerPC);
	if (bUsed)
	{
		if (UMMOConsumableItem* Consumable = Cast<UMMOConsumableItem>(ItemDef))
		{
			int32 Consumed = Consumable->StacksConsumedPerUse;
			TargetContainer->RemoveItem(SlotIndex, Consumed);
		}
		SyncReplicatedFromInventory();
	}

	return bUsed;
}

bool UMMOInventoryComponent::SplitItem(int32 SlotIndex, int32 SplitQuantity, int32 TargetIndex, bool bFromBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_SplitItem(SlotIndex, SplitQuantity, TargetIndex, bFromBank);
		return true;
	}

	UMMOItemContainer* TargetContainer = bFromBank ? BankContainer : InventoryContainer;
	if (!TargetContainer || !TargetContainer->Items.IsValidIndex(SlotIndex))
	{
		return false;
	}

	FItemInstance& SourceItem = TargetContainer->Items[SlotIndex];
	if (SplitQuantity <= 0 || SplitQuantity >= SourceItem.Quantity)
	{
		return false;
	}

	FItemInstance NewItem;
	NewItem.ItemDefinition = SourceItem.ItemDefinition;
	NewItem.Quantity = SplitQuantity;
	NewItem.CurrentDurability = SourceItem.CurrentDurability;
	NewItem.EnchantmentLevel = SourceItem.EnchantmentLevel;
	NewItem.SocketedGems = SourceItem.SocketedGems;
	NewItem.EnchantmentId = SourceItem.EnchantmentId;

	SourceItem.Quantity -= SplitQuantity;

	if (TargetIndex >= 0 && TargetIndex < TargetContainer->Items.Num())
	{
		TargetContainer->Items.Insert(NewItem, TargetIndex);
	}
	else
	{
		TargetContainer->Items.Add(NewItem);
	}

	TargetContainer->OnContainerUpdated.Broadcast();
	SyncReplicatedFromInventory();
	return true;
}

bool UMMOInventoryComponent::EquipItem(int32 SlotIndex, bool bFromBank)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_EquipItem(SlotIndex, bFromBank);
		return true;
	}

	UMMOItemContainer* TargetContainer = bFromBank ? BankContainer : InventoryContainer;
	if (!TargetContainer || !TargetContainer->Items.IsValidIndex(SlotIndex))
	{
		return false;
	}

	FItemInstance& ItemInstance = TargetContainer->Items[SlotIndex];
	if (!ItemInstance.ItemDefinition.IsValid())
	{
		return false;
	}

	UMMOItemBase* ItemDef = ItemInstance.ItemDefinition.LoadSynchronous();
	UMMOEquipmentItem* EquipDef = Cast<UMMOEquipmentItem>(ItemDef);
	if (!EquipDef)
	{
		return false;
	}

	EMMOEquipmentSlot TargetSlot = EquipDef->EquipmentSlot;

	FItemInstance* ExistingItem = EquipmentSlots.Find(TargetSlot);
	if (ExistingItem && ExistingItem->IsValid())
	{
		TargetContainer->AddItem(*ExistingItem);
	}

	FItemInstance EquippedInstance = ItemInstance;
	TargetContainer->RemoveItem(SlotIndex, ItemInstance.Quantity);
	EquipmentSlots.Add(TargetSlot, EquippedInstance);

	AttachEquipmentMesh(EquippedInstance, TargetSlot);

	OnEquipmentChanged.Broadcast(TargetSlot, EquippedInstance, true);
	OnRep_Equipment();
	return true;
}

bool UMMOInventoryComponent::UnequipItem(EMMOEquipmentSlot Slot)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_UnequipItem(Slot);
		return true;
	}

	FItemInstance* ItemInstance = EquipmentSlots.Find(Slot);
	if (!ItemInstance || !ItemInstance->IsValid())
	{
		return false;
	}

	if (!InventoryContainer->HasSpaceFor(*ItemInstance))
	{
		return false;
	}

	FItemInstance UnequippedItem = *ItemInstance;
	EquipmentSlots.Remove(Slot);
	InventoryContainer->AddItem(UnequippedItem);

	DetachEquipmentMesh(Slot);

	OnEquipmentChanged.Broadcast(Slot, UnequippedItem, false);
	OnRep_Equipment();
	SyncReplicatedFromInventory();
	return true;
}

FItemInstance UMMOInventoryComponent::GetEquippedItem(EMMOEquipmentSlot Slot) const
{
	const FItemInstance* Found = EquipmentSlots.Find(Slot);
	if (Found)
	{
		return *Found;
	}
	return FItemInstance();
}

void UMMOInventoryComponent::AddCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		Server_AddCurrency(GoldAmount, SilverAmount, CopperAmount);
		return;
	}

	int32 TotalCopper = (Gold * 10000) + (Silver * 100) + Copper;
	TotalCopper += (GoldAmount * 10000) + (SilverAmount * 100) + CopperAmount;

	if (TotalCopper < 0)
	{
		TotalCopper = 0;
	}

	Gold = TotalCopper / 10000;
	Silver = (TotalCopper % 10000) / 100;
	Copper = TotalCopper % 100;

	OnRep_Currency();
}

bool UMMOInventoryComponent::RemoveCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	if (!HasEnoughCurrency(GoldAmount, SilverAmount, CopperAmount))
	{
		return false;
	}

	AddCurrency(-GoldAmount, -SilverAmount, -CopperAmount);
	return true;
}

bool UMMOInventoryComponent::HasEnoughCurrency(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount) const
{
	int32 TotalCopper = (Gold * 10000) + (Silver * 100) + Copper;
	int32 RequiredCopper = (GoldAmount * 10000) + (SilverAmount * 100) + CopperAmount;
	return TotalCopper >= RequiredCopper;
}

float UMMOInventoryComponent::GetTotalWeight() const
{
	if (!InventoryContainer)
	{
		return 0.0f;
	}
	return InventoryContainer->GetTotalWeight();
}

float UMMOInventoryComponent::GetRemainingWeight() const
{
	return MaxInventoryWeight - GetTotalWeight();
}

bool UMMOInventoryComponent::WouldExceedWeight(const FItemInstance& Item) const
{
	if (!InventoryContainer)
	{
		return true;
	}
	return !InventoryContainer->HasSpaceFor(Item);
}

int32 UMMOInventoryComponent::FindItemInInventory(FName ItemId) const
{
	if (!InventoryContainer)
	{
		return INDEX_NONE;
	}
	return InventoryContainer->FindItemIndex(ItemId);
}

int32 UMMOInventoryComponent::GetItemCount(FName ItemId) const
{
	if (!InventoryContainer)
	{
		return 0;
	}

	int32 TotalCount = 0;
	for (const FItemInstance& Item : InventoryContainer->Items)
	{
		if (Item.ItemDefinition.IsValid())
		{
			UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
			if (ItemDef && ItemDef->ItemID == ItemId)
			{
				TotalCount += Item.Quantity;
			}
		}
	}
	return TotalCount;
}

void UMMOInventoryComponent::AutoSort()
{
	if (!InventoryContainer)
	{
		return;
	}
	InventoryContainer->SortByType();
	SyncReplicatedFromInventory();
}

void UMMOInventoryComponent::OpenLootContainer(UMMOItemContainer* LootContainer)
{
	if (!LootContainer)
	{
		return;
	}
}

void UMMOInventoryComponent::LootItem(int32 ContainerSlotIndex, UMMOItemContainer* LootContainer)
{
	if (!LootContainer || !LootContainer->Items.IsValidIndex(ContainerSlotIndex))
	{
		return;
	}

	FItemInstance ItemToLoot = LootContainer->Items[ContainerSlotIndex];
	if (AddItem(ItemToLoot, false))
	{
		LootContainer->RemoveItem(ContainerSlotIndex, ItemToLoot.Quantity);
	}
}

void UMMOInventoryComponent::LootAll(UMMOItemContainer* LootContainer)
{
	if (!LootContainer)
	{
		return;
	}

	for (int32 i = LootContainer->Items.Num() - 1; i >= 0; i--)
	{
		LootItem(i, LootContainer);
	}
}

void UMMOInventoryComponent::Server_AddItem_Implementation(const FItemInstance& Item, bool bToBank)
{
	AddItem(Item, bToBank);
}

bool UMMOInventoryComponent::Server_AddItem_Validate(const FItemInstance& Item, bool bToBank)
{
	return true;
}

void UMMOInventoryComponent::Server_RemoveItem_Implementation(int32 SlotIndex, int32 Quantity, bool bFromBank)
{
	RemoveItem(SlotIndex, Quantity, bFromBank);
}

bool UMMOInventoryComponent::Server_RemoveItem_Validate(int32 SlotIndex, int32 Quantity, bool bFromBank)
{
	return SlotIndex >= 0 && Quantity > 0;
}

void UMMOInventoryComponent::Server_MoveItem_Implementation(int32 FromIndex, int32 ToIndex, bool bFromBank, bool bToBank)
{
	MoveItem(FromIndex, ToIndex, bFromBank, bToBank);
}

bool UMMOInventoryComponent::Server_MoveItem_Validate(int32 FromIndex, int32 ToIndex, bool bFromBank, bool bToBank)
{
	return FromIndex >= 0 && ToIndex >= 0;
}

void UMMOInventoryComponent::Server_UseItem_Implementation(int32 SlotIndex, bool bFromBank)
{
	UseItem(SlotIndex, bFromBank);
}

bool UMMOInventoryComponent::Server_UseItem_Validate(int32 SlotIndex, bool bFromBank)
{
	return SlotIndex >= 0;
}

void UMMOInventoryComponent::Server_SplitItem_Implementation(int32 SlotIndex, int32 SplitQuantity, int32 TargetIndex, bool bFromBank)
{
	SplitItem(SlotIndex, SplitQuantity, TargetIndex, bFromBank);
}

bool UMMOInventoryComponent::Server_SplitItem_Validate(int32 SlotIndex, int32 SplitQuantity, int32 TargetIndex, bool bFromBank)
{
	return SlotIndex >= 0 && SplitQuantity > 0;
}

void UMMOInventoryComponent::Server_EquipItem_Implementation(int32 SlotIndex, bool bFromBank)
{
	EquipItem(SlotIndex, bFromBank);
}

bool UMMOInventoryComponent::Server_EquipItem_Validate(int32 SlotIndex, bool bFromBank)
{
	return SlotIndex >= 0;
}

void UMMOInventoryComponent::Server_UnequipItem_Implementation(EMMOEquipmentSlot Slot)
{
	UnequipItem(Slot);
}

bool UMMOInventoryComponent::Server_UnequipItem_Validate(EMMOEquipmentSlot Slot)
{
	return true;
}

void UMMOInventoryComponent::Server_AddCurrency_Implementation(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	AddCurrency(GoldAmount, SilverAmount, CopperAmount);
}

bool UMMOInventoryComponent::Server_AddCurrency_Validate(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	return GoldAmount >= 0 && SilverAmount >= 0 && CopperAmount >= 0;
}

void UMMOInventoryComponent::Server_RemoveCurrency_Implementation(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	RemoveCurrency(GoldAmount, SilverAmount, CopperAmount);
}

bool UMMOInventoryComponent::Server_RemoveCurrency_Validate(int32 GoldAmount, int32 SilverAmount, int32 CopperAmount)
{
	return GoldAmount >= 0 && SilverAmount >= 0 && CopperAmount >= 0;
}

void UMMOInventoryComponent::Client_UpdateInventoryItem_Implementation(int32 SlotIndex, const FItemInstance& Item)
{
	if (InventoryContainer && InventoryContainer->Items.IsValidIndex(SlotIndex))
	{
		InventoryContainer->Items[SlotIndex] = Item;
		InventoryContainer->OnItemChanged.Broadcast(SlotIndex, Item);
	}
}

void UMMOInventoryComponent::Client_RefreshInventory_Implementation()
{
	SyncInventoryFromReplicated();
}

void UMMOInventoryComponent::OnRep_Equipment()
{
	for (const TPair<EMMOEquipmentSlot, FItemInstance>& Pair : EquipmentSlots)
	{
		if (Pair.Value.IsValid())
		{
			OnEquipmentChanged.Broadcast(Pair.Key, Pair.Value, true);
			AttachEquipmentMesh(Pair.Value, Pair.Key);
		}
	}
}

void UMMOInventoryComponent::OnRep_Currency()
{
	OnCurrencyChanged.Broadcast(Gold, Silver, Copper);
}

void UMMOInventoryComponent::AttachEquipmentMesh(const FItemInstance& Item, EMMOEquipmentSlot Slot)
{
	if (!Item.ItemDefinition.IsValid())
	{
		return;
	}

	UMMOItemBase* ItemDef = Item.ItemDefinition.LoadSynchronous();
	UMMOEquipmentItem* EquipDef = Cast<UMMOEquipmentItem>(ItemDef);
	if (!EquipDef || EquipDef->EquippedMesh.IsNull())
	{
		return;
	}

	AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	USkeletalMeshComponent* SkeletalComp = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();
	if (!SkeletalComp)
	{
		return;
	}

	DetachEquipmentMesh(Slot);
}

void UMMOInventoryComponent::DetachEquipmentMesh(EMMOEquipmentSlot Slot)
{
	UMeshComponent** Found = EquippedMeshes.Find(Slot);
	if (Found && *Found)
	{
		(*Found)->DestroyComponent();
		EquippedMeshes.Remove(Slot);
	}
}

void UMMOInventoryComponent::SyncInventoryFromReplicated()
{
	if (InventoryContainer)
	{
		InventoryContainer->Items = ReplicatedInventoryItems;
	}
	if (BankContainer)
	{
		BankContainer->Items = ReplicatedBankItems;
	}
}

void UMMOInventoryComponent::SyncReplicatedFromInventory()
{
	if (InventoryContainer)
	{
		ReplicatedInventoryItems = InventoryContainer->Items;
	}
	if (BankContainer)
	{
		ReplicatedBankItems = BankContainer->Items;
	}
}
