#include "UI/MMOHUD.h"
#include "UI/MMOHUDWidget.h"
#include "UI/MMOMenuManager.h"
#include "UI/MMOCombatTextManager.h"
#include "Blueprint/UserWidget.h"
#include "Engine/World.h"

AMMOHUD::AMMOHUD()
{
	PrimaryActorTick.bCanEverTick = true;
	CurrentMode = EMMOHUDMode::Normal;
	bCrosshairVisible = true;
}

void AMMOHUD::BeginPlay()
{
	Super::BeginPlay();

	MenuManager = NewObject<UMMOMenuManager>(this, UMMOMenuManager::StaticClass());
	MenuManager->RegisterComponent();

	CombatTextManager = NewObject<UMMOCombatTextManager>(this, UMMOCombatTextManager::StaticClass());
	CombatTextManager->RegisterComponent();

	if (OverlayWidgetClass && !OverlayWidget)
	{
		OverlayWidget = CreateWidget<UMMOHUDWidget>(GetOwningPlayerController(), OverlayWidgetClass);
		if (OverlayWidget)
		{
			OverlayWidget->AddToViewport(0);
			OverlayWidget->Show();
		}
	}

	auto CreateWidgetOfClass = [&](TSubclassOf<UMMOHUDWidget>& WidgetClass, TObjectPtr<UMMOHUDWidget>& OutWidget, const FName& WidgetId, int32 ZOrder)
	{
		if (WidgetClass && !OutWidget)
		{
			OutWidget = CreateWidget<UMMOHUDWidget>(GetOwningPlayerController(), WidgetClass);
			if (OutWidget)
			{
				RegisterWidget(WidgetId, OutWidget, EMMOWidgetLayer::Medium);
				OutWidget->AddToViewport(ZOrder);
			}
		}
	};

	CreateWidgetOfClass(HealthBarWidgetClass, HealthBarWidget, FName("HealthBar"), 1);
	CreateWidgetOfClass(ManaBarWidgetClass, ManaBarWidget, FName("ManaBar"), 2);
	CreateWidgetOfClass(ExpBarWidgetClass, ExpBarWidget, FName("ExpBar"), 3);
	CreateWidgetOfClass(CastBarWidgetClass, CastBarWidget, FName("CastBar"), 4);
	CreateWidgetOfClass(TargetFrameWidgetClass, TargetFrameWidget, FName("TargetFrame"), 5);
	CreateWidgetOfClass(MinimapWidgetClass, MinimapWidget, FName("Minimap"), 6);
	CreateWidgetOfClass(HotbarWidgetClass, HotbarWidget, FName("Hotbar"), 7);
	CreateWidgetOfClass(BuffBarWidgetClass, BuffBarWidget, FName("BuffBar"), 8);
	CreateWidgetOfClass(ChatWidgetClass, ChatWidget, FName("Chat"), 9);
	CreateWidgetOfClass(PartyFramesWidgetClass, PartyFramesWidget, FName("PartyFrames"), 10);
	CreateWidgetOfClass(QuestTrackerWidgetClass, QuestTrackerWidget, FName("QuestTracker"), 11);
	CreateWidgetOfClass(MenuBarWidgetClass, MenuBarWidget, FName("MenuBar"), 12);

	ApplyHUDModeVisibility();
}

void AMMOHUD::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OverlayWidget)
	{
		OverlayWidget->RemoveFromParent();
		OverlayWidget = nullptr;
	}

	RegisteredWidgets.Empty();

	Super::EndPlay(EndPlayReason);
}

void AMMOHUD::DrawHUD()
{
	Super::DrawHUD();
}

void AMMOHUD::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AMMOHUD::ShowWidget(const FName& WidgetId)
{
	FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		Entry->Widget->Show();
	}
}

void AMMOHUD::HideWidget(const FName& WidgetId)
{
	FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		Entry->Widget->Hide();
	}
}

void AMMOHUD::ToggleWidget(const FName& WidgetId)
{
	FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		if (Entry->Widget->IsVisible())
		{
			Entry->Widget->Hide();
		}
		else
		{
			Entry->Widget->Show();
		}
	}
}

bool AMMOHUD::IsWidgetVisible(const FName& WidgetId) const
{
	const FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		return Entry->Widget->IsVisible();
	}
	return false;
}

void AMMOHUD::SetHUDMode(EMMOHUDMode NewMode)
{
	if (CurrentMode != NewMode)
	{
		CurrentMode = NewMode;
		ApplyHUDModeVisibility();
	}
}

void AMMOHUD::SetWidgetZOrder(const FName& WidgetId, int32 ZOrder)
{
	FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		Entry->ZOrder = ZOrder;
		Entry->Widget->RemoveFromParent();
		Entry->Widget->AddToViewport(Entry->ZOrder);
	}
}

void AMMOHUD::RegisterWidget(const FName& WidgetId, UMMOHUDWidget* Widget, EMMOWidgetLayer Layer)
{
	if (Widget)
	{
		FMMOWidgetEntry Entry;
		Entry.Widget = Widget;
		Entry.Layer = Layer;
		Entry.ZOrder = static_cast<int32>(Layer) * 100;
		RegisteredWidgets.Add(WidgetId, Entry);
	}
}

void AMMOHUD::UnregisterWidget(const FName& WidgetId)
{
	FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry && Entry->Widget)
	{
		Entry->Widget->RemoveFromParent();
	}
	RegisteredWidgets.Remove(WidgetId);
}

UMMOHUDWidget* AMMOHUD::GetWidget(const FName& WidgetId) const
{
	const FMMOWidgetEntry* Entry = RegisteredWidgets.Find(WidgetId);
	if (Entry)
	{
		return Entry->Widget;
	}
	return nullptr;
}

void AMMOHUD::ShowCrosshair(bool bShow)
{
	bCrosshairVisible = bShow;
	if (GetOwningPlayerController())
	{
		GetOwningPlayerController()->bShowMouseCursor = !bShow;
	}
}

void AMMOHUD::RebuildLayout()
{
	SortWidgetsByZOrder();
	ApplyHUDModeVisibility();
}

void AMMOHUD::SortWidgetsByZOrder()
{
	RegisteredWidgets.ValueSort([](const FMMOWidgetEntry& A, const FMMOWidgetEntry& B)
	{
		return A.ZOrder < B.ZOrder;
	});
}

void AMMOHUD::ApplyHUDModeVisibility()
{
	bool bShowNormalUI = (CurrentMode == EMMOHUDMode::Normal || CurrentMode == EMMOHUDMode::Combat);
	bool bShowCombat = (CurrentMode == EMMOHUDMode::Combat);
	bool bShowDialog = (CurrentMode == EMMOHUDMode::Dialog);
	bool bCinematic = (CurrentMode == EMMOHUDMode::Cinematic);
	bool bDead = (CurrentMode == EMMOHUDMode::Dead);

	if (HealthBarWidget) HealthBarWidget->SetVisibility(bShowNormalUI || bShowCombat || bDead ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ManaBarWidget) ManaBarWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ExpBarWidget) ExpBarWidget->SetVisibility(bShowNormalUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (CastBarWidget) CastBarWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (TargetFrameWidget) TargetFrameWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (MinimapWidget) MinimapWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (HotbarWidget) HotbarWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (BuffBarWidget) BuffBarWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (ChatWidget) ChatWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (PartyFramesWidget) PartyFramesWidget->SetVisibility(bShowNormalUI || bShowCombat ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (QuestTrackerWidget) QuestTrackerWidget->SetVisibility(bShowNormalUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	if (MenuBarWidget) MenuBarWidget->SetVisibility(bShowNormalUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}
