#pragma once

#include "CoreMinimal.h"
#include "Characters/MMOCharacterBase.h"
#include "Components/SphereComponent.h"
#include "Engine/DataTable.h"
#include "MMONonPlayerCharacter.generated.h"

class UMMOInteractionComponent;
class UWidgetComponent;

UENUM(BlueprintType)
enum class EMMONPCType : uint8
{
	Vendor,
	QuestGiver,
	Trainer,
	Banker,
	Guard,
	StableMaster,
	InnKeeper,
	Craftsman,
	Generic
};

UENUM(BlueprintType)
enum class EMMOQuestIndicator : uint8
{
	None,
	Available,
	Active,
	Completed,
	Failed
};

UCLASS(Blueprintable, BlueprintType)
class MMCHARACTER_API AMMONonPlayerCharacter : public AMMOCharacterBase
{
	GENERATED_BODY()

public:
	AMMONonPlayerCharacter(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	virtual void OnInteract(AMMOCharacterBase* InstigatorCharacter) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|NPC")
	EMMONPCType NPCType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Dialogue")
	FText GreetingText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Dialogue")
	TArray<FText> GossipTextLines;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|NPC|Dialogue")
	FText CurrentGreeting;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Dialogue")
	float GreetingDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Dialogue")
	TObjectPtr<USphereComponent> DialogueTriggerSphere;

	UFUNCTION(BlueprintCallable, Category = "MMO|NPC|Dialogue")
	virtual void OnDialogueTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintCallable, Category = "MMO|NPC|Dialogue")
	virtual void OnDialogueTriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(ReplicatedUsing = OnRep_QuestIndicator, BlueprintReadOnly, Category = "MMO|NPC|Quest")
	EMMOQuestIndicator QuestIndicator;

	UFUNCTION()
	virtual void OnRep_QuestIndicator(EMMOQuestIndicator OldIndicator);

	UFUNCTION(BlueprintCallable, Category = "MMO|NPC|Quest")
	virtual void SetQuestIndicator(EMMOQuestIndicator NewIndicator);

	UFUNCTION(BlueprintImplementableEvent, Category = "MMO|NPC|Quest")
	void BP_UpdateQuestIndicator(EMMOQuestIndicator Indicator);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Quest")
	TObjectPtr<class UMMOQuestComponent> QuestComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Vendor")
	TArray<FPrimaryAssetId> VendorInventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Vendor")
	FDataTableRowHandle VendorPriceTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Vendor")
	float BuyPriceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Vendor")
	float SellPriceMultiplier;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Trainer")
	TArray<FPrimaryAssetId> TrainableSkills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "MMO|NPC|Data")
	bool bIsStaticSpawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Data")
	float SpawnRadius;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MMO|NPC|Visual")
	TObjectPtr<UWidgetComponent> NameplateWidget;

protected:
	virtual void InitializeNPCComponents();
	virtual void OnRep_Level(int32 OldLevel) override;
};
