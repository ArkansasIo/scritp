#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MMOCharacterMovementComponent.generated.h"

UENUM(BlueprintType)
enum class EMMOMovementMode : uint8
{
	Walking,
	Running,
	Sprinting,
	Sneaking,
	Swimming,
	Mounted,
	Flying
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMovementModeChanged, EMMOMovementMode, NewMode);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaExhausted, bool, bExhausted);

UCLASS(ClassGroup = (MMO), meta = (BlueprintSpawnableComponent))
class MMOCHARACTER_API UMMOCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	UMMOCharacterMovementComponent(const FObjectInitializer& ObjectInitializer);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float WalkSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float RunSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float SprintSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float SneakSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float SwimSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float MountedSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Speeds")
	float FlySpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|AntiCheat")
	float MaxAllowedSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|AntiCheat")
	float SpeedHackTolerance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|AntiCheat")
	int32 SpeedHackViolationLimit;

	UPROPERTY(BlueprintReadOnly, Category = "MMO|Movement|AntiCheat")
	int32 SpeedHackViolations;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Stamina")
	float SprintStaminaDrainRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Stamina")
	float MinStaminaForSprint;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MMO|Movement|Stamina")
	float StaminaRegenDelay;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MMO|Movement")
	EMMOMovementMode MMOMovementMode;

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement")
	virtual void SetMMOMovementMode(EMMOMovementMode NewMode);

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement")
	virtual EMMOMovementMode GetMMOMovementMode() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement|Stamina")
	virtual bool CanSprint() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement|Stamina")
	virtual bool IsSprinting() const;

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement")
	virtual void ApplyMovementSpeed();

	UFUNCTION(Server, Reliable, WithValidation, Category = "MMO|Movement|AntiCheat")
	virtual void ServerValidateMovement(FVector_NetQuantize10 ClientLocation, FVector_NetQuantize10 ClientVelocity, float ClientTimestamp);

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement|AntiCheat")
	virtual bool ValidateClientMovement(FVector ClientLocation, FVector ClientVelocity);

	UFUNCTION(BlueprintCallable, Category = "MMO|Movement|AntiCheat")
	virtual void ReportSpeedHackViolation();

	UFUNCTION()
	void OnRep_MMOMovementMode(EMMOMovementMode OldMode);

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnMovementModeChanged OnMovementModeChanged;

	UPROPERTY(BlueprintAssignable, Category = "MMO|Events")
	FOnStaminaExhausted OnStaminaExhausted;

protected:
	float RegenDelayTimer;
	bool bStaminaExhausted;

	virtual void HandleSprintStamina(float DeltaTime);
	virtual void UpdateMMOSpeed();
	virtual bool CheckSpeedHack(FVector ClientVelocity);
};
