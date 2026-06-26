#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "UObject/SoftObjectPtr.h"
#include "MMOHUD.generated.h"

class UMMOHUDWidget;
class UMMOMenuManager;
class UMMOCombatTextManager;

UENUM(BlueprintType)
enum class EMMOHUDMode : uint8
{
	Normal,
	Combat,
	Dialog,
	UI,
	Cinematic,
	Dead
};

UENUM(BlueprintType)
enum class EMMOWidgetLayer : uint8
{
	Background,
	Low,
	Medium,
	High,
	Overlay,
	Modal,
	Tooltip,
	Max
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOWidgetEntry
{
	GENERATED_BODY()

	UPROPERTY()
	UMMOHUDWidget* Widget = nullptr;

	UPROPERTY()
	EMMOWidgetLayer Layer = EMMOWidgetLayer::Medium;

	UPROPERTY()
	int32 ZOrder = 0;
};

UCLASS()
class MMOUI_API AMMOHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMMOHUD();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void DrawHUD() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void ShowWidget(const FName& WidgetId);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void HideWidget(const FName& WidgetId);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void ToggleWidget(const FName& WidgetId);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	bool IsWidgetVisible(const FName& WidgetId) const;

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void SetHUDMode(EMMOHUDMode NewMode);

	UFUNCTION(BlueprintPure, Category = "MMO HUD")
	EMMOHUDMode GetHUDMode() const { return CurrentMode; }

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void SetWidgetZOrder(const FName& WidgetId, int32 ZOrder);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void RegisterWidget(const FName& WidgetId, UMMOHUDWidget* Widget, EMMOWidgetLayer Layer = EMMOWidgetLayer::Medium);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void UnregisterWidget(const FName& WidgetId);

	UFUNCTION(BlueprintPure, Category = "MMO HUD")
	UMMOHUDWidget* GetWidget(const FName& WidgetId) const;

	UFUNCTION(BlueprintPure, Category = "MMO HUD")
	FORCEINLINE UMMOMenuManager* GetMenuManager() const { return MenuManager; }

	UFUNCTION(BlueprintPure, Category = "MMO HUD")
	FORCEINLINE UMMOCombatTextManager* GetCombatTextManager() const { return CombatTextManager; }

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void ShowCrosshair(bool bShow);

	UFUNCTION(BlueprintPure, Category = "MMO HUD")
	bool IsCrosshairVisible() const { return bCrosshairVisible; }

	UFUNCTION(BlueprintCallable, Category = "MMO HUD")
	void RebuildLayout();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> OverlayWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> OverlayWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> HealthBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> HealthBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> ManaBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> ManaBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> ExpBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> ExpBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> CastBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> CastBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> TargetFrameWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> TargetFrameWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> MinimapWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> MinimapWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> HotbarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> HotbarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> BuffBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> BuffBarWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> ChatWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> ChatWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> PartyFramesWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> PartyFramesWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> QuestTrackerWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> QuestTrackerWidget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widgets")
	TSubclassOf<UMMOHUDWidget> MenuBarWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Widgets")
	TObjectPtr<UMMOHUDWidget> MenuBarWidget;

	UPROPERTY()
	TObjectPtr<UMMOMenuManager> MenuManager;

	UPROPERTY()
	TObjectPtr<UMMOCombatTextManager> CombatTextManager;

	UPROPERTY()
	TMap<FName, FMMOWidgetEntry> RegisteredWidgets;

	UPROPERTY()
	EMMOHUDMode CurrentMode;

	bool bCrosshairVisible;

	void SortWidgetsByZOrder();
	void ApplyHUDModeVisibility();
};
