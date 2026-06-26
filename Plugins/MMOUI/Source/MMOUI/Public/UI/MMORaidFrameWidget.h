#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "MMORaidFrameWidget.generated.h"

class UImage;
class UProgressBar;

UENUM(BlueprintType)
enum class EMMORole : uint8
{
	Tank,
	Healer,
	DPS,
	None
};

USTRUCT(BlueprintType)
struct MMOUI_API FMMOPartyMemberData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MemberID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText MemberName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HealthPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ManaPercent = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EMMORole Role = EMMORole::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GroupIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Distance = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOnline = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> BuffIDs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FString> DebuffIDs;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMMOOnPartyMemberClicked, const FString&, MemberID);

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMORaidFrameWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMORaidFrameWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void SetPartyMembers(const TArray<FMMOPartyMemberData>& Members);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void UpdateMember(int32 Index, const FMMOPartyMemberData& MemberData);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void RemoveMember(int32 Index);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void ClearMembers();

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void SetRaidMode(bool bInRaidMode);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void SetGroupNumbering(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void SetRangeFadeEnabled(bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Raid Frames")
	void SetRangeFadeDistance(float Distance);

	UFUNCTION(BlueprintImplementableEvent, Category = "Raid Frames")
	void OnMemberAdded(const FMMOPartyMemberData& MemberData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Raid Frames")
	void OnMemberRemoved(int32 Index);

	UFUNCTION(BlueprintImplementableEvent, Category = "Raid Frames")
	void OnMemberUpdated(int32 Index, const FMMOPartyMemberData& MemberData);

	UFUNCTION(BlueprintImplementableEvent, Category = "Raid Frames")
	void OnRaidModeChanged(bool bIsRaid);

	UPROPERTY(BlueprintAssignable, Category = "Raid Frames")
	FMMOOnPartyMemberClicked OnMemberClicked;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid Frames")
	int32 MaxRaidSize = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Raid Frames")
	int32 MembersPerGroup = 5;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	TArray<FMMOPartyMemberData> PartyMembers;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	bool bIsRaidMode = false;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	bool bShowGroupNumbers = true;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	bool bRangeFadeEnabled = true;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	float RangeFadeDistance = 40.0f;

	UPROPERTY()
	TArray<TObjectPtr<UWidget>> MemberWidgets;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Raid Frames")
	TSubclassOf<UMMOHUDWidget> MemberFrameClass;

	UPROPERTY(BlueprintReadOnly, Category = "Raid Frames")
	TObjectPtr<UPanelWidget> MemberContainer;

	void RebuildMemberWidgets();
	void UpdateRangeFading();
};
