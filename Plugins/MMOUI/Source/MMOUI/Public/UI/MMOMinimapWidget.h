#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "MMOMinimapWidget.generated.h"

class UImage;
class UTextBlock;

USTRUCT(BlueprintType)
struct MMOUI_API FMMOPOIData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector WorldLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Label;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor IconColor = FLinearColor::White;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Scale = 1.0f;
};

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOMinimapWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMOMinimapWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetPlayerLocation(const FVector& WorldLocation, float Yaw);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetZoomLevel(float NewZoom);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	float GetZoomLevel() const { return ZoomLevel; }

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ZoomIn();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ZoomOut();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void AddPOI(const FMMOPOIData& POI);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void RemovePOI(const FString& POIId);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void ClearPOIs();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void AddPartyMember(const FString& MemberId, const FVector& WorldLocation, const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void UpdatePartyMember(const FString& MemberId, const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void RemovePartyMember(const FString& MemberId);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void AddQuestGiver(const FString& QuestGiverId, const FVector& WorldLocation, bool bHasAvailableQuest);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetZoneName(const FText& NewZoneName);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetCoordinates(const FVector& WorldLocation);

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void UpdateTimeDisplay();

	UFUNCTION(BlueprintCallable, Category = "Minimap")
	void SetCompassRotation(float Yaw);

	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void OnPlayerArrowUpdated(float Yaw);

	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void OnPOIAdded(const FMMOPOIData& POI);

	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void OnPOIRemoved(const FString& POIId);

	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void OnZoomChanged(float NewZoom);

	UFUNCTION(BlueprintImplementableEvent, Category = "Minimap")
	void OnZoneNameChanged(const FText& NewZoneName);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float ZoomLevel = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float MinZoom = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float MaxZoom = 3.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float ZoomStep = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float MinimapSize = 256.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Minimap")
	float WorldToMinimapScale = 0.01f;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	FVector PlayerWorldLocation;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	float PlayerYaw = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	FText ZoneName;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	FText CoordinatesText;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	FText TimeText;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TArray<FMMOPOIData> POIs;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TMap<FString, FMMOPOIData> PartyMemberMarkers;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TMap<FString, FMMOPOIData> QuestGiverMarkers;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TObjectPtr<UImage> MinimapRenderTarget;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TObjectPtr<UImage> CompassArrow;

	UPROPERTY(BlueprintReadOnly, Category = "Minimap")
	TObjectPtr<UImage> PlayerArrow;

	FVector2D WorldToMinimap(const FVector& WorldLocation) const;
};
