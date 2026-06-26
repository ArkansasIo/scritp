#pragma once

#include "CoreMinimal.h"
#include "UI/MMOHUDWidget.h"
#include "Chat/MMOChatMessage.h"
#include "MMOChatWidget.generated.h"

class UEditableTextBox;
class UScrollBox;
class UTextBlock;
class UButton;

UENUM(BlueprintType)
enum class EMMOWidgetChatChannel : uint8
{
	General,
	Combat,
	Loot,
	System,
	Trade,
	Party,
	Guild,
	Whisper,
	Emote,
	Yell,
	Say,
	Raid,
	BattleGround
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMMOOnChatMessageSent, const FString&, Message, EMMOWidgetChatChannel, Channel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMMOOnPlayerNameClicked, const FString&, PlayerID);

UCLASS(Blueprintable, BlueprintType)
class MMOUI_API UMMOChatWidget : public UMMOHUDWidget
{
	GENERATED_BODY()

public:
	UMMOChatWidget(const FObjectInitializer& ObjectInitializer);

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddChatMessage(const FMMOChatMessage& ChatMessage);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SendChatMessage(const FString& Message, EMMOWidgetChatChannel Channel);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetActiveChannel(EMMOWidgetChatChannel NewChannel);

	UFUNCTION(BlueprintPure, Category = "Chat")
	EMMOWidgetChatChannel GetActiveChannel() const { return ActiveChannel; }

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SwitchToTab(const FString& TabName);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void AddTab(const FString& TabName, const TArray<EMMOWidgetChatChannel>& Channels);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void RemoveTab(const FString& TabName);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ClearChat();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetFontSize(int32 NewFontSize);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	int32 GetFontSize() const { return FontSize; }

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void IncreaseFontSize();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void DecreaseFontSize();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ToggleTimestamp();

	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool AreTimestampsEnabled() const { return bShowTimestamps; }

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void SetChannelFilter(EMMOWidgetChatChannel Channel, bool bEnabled);

	UFUNCTION(BlueprintCallable, Category = "Chat")
	bool IsChannelEnabled(EMMOWidgetChatChannel Channel) const;

	UFUNCTION(BlueprintCallable, Category = "Chat")
	void ScrollToBottom();

	UFUNCTION(BlueprintPure, Category = "Chat")
	FLinearColor GetChannelColor(EMMOWidgetChatChannel Channel) const;

	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void OnMessageReceived(const FMMOChatMessage& ChatMessage);

	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void OnTabSwitched(const FString& TabName);

	UFUNCTION(BlueprintImplementableEvent, Category = "Chat")
	void OnFontSizeChanged(int32 NewFontSize);

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FMMOOnChatMessageSent OnChatMessageSent;

	UPROPERTY(BlueprintAssignable, Category = "Chat")
	FMMOOnPlayerNameClicked OnPlayerNameClicked;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat")
	int32 MaxMessages = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat")
	int32 FontSize = 12;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chat")
	bool bShowTimestamps = true;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	EMMOWidgetChatChannel ActiveChannel = EMMOWidgetChatChannel::General;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TArray<FMMOChatMessage> ChatHistory;

	TMap<FString, TArray<EMMOWidgetChatChannel>> ChatTabs;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	FString ActiveTabName;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TObjectPtr<UScrollBox> ChatScrollBox;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TObjectPtr<UEditableTextBox> ChatInputBox;

	UPROPERTY(BlueprintReadOnly, Category = "Chat")
	TMap<EMMOWidgetChatChannel, bool> ChannelFilters;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chat")
	TSubclassOf<UMMOHUDWidget> ChatMessageWidgetClass;

	void SetupDefaultTabs();
	void SetupDefaultChannelColors();
	bool ShouldShowMessage(const FMMOChatMessage& Message) const;
	FText FormatChatMessage(const FMMOChatMessage& Message) const;

	TMap<EMMOWidgetChatChannel, FLinearColor> ChannelColors;
};
