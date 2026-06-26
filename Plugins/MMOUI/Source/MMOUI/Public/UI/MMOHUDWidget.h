#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MMOHUDWidget.generated.h"

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UMMOHUDWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void PlayFadeIn(float Duration = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void PlayFadeOut(float Duration = 0.25f);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void SetWidgetVisibility(bool bVisible);

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void Show();

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void Hide();

	bool IsVisible() const { return bIsVisible; }

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void FadeInFinished();

	UFUNCTION(BlueprintCallable, Category = "MMO HUD Widget")
	void FadeOutFinished();

	UFUNCTION(BlueprintCallable, Category = "Animation")
	UUMGSequencePlayer* PlayAnimationByName(FName AnimationName, float StartAtTime = 0.0f, int32 NumLoopsToPlay = 1, EUMGSequencePlayMode::Type PlayMode = EUMGSequencePlayMode::Forward, float PlaybackSpeed = 1.0f);

	UFUNCTION(BlueprintPure, Category = "Safe Zone")
	FMargin GetSafeZoneInsets() const;

	UFUNCTION(BlueprintPure, Category = "Resolution")
	float GetResolutionScale() const;

	UFUNCTION(BlueprintCallable, Category = "Resolution")
	void OnResolutionChanged();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO HUD Widget")
	void OnShow();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO HUD Widget")
	void OnHide();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO HUD Widget")
	void OnFadeInStarted();

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO HUD Widget")
	void OnFadeOutStarted();

protected:
	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsVisible;

	UPROPERTY(BlueprintReadOnly, Category = "State")
	bool bIsFading;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UWidgetAnimation> FadeInAnimation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UWidgetAnimation> FadeOutAnimation;

	UPROPERTY(BlueprintReadOnly, Category = "Safe Zone")
	FMargin CachedSafeZoneInsets;

	virtual void UpdateSafeZone();
};
