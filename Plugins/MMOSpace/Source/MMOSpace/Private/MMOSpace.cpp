#include "MMOSpace.h"

DEFINE_LOG_CATEGORY(LogMMOSpace);

IMPLEMENT_MODULE(FMMOSpaceModule, MMOSpace)

void FMMOSpaceModule::StartupModule()
{
	UE_LOG(LogMMOSpace, Log, TEXT("MMOSpace module loaded - RTS/4X Strategy Engine"));
}

void FMMOSpaceModule::ShutdownModule()
{
	UE_LOG(LogMMOSpace, Log, TEXT("MMOSpace module shutting down"));
}
