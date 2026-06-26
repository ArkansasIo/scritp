#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SphereComponent.h"
#include "MMOInteractionComponent.generated.h"

class AMMOCharacterBase;
class UWidgetComponent;

USTRUCT(BlueprintType)
struct FInteractionPromptData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText InteractionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FText ActionText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	float InteractionDuration;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bRequiresConfirmation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	bool bShowProgressBar;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UTexture2D* InteractionIcon;

	FInteractionPromptData()
		: InteractionDuration(0.0f)
		, bRequiresConfirmation(false)
		, bShowProgressBar(false)
		, InteractionIcon(nullptr)
	{}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, AMMOCharacterBase*, FocusedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, AMMOCharacterBase*, UnfocusedCharacter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, AMMOCharacterBase*, InteractingCharacter, AMMOCharacterBase*, TargetCharacter);

UCLASS(ClassGroup = (MMO), meta = (BlueprintSpawnableComponent), Blueprintable, BlueprintType)
class MMOCHARACTER_API UMMOInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMMOInteractionComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Interaction")
	TObjectPtr<USphereComponent> InteractionRangeSphere;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Interaction")
	float InteractionRange;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Interaction")
	float InteractionAngle;

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void GetInteractablesInRange(TArray<AActor*>& OutActors) const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual AMMOCharacterBase* GetClosestInteractable() const;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Interaction")
	TArray<AActor*> InteractablesInRange;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Interaction")
	TObjectPtr<AMMOCharacterBase> CurrentFocusTarget;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Interaction")
	TObjectPtr<AMMOCharacterBase> CurrentInteractionTarget;

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void Interact();

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void InteractWithTarget(AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void ServerInteract(AMMOCharacterBase* Target);

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerInteractWith(AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void FocusTarget(AMMOCharacterBase* Target);

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void UnfocusTarget();

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual void UpdateInteractionPrompt();

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Interaction")
	FInteractionPromptData CurrentPromptData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|Interaction")
	FInteractionPromptData DefaultInteractionPrompt;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnBeginFocus OnBeginFocus;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnEndFocus OnEndFocus;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnInteract OnInteract;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Interaction")
	bool bCanInteract;

	UFUNCTION(BlueprintCallable, Category = "MMO|Interaction")
	virtual bool TraceForInteraction(AMMOCharacterBase*& OutHitCharacter) const;

protected:
	virtual void UpdateInteractablesInRange();
	virtual void UpdateFocus();


};
