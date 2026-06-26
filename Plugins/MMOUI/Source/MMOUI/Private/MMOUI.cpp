#include "MMOUI.h"

DEFINE_LOG_CATEGORY(LogMMOUI);

IMPLEMENT_MODULE(FMMOUI, MMOUI);

void FMMOUI::StartupModule()
{
	UE_LOG(LogMMOUI, Log, TEXT("MMOUI module starting up"));
}

void FMMOUI::ShutdownModule()
{
	UE_LOG(LogMMOUI, Log, TEXT("MMOUI module shutting down"));
}
