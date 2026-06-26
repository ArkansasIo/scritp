#include "MMOCore.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

DEFINE_LOG_CATEGORY(LogMMOCore);
DEFINE_LOG_CATEGORY(LogMMOCoreNetwork);
DEFINE_LOG_CATEGORY(LogMMOCoreLogin);
DEFINE_LOG_CATEGORY(LogMMOCoreChat);

class FMMOCoreModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		UE_LOG(LogMMOCore, Log, TEXT("MMOCore module starting up"));

		FString BaseDir = IPluginManager::Get().FindPlugin(TEXT("MMOCore"))->GetBaseDir();
		UE_LOG(LogMMOCore, Verbose, TEXT("MMOCore plugin directory: %s"), *BaseDir);
	}

	virtual void ShutdownModule() override
	{
		UE_LOG(LogMMOCore, Log, TEXT("MMOCore module shutting down"));
	}

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}
};

IMPLEMENT_MODULE(FMMOCoreModule, MMOCore)
