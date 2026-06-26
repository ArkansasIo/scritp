#include "MMOCharacter.h"

DEFINE_LOG_CATEGORY(LogMMOCharacter);

IMPLEMENT_MODULE(FMMOCharacterModule, MMOCharacter)

void FMMOCharacterModule::StartupModule()
{
	UE_LOG(LogMMOCharacter, Log, TEXT("MMOCharacter module started."));
}

void FMMOCharacterModule::ShutdownModule()
{
	UE_LOG(LogMMOCharacter, Log, TEXT("MMOCharacter module shut down."));
}
