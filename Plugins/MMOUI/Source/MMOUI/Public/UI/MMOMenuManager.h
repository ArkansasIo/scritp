#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MMOMenuManager.generated.h"

class UMMOHUDWidget;
class UMMOTooltipWidget;

UENUM(BlueprintType)
enum class EMMOMenuType : uint8
{
	None,
	CharacterSheet,
	Inventory,
	SpellBook,
	QuestLog,
	Map,
	MainMenu,
	Settings,
	Social,
	Guild,
	Shop,
	Mail,
	Loot,
	Dialog,
	Merchant,
	Trainer,
	TalentTree,
	Achievement,
	Collections
};

UENUM(BlueprintType)
enum class EMMOModalResult : uint8
{
	None,
	Confirmed,
	Cancelled
};

DECLARE_DELEGATE_OneParam(FMMOModalCallback, EMMOModalResult);

USTRUCT(BlueprintType)
struct MMOUI_API FMMOMenuEntry
{
	GENERATED_BODY()

	UPROPERTY()
	EMMOMenuType MenuType = EMMOMenuType::None;

	UPROPERTY()
	TObjectPtr<UMMOHUDWidget> MenuWidget = nullptr;
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOModalEntry
{
	GENERATED_BODY()

	UPROPERTY()
	FText Title;

	UPROPERTY()
	FText Message;

	FMMOModalCallback Callback;
};

UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class MMOUI_API UMMOMenuManager : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOMenuManager();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void OpenMenu(EMMOMenuType MenuType, UMMOHUDWidget* MenuWidget);

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void CloseMenu(EMMOMenuType MenuType);

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void ToggleMenu(EMMOMenuType MenuType, UMMOHUDWidget* MenuWidget);

	UFUNCTION(BlueprintPure, Category = "Menu Manager")
	UMMOHUDWidget* GetTopMenu() const;

	UFUNCTION(BlueprintPure, Category = "Menu Manager")
	bool IsMenuOpen(EMMOMenuType MenuType) const;

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void CloseAllMenus();

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void NavigateBack();

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void NavigateForward();

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void ClearHistory();

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void ShowModalDialog(const FText& Title, const FText& Message, FMMOModalCallback Callback);

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void ConfirmModal();

	UFUNCTION(BlueprintCallable, Category = "Menu Manager")
	void CancelModal();

	UFUNCTION(BlueprintPure, Category = "Menu Manager")
	bool IsModalActive() const { return bModalActive; }

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowItemTooltip(UMMOTooltipWidget* TooltipWidget, const FString& ItemId);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowAbilityTooltip(UMMOTooltipWidget* TooltipWidget, const FString& AbilityId);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowUnitTooltip(UMMOTooltipWidget* TooltipWidget, const FString& UnitId);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowQuestTooltip(UMMOTooltipWidget* TooltipWidget, const FString& QuestId);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void HideTooltip();

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void ShowContextMenu(UMMOHUDWidget* ContextWidget, const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintCallable, Category = "Context Menu")
	void HideContextMenu();

	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowNotification(const FText& Message, float Duration = 3.0f);

	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ShowSystemMessage(const FText& Message, float Duration = 5.0f);

	UFUNCTION(BlueprintCallable, Category = "Notifications")
	void ClearNotifications();

protected:
	UPROPERTY()
	TArray<FMMOMenuEntry> MenuStack;

	UPROPERTY()
	TArray<EMMOMenuType> BackHistory;

	UPROPERTY()
	TArray<EMMOMenuType> ForwardHistory;

	UPROPERTY()
	int32 HistoryIndex;

	UPROPERTY()
	FMMOModalEntry ActiveModal;

	UPROPERTY()
	bool bModalActive;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<UMMOTooltipWidget> ItemTooltipClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<UMMOTooltipWidget> AbilityTooltipClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<UMMOTooltipWidget> UnitTooltipClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Tooltip")
	TSubclassOf<UMMOTooltipWidget> QuestTooltipClass;

	UPROPERTY()
	TObjectPtr<UMMOTooltipWidget> ActiveTooltip;

	UPROPERTY()
	TObjectPtr<UMMOHUDWidget> ActiveContextMenu;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Context Menu")
	TSubclassOf<UMMOHUDWidget> ContextMenuClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notifications")
	TSubclassOf<UMMOHUDWidget> NotificationWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UMMOHUDWidget>> ActiveNotifications;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Notifications")
	TSubclassOf<UMMOHUDWidget> SystemMessageWidgetClass;
};
