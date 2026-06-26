#include "MMOInventory.h"

DEFINE_LOG_CATEGORY(LogMMOInventory);

IMPLEMENT_MODULE(FMMOInventoryModule, MMOInventory)

void FMMOInventoryModule::StartupModule()
{
	UE_LOG(LogMMOInventory, Log, TEXT("MMOInventory module started."));
}

void FMMOInventoryModule::ShutdownModule()
{
	UE_LOG(LogMMOInventory, Log, TEXT("MMOInventory module shut down."));
}
