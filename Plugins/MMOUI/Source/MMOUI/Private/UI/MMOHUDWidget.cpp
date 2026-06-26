#include "UI/MMOHUDWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/UserInterfaceSettings.h"
#include "Engine/Engine.h"
#include "Framework/Application/SlateApplication.h"

UMMOHUDWidget::UMMOHUDWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVisible = true;
	bIsFading = false;
}

void UMMOHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();
	UpdateSafeZone();
}

void UMMOHUDWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

void UMMOHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
}

void UMMOHUDWidget::PlayFadeIn(float Duration)
{
	if (FadeInAnimation)
	{
		bIsFading = true;
		OnFadeInStarted();
		SetVisibility(ESlateVisibility::Visible);
		PlayAnimation(FadeInAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
	else
	{
		SetVisibility(ESlateVisibility::Visible);
		FadeInFinished();
	}
}

void UMMOHUDWidget::PlayFadeOut(float Duration)
{
	if (FadeOutAnimation)
	{
		bIsFading = true;
		OnFadeOutStarted();
		PlayAnimation(FadeOutAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f, false);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
		FadeOutFinished();
	}
}

void UMMOHUDWidget::SetWidgetVisibility(bool bVisible)
{
	if (bVisible)
	{
		Show();
	}
	else
	{
		Hide();
	}
}

void UMMOHUDWidget::Show()
{
	bIsVisible = true;
	SetVisibility(ESlateVisibility::Visible);
	OnShow();
}

void UMMOHUDWidget::Hide()
{
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
	OnHide();
}

void UMMOHUDWidget::FadeInFinished()
{
	bIsFading = false;
	bIsVisible = true;
}

void UMMOHUDWidget::FadeOutFinished()
{
	bIsFading = false;
	bIsVisible = false;
	SetVisibility(ESlateVisibility::Collapsed);
}

UUMGSequencePlayer* UMMOHUDWidget::PlayAnimationByName(FName AnimationName, float StartAtTime, int32 NumLoopsToPlay, EUMGSequencePlayMode::Type PlayMode, float PlaybackSpeed)
{
	UWidgetAnimation* const* FoundAnimation = nullptr;
	for (const auto& Anim : GetAllAnimations())
	{
		if (Anim && Anim->GetFName() == AnimationName)
		{
			FoundAnimation = &Anim;
			break;
		}
	}

	if (FoundAnimation && *FoundAnimation)
	{
		return PlayAnimation(*FoundAnimation, StartAtTime, NumLoopsToPlay, PlayMode, PlaybackSpeed);
	}

	return nullptr;
}

FMargin UMMOHUDWidget::GetSafeZoneInsets() const
{
	return CachedSafeZoneInsets;
}

float UMMOHUDWidget::GetResolutionScale() const
{
	if (GEngine)
	{
		const UUserInterfaceSettings* UISettings = GetDefault<UUserInterfaceSettings>();
		if (UISettings)
		{
			return UISettings->GetDPIScaleBasedOnSize(FIntPoint(GSystemResolution.ResX, GSystemResolution.ResY));
		}
	}
	return 1.0f;
}

void UMMOHUDWidget::OnResolutionChanged()
{
	UpdateSafeZone();
}

void UMMOHUDWidget::UpdateSafeZone()
{
	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);
	CachedSafeZoneInsets = DisplayMetrics.TitleSafePaddingSize;
}
