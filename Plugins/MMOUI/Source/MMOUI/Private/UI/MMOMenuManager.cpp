#include "UI/MMOMenuManager.h"
#include "UI/MMOHUDWidget.h"
#include "UI/MMOTooltipWidget.h"
#include "Engine/World.h"

UMMOMenuManager::UMMOMenuManager()
{
	PrimaryComponentTick.bCanEverTick = true;
	HistoryIndex = -1;
	bModalActive = false;
}

void UMMOMenuManager::BeginPlay()
{
	Super::BeginPlay();
}

void UMMOMenuManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UMMOMenuManager::OpenMenu(EMMOMenuType MenuType, UMMOHUDWidget* MenuWidget)
{
	if (!MenuWidget)
		return;

	if (!IsMenuOpen(MenuType))
	{
		FMMOMenuEntry Entry;
		Entry.MenuType = MenuType;
		Entry.MenuWidget = MenuWidget;
		MenuStack.Push(Entry);

		MenuWidget->Show();

		if (HistoryIndex >= 0)
		{
			ForwardHistory.Empty();
		}
		BackHistory.Push(MenuType);
		HistoryIndex = BackHistory.Num() - 1;
	}
}

void UMMOMenuManager::CloseMenu(EMMOMenuType MenuType)
{
	for (int32 i = MenuStack.Num() - 1; i >= 0; i--)
	{
		if (MenuStack[i].MenuType == MenuType)
		{
			if (MenuStack[i].MenuWidget)
			{
				MenuStack[i].MenuWidget->Hide();
			}
			MenuStack.RemoveAt(i);
			break;
		}
	}
}

void UMMOMenuManager::ToggleMenu(EMMOMenuType MenuType, UMMOHUDWidget* MenuWidget)
{
	if (IsMenuOpen(MenuType))
	{
		CloseMenu(MenuType);
	}
	else
	{
		OpenMenu(MenuType, MenuWidget);
	}
}

UMMOHUDWidget* UMMOMenuManager::GetTopMenu() const
{
	if (MenuStack.Num() > 0)
	{
		return MenuStack.Last().MenuWidget;
	}
	return nullptr;
}

bool UMMOMenuManager::IsMenuOpen(EMMOMenuType MenuType) const
{
	for (const auto& Entry : MenuStack)
	{
		if (Entry.MenuType == MenuType)
		{
			return true;
		}
	}
	return false;
}

void UMMOMenuManager::CloseAllMenus()
{
	for (auto& Entry : MenuStack)
	{
		if (Entry.MenuWidget)
		{
			Entry.MenuWidget->Hide();
		}
	}
	MenuStack.Empty();
}

void UMMOMenuManager::NavigateBack()
{
	if (HistoryIndex > 0)
	{
		ForwardHistory.Push(BackHistory[HistoryIndex]);
		HistoryIndex--;
		EMMOMenuType TargetMenu = BackHistory[HistoryIndex];

		CloseAllMenus();
	}
}

void UMMOMenuManager::NavigateForward()
{
	if (ForwardHistory.Num() > 0)
	{
		EMMOMenuType TargetMenu = ForwardHistory.Last();
		ForwardHistory.Pop();

		BackHistory.Push(TargetMenu);
		HistoryIndex = BackHistory.Num() - 1;
	}
}

void UMMOMenuManager::ClearHistory()
{
	BackHistory.Empty();
	ForwardHistory.Empty();
	HistoryIndex = -1;
}

void UMMOMenuManager::ShowModalDialog(const FText& Title, const FText& Message, FMMOModalCallback Callback)
{
	ActiveModal.Title = Title;
	ActiveModal.Message = Message;
	ActiveModal.Callback = Callback;
	bModalActive = true;
}

void UMMOMenuManager::ConfirmModal()
{
	if (bModalActive)
	{
		ActiveModal.Callback.ExecuteIfBound(EMMOModalResult::Confirmed);
		bModalActive = false;
	}
}

void UMMOMenuManager::CancelModal()
{
	if (bModalActive)
	{
		ActiveModal.Callback.ExecuteIfBound(EMMOModalResult::Cancelled);
		bModalActive = false;
	}
}

void UMMOMenuManager::ShowItemTooltip(UMMOTooltipWidget* TooltipWidget, const FString& ItemId)
{
	if (ActiveTooltip)
	{
		ActiveTooltip->Hide();
	}
	ActiveTooltip = TooltipWidget;
	if (ActiveTooltip)
	{
		ActiveTooltip->Show();
	}
}

void UMMOMenuManager::ShowAbilityTooltip(UMMOTooltipWidget* TooltipWidget, const FString& AbilityId)
{
	if (ActiveTooltip)
	{
		ActiveTooltip->Hide();
	}
	ActiveTooltip = TooltipWidget;
	if (ActiveTooltip)
	{
		ActiveTooltip->Show();
	}
}

void UMMOMenuManager::ShowUnitTooltip(UMMOTooltipWidget* TooltipWidget, const FString& UnitId)
{
	if (ActiveTooltip)
	{
		ActiveTooltip->Hide();
	}
	ActiveTooltip = TooltipWidget;
	if (ActiveTooltip)
	{
		ActiveTooltip->Show();
	}
}

void UMMOMenuManager::ShowQuestTooltip(UMMOTooltipWidget* TooltipWidget, const FString& QuestId)
{
	if (ActiveTooltip)
	{
		ActiveTooltip->Hide();
	}
	ActiveTooltip = TooltipWidget;
	if (ActiveTooltip)
	{
		ActiveTooltip->Show();
	}
}

void UMMOMenuManager::HideTooltip()
{
	if (ActiveTooltip)
	{
		ActiveTooltip->Hide();
		ActiveTooltip = nullptr;
	}
}

void UMMOMenuManager::ShowContextMenu(UMMOHUDWidget* ContextWidget, const FVector2D& ScreenPosition)
{
	if (ActiveContextMenu)
	{
		ActiveContextMenu->Hide();
	}
	ActiveContextMenu = ContextWidget;
	if (ActiveContextMenu)
	{
		ActiveContextMenu->Show();
	}
}

void UMMOMenuManager::HideContextMenu()
{
	if (ActiveContextMenu)
	{
		ActiveContextMenu->Hide();
		ActiveContextMenu = nullptr;
	}
}

void UMMOMenuManager::ShowNotification(const FText& Message, float Duration)
{
	if (NotificationWidgetClass)
	{
		UMMOHUDWidget* Notification = CreateWidget<UMMOHUDWidget>(GetWorld(), NotificationWidgetClass);
		if (Notification)
		{
			Notification->AddToViewport(10000);
			Notification->Show();
			ActiveNotifications.Add(Notification);
		}
	}
}

void UMMOMenuManager::ShowSystemMessage(const FText& Message, float Duration)
{
	if (SystemMessageWidgetClass)
	{
		UMMOHUDWidget* SysMsg = CreateWidget<UMMOHUDWidget>(GetWorld(), SystemMessageWidgetClass);
		if (SysMsg)
		{
			SysMsg->AddToViewport(10001);
			SysMsg->Show();
			ActiveNotifications.Add(SysMsg);
		}
	}
}

void UMMOMenuManager::ClearNotifications()
{
	for (auto& Notification : ActiveNotifications)
	{
		if (Notification)
		{
			Notification->Hide();
			Notification->RemoveFromParent();
		}
	}
	ActiveNotifications.Empty();
}
