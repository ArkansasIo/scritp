#include "MMOQuest.h"

DEFINE_LOG_CATEGORY(LogMMOQuest);

IMPLEMENT_MODULE(FMMOQuestModule, MMOQuest)

void FMMOQuestModule::StartupModule()
{
	UE_LOG(LogMMOQuest, Log, TEXT("MMOQuest module started."));
}

void FMMOQuestModule::ShutdownModule()
{
	UE_LOG(LogMMOQuest, Log, TEXT("MMOQuest module shut down."));
}
