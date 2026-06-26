#pragma once

#include "Engine.h"
#include "Net/UnrealNetwork.h"
#include "Net/OnlineEngineInterface.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/HUD.h"
#include "GameFramework/Character.h"
#include "GameFramework/Actor.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/ActorComponent.h"
#include "Components/InputComponent.h"
#include "Engine/AssetManager.h"
#include "Engine/DataTable.h"
#include "Engine/DataAsset.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/GameInstance.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LocalPlayer.h"
#include "Engine/NetDriver.h"
#include "Engine/World.h"
#include "GameplayTagContainer.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/GameSession.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UObject/Interface.h"

#include "GenericPlatform/GenericPlatformMisc.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMMOCore, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMMOCoreNetwork, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMMOCoreLogin, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogMMOCoreChat, Log, All);

// API macro defined by UBT as MMOCORE_API
