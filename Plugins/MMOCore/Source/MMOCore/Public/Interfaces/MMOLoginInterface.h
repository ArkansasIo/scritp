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

UINTERFACE()
class MMOCORE_API UMMOLoginInterface : public UInterface
{
	GENERATED_BODY()
};

class MMOCORE_API IMMOLoginInterface
{
	GENERATED_BODY()

public:
	virtual void Login(const FString& Username, const FString& Password, FOnLoginComplete OnComplete) = 0;

	virtual void Logout(FOnLogoutComplete OnComplete) = 0;

	virtual void CreateAccount(const FString& Username, const FString& Password, const FString& Email, FOnCreateAccountComplete OnComplete) = 0;

	virtual void GetCharacterList(FOnCharacterListReceived OnComplete) = 0;

	virtual void CreateCharacter(const FString& CharacterName, const FString& ClassName, FOnCharacterCreated OnComplete) = 0;

	virtual void DeleteCharacter(const FString& CharacterID, FOnCharacterDeleted OnComplete) = 0;

	virtual void SelectCharacter(const FString& CharacterID, FOnCharacterSelected OnComplete) = 0;

	virtual bool IsLoggedIn() const = 0;

	virtual FString GetLoggedInAccountID() const = 0;

	virtual FString GetLoggedInUsername() const = 0;
};
