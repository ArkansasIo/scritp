#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "Engine/DataTable.h"
#include "MMOTooltipWidget.generated.h"

UENUM(BlueprintType)
enum class EMMOTooltipType : uint8
{
	Item,
	Ability,
	Unit,
	Quest
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOItemTooltipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor RarityColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 RequiredLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText RequiredClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Stats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOAbilityTooltipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString AbilityID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText AbilityName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cost = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Range = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float CastTime = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* Icon = nullptr;
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOUnitTooltipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString UnitID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText UnitName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ClassName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText GuildName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMOHUDMode UnitAffiliation;
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOQuestTooltipData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Rewards;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuestLevel = 1;
};

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOTooltipWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMOTooltipWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowItemTooltip(const FMMOItemTooltipData& Data);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowAbilityTooltip(const FMMOAbilityTooltipData& Data);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowUnitTooltip(const FMMOUnitTooltipData& Data);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void ShowQuestTooltip(const FMMOQuestTooltipData& Data);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void SetPosition(const FVector2D& ScreenPosition);

	UFUNCTION(BlueprintCallable, Category = "Tooltip")
	void AutoPositionNearCursor(const FVector2D& CursorPosition, const FVector2D& WidgetSize);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip")
	void OnItemTooltipSet(const FMMOItemTooltipData& Data);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip")
	void OnAbilityTooltipSet(const FMMOAbilityTooltipData& Data);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip")
	void OnUnitTooltipSet(const FMMOUnitTooltipData& Data);

	UFUNCTION(BlueprintImplementableEvent, Category = "Tooltip")
	void OnQuestTooltipSet(const FMMOQuestTooltipData& Data);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Tooltip")
	EMMOTooltipType CurrentTooltipType;

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip")
	FMMOItemTooltipData CurrentItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip")
	FMMOAbilityTooltipData CurrentAbilityData;

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip")
	FMMOUnitTooltipData CurrentUnitData;

	UPROPERTY(BlueprintReadOnly, Category = "Tooltip")
	FMMOQuestTooltipData CurrentQuestData;
};
