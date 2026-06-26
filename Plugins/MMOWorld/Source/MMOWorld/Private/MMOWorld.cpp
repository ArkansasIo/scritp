#include "MMOWorld.h"

DEFINE_LOG_CATEGORY(LogMMOWorld);

IMPLEMENT_MODULE(FMMOWorldModule, MMOWorld);

void FMMOWorldModule::StartupModule()
{
	UE_LOG(LogMMOWorld, Log, TEXT("MMOWorld module started."));
}

void FMMOWorldModule::ShutdownModule()
{
	UE_LOG(LogMMOWorld, Log, TEXT("MMOWorld module shut down."));
}
