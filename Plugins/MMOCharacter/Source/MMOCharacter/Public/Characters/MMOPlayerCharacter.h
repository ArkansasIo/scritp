#pragma once

#include "CoreMinimal.h"
#include "Characters/MMOCharacterBase.h"
#include "Items/MMOEquipmentItem.h"
#include "GameplayTagContainer.h"
#include "MMOPlayerCharacter.generated.h"

class UMMOCharacterMovementComponent;
class UDataAsset;

USTRUCT(BlueprintType)
struct FMMOAppearanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	int32 HairStyleIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	int32 FaceTypeIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor SkinColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor HairColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FLinearColor EyeColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	int32 BodyTypeIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float HeightScale;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	float WeightScale;
};

UENUM(BlueprintType)
enum class EMMOEmoteType : uint8
{
	Wave,
	Cheer,
	Bow,
	Laugh,
	Cry,
	Dance,
	Applaud,
	Salute,
	Sit,
	LayDown,
	Custom
};

UENUM(BlueprintType)
enum class EMMOPlayerRestState : uint8
{
	Standing,
	Sitting,
	Resting
};

UENUM(BlueprintType)
enum class EMMOPvPFlag : uint8
{
	Peaceful,
	Flagged,
	InPvP
};

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipmentChanged, UMMOEquipmentItem*, EquipItem);

UCLASS(Blueprintable, BlueprintType)
class MMOCHARACTER_API AMMOPlayerCharacter : public AMMOCharacterBase
{
	GENERATED_BODY()

public:
	AMMOPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, ReplicatedUsing = OnRep_AppearanceData, Category = "MMO|Appearance")
	FMMOAppearanceData AppearanceData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Appearance")
	TObjectPtr<UDataAsset> AppearanceDataAsset;

	UFUNCTION()
	virtual void OnRep_AppearanceData(const FMMOAppearanceData& OldAppearanceData);

	UFUNCTION(BlueprintCallable, Category = "MMO|Appearance")
	virtual void UpdateAppearance(const FMMOAppearanceData& NewAppearance);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Appearance")
	void BP_ApplyAppearance(const FMMOAppearanceData& Appearance);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "MMO|Emote")
	virtual void MulticastPlayEmote(EMMOEmoteType Emote, UAnimMontage* CustomMontage);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Emote")
	void BP_PlayEmoteMontage(EMMOEmoteType Emote, UAnimMontage* CustomMontage);

	UFUNCTION(BlueprintCallable, Category = "MMO|Emote")
	virtual void PlayEmote(EMMOEmoteType Emote, UAnimMontage* CustomMontage);

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|State")
	EMMOPlayerRestState RestState;

	UFUNCTION(BlueprintCallable, Category = "MMO|State")
	virtual void SetRestState(EMMOPlayerRestState NewRestState);

	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastSetRestState(EMMOPlayerRestState NewRestState);

	UPROPERTY(ReplicatedUsing = OnRep_PvPFlag, BlueprintReadOnly, Category = "MMO|PvP")
	EMMOPvPFlag PvPFlag;

	UFUNCTION()
	virtual void OnRep_PvPFlag(EMMOPvPFlag OldFlag);

	UFUNCTION(BlueprintCallable, Category = "MMO|PvP")
	virtual void SetPvPFlag(EMMOPvPFlag NewFlag);

	UFUNCTION(Server, Reliable)
	virtual void ServerSetPvPFlag(EMMOPvPFlag NewFlag);

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Equipment")
	TArray<TObjectPtr<UMMOEquipmentItem>> EquipmentSlots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Equipment")
	int32 NumEquipmentSlots;

	UFUNCTION(BlueprintCallable, Category = "MMO|Equipment")
	virtual void EquipItem(UMMOEquipmentItem* Item);

	UFUNCTION(BlueprintCallable, Category = "MMO|Equipment")
	virtual void UnEquipItem(int32 SlotIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|Equipment")
	void BP_UpdateEquipmentMesh(int32 SlotIndex, UMMOEquipmentItem* Item);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnEquipmentChanged OnEquipmentChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Abilities")
	TArray<FGameplayTag> QuickSlotAbilities;

	UFUNCTION(BlueprintCallable, Category = "MMO|Abilities")
	virtual void SetQuickSlotAbility(int32 SlotIndex, const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable, Category = "MMO|Abilities")
	virtual void ClearQuickSlotAbility(int32 SlotIndex);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	int32 PlayerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|CharacterInfo")
	int32 TotalPlayTimeSeconds;

protected:
	virtual void OnRep_Level(int32 OldLevel) override;
	virtual void OnDeath() override;
};
