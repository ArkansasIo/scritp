#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "GameplayTagContainer.h"
#include "Components/MMOStatComponent.h"
#include "MMOCharacterData.generated.h"

USTRUCT(BlueprintType)
struct FMMORaceBonus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FString BonusDescription;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TMap<FString, int32> StatBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TArray<FGameplayTag> RacialAbilities;
};

USTRUCT(BlueprintType)
struct FMMORaceDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FName RaceID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TObjectPtr<UTexture2D> PortraitIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TObjectPtr<USkeletalMesh> BaseMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FMMORaceBonus RaceBonuses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	TArray<FName> AllowedClasses;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FVector StartingLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race")
	FRotator StartingRotation;
};

USTRUCT(BlueprintType)
struct FMMOClassSkill
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FGameplayTag SkillTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	int32 UnlockLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	bool bAutoLearn;
};

USTRUCT(BlueprintType)
struct FMMOClassDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FName ClassID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TObjectPtr<UTexture2D> ClassIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FMMOPrimaryStats BaseStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FMMOPrimaryStats StatGainPerLevel;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TArray<FMMOClassSkill> ClassSkills;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FGameplayTagContainer ClassTags;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FString PrimaryResource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	FString SecondaryResource;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
	TSubclassOf<class AMMOCharacterBase> CharacterClass;
};

USTRUCT(BlueprintType)
struct FMMOAppearanceOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<TObjectPtr<USkeletalMesh>> HairMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<TObjectPtr<USkeletalMesh>> FaceMeshes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<FLinearColor> SkinColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<FLinearColor> HairColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<FLinearColor> EyeColors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TArray<FString> BodyTypeNames;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TMap<FString, float> HeightOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	TMap<FString, float> WeightOptions;
};

USTRUCT(BlueprintType)
struct FMMOStartingEquipment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId OffHand;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Helmet;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId ChestPiece;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Legs;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Boots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Gloves;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	FPrimaryAssetId Cape;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TArray<FPrimaryAssetId> Accessories;
};

UCLASS(BlueprintType)
class MMCHARACTER_API UMMOCharacterDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Races")
	TArray<FMMORaceDefinition> RaceDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Classes")
	TArray<FMMOClassDefinition> ClassDefinitions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
	FMMOAppearanceOptions AppearanceOptions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<FName, FMMOStartingEquipment> StartingEquipmentMap;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Equipment")
	TMap<FName, FVector> StartingLocations;

	UFUNCTION(BlueprintCallable, Category = "MMO|Data")
	FMMORaceDefinition* GetRaceDefinition(FName RaceID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Data")
	FMMOClassDefinition* GetClassDefinition(FName ClassID);

	UFUNCTION(BlueprintCallable, Category = "MMO|Data")
	FMMOStartingEquipment* GetStartingEquipment(FName Key);
};
