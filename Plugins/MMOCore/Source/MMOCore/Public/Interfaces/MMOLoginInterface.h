#pragma once

#include "MMOCore.h"
#include "UObject/Interface.h"
#include "MMOLoginInterface.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLoginComplete, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCreateAccountComplete, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnLogoutComplete, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCharacterListReceived, bool, bSuccess, const TArray<FString>&, CharacterList);
DECLARE_DYNAMIC_DELEGATE_ThreeParams(FOnCharacterCreated, bool, bSuccess, const FString&, CharacterID, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCharacterDeleted, bool, bSuccess, const FString&, ErrorMessage);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnCharacterSelected, bool, bSuccess, const FString&, ErrorMessage);

UINTERFACE(BlueprintType, meta = (CannotImplementInterfaceInBlueprint = false))
class MMCORE_API UMMOLoginInterface : public UInterface
{
	GENERATED_BODY()
};

class MMCORE_API IMMOLoginInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void Login(const FString& Username, const FString& Password, FOnLoginComplete OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void Logout(FOnLogoutComplete OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void CreateAccount(const FString& Username, const FString& Password, const FString& Email, FOnCreateAccountComplete OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void GetCharacterList(FOnCharacterListReceived OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void CreateCharacter(const FString& CharacterName, const FString& ClassName, FOnCharacterCreated OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void DeleteCharacter(const FString& CharacterID, FOnCharacterDeleted OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	void SelectCharacter(const FString& CharacterID, FOnCharacterSelected OnComplete);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	bool IsLoggedIn() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	FString GetLoggedInAccountID() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "MMO|Login")
	FString GetLoggedInUsername() const;
};
