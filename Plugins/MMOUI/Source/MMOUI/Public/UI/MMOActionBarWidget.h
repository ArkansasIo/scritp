#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "MMOActionBarWidget.generated.h"

UENUM(BlueprintType)
enum class EMMOActionBarType : uint8
{
	MainBar,
	BottomBar,
	RightBar1,
	RightBar2,
	LeftBar,
	StanceBar,
	PetBar,
	VehicleBar
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOActionSlotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 SlotIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText KeyBind;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownRemaining = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CooldownMax = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsUsable = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOutOfRange = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bOutOfResources = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 StackCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEmpty = true;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMMOOnActionSlotClicked, int32, SlotIndex, EMMOActionBarType, BarType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FMMOOnActionSlotDragged, int32, SourceSlot, int32, TargetSlot, EMMOActionBarType, BarType);

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOActionBarWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMOActionBarWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void SetActionBarType(EMMOActionBarType NewType);

	UFUNCTION(BlueprintPure, Category = "Action Bar")
	EMMOActionBarType GetActionBarType() const { return BarType; }

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void UpdateSlot(int32 SlotIndex, const FMMOActionSlotData& SlotData);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void UpdateAllSlots(const TArray<FMMOActionSlotData>& SlotData);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void ClearSlot(int32 SlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void ClearAllSlots();

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void SetSlotsPerRow(int32 SlotsPerRow);

	UFUNCTION(BlueprintPure, Category = "Action Bar")
	int32 GetNumSlots() const { return NumSlots; }

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void SetNumSlots(int32 InNumSlots);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void SetKeyBind(int32 SlotIndex, const FText& KeyBind);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void UpdateCooldowns(const TArray<float>& Cooldowns);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void UpdateUsability(const TArray<bool>& Usability);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action Bar")
	void OnSlotUpdated(int32 SlotIndex, const FMMOActionSlotData& SlotData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action Bar")
	void OnSlotClicked(int32 SlotIndex);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action Bar")
	void OnBarTypeChanged(EMMOActionBarType NewType);

	UFUNCTION(BlueprintImplementableEvent, Category = "Action Bar")
	void OnStanceChanged(int32 NewStanceIndex);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void BindActionSlot(int32 SlotIndex, const FString& AbilityID);

	UFUNCTION(BlueprintCallable, Category = "Action Bar")
	void UnbindActionSlot(int32 SlotIndex);

	UPROPERTY(BlueprintAssignable, Category = "Action Bar")
	FMMOOnActionSlotClicked OnActionSlotClicked;

	UPROPERTY(BlueprintAssignable, Category = "Action Bar")
	FMMOOnActionSlotDragged OnActionSlotDragged;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Bar")
	EMMOActionBarType BarType = EMMOActionBarType::MainBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Bar")
	int32 NumSlots = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Bar")
	int32 SlotsPerRow = 6;

	UPROPERTY(BlueprintReadOnly, Category = "Action Bar")
	TArray<FMMOActionSlotData> Slots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Bar")
	TSubclassOf<UMMOHUDWidget> SlotWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Action Bar")
	TObjectPtr<UPanelWidget> SlotContainer;

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> SlotWidgets;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Action Bar")
	int32 CurrentStanceIndex = 0;

	void RebuildSlots();
	void ApplySlotVisual(int32 SlotIndex);
};
