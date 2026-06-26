#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "MMOQuestTrackerWidget.generated.h"

UENUM(BlueprintType)
enum class EMMOQuestState : uint8
{
	Inactive,
	Active,
	Completed,
	Failed,
	TurnedIn
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOTrackedQuestData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString QuestID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText QuestName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 QuestLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMOQuestState State = EMMOQuestState::Active;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FText> Objectives;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ObjectiveProgress;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<int32> ObjectiveMax;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsComplete = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor QuestLevelColor = FLinearColor::Yellow;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMMOOnQuestClicked, const FString&, QuestID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMMOOnQuestAbandoned, const FString&, QuestID);

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOQuestTrackerWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMOQuestTrackerWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void SetTrackedQuests(const TArray<FMMOTrackedQuestData>& Quests);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void AddTrackedQuest(const FMMOTrackedQuestData& QuestData);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void RemoveTrackedQuest(const FString& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void UpdateObjectiveProgress(const FString& QuestID, int32 ObjectiveIndex, int32 Progress, int32 Max);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void SetQuestComplete(const FString& QuestID, bool bComplete);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void ClearTrackedQuests();

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void QuestClicked(const FString& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void AbandonQuest(const FString& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void ShowQuestDetails(const FString& QuestID);

	UFUNCTION(BlueprintCallable, Category = "Quest Tracker")
	void SetMaxTrackedQuests(int32 MaxQuests);

	UFUNCTION(BlueprintPure, Category = "Quest Tracker")
	int32 GetMaxTrackedQuests() const { return MaxTrackedQuests; }

	UFUNCTION(BlueprintPure, Category = "Quest Tracker")
	FLinearColor GetQuestLevelColor(int32 QuestLevel) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tracker")
	void OnQuestAdded(const FMMOTrackedQuestData& QuestData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tracker")
	void OnQuestRemoved(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tracker")
	void OnObjectiveUpdated(const FString& QuestID, int32 ObjectiveIndex, int32 Progress, int32 Max);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tracker")
	void OnQuestCompleted(const FString& QuestID);

	UFUNCTION(BlueprintImplementableEvent, Category = "Quest Tracker")
	void OnQuestDetailsOpened(const FString& QuestID);

	UPROPERTY(BlueprintAssignable, Category = "Quest Tracker")
	FMMOOnQuestClicked OnQuestClicked;

	UPROPERTY(BlueprintAssignable, Category = "Quest Tracker")
	FMMOOnQuestAbandoned OnQuestAbandoned;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Quest Tracker")
	int32 MaxTrackedQuests = 25;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Tracker")
	TArray<FMMOTrackedQuestData> TrackedQuests;

	UPROPERTY(BlueprintReadOnly, Category = "Quest Tracker")
	TObjectPtr<UPanelWidget> QuestListContainer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Quest Tracker")
	TSubclassOf<UMMOHUDWidget> QuestEntryWidgetClass;

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> QuestEntryWidgets;

	void RebuildQuestList();
	void ApplyQuestColors();
};
