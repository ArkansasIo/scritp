using UnrealBuildTool;

public class MMOParty : ModuleRules
{
	public MMOParty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"MMOCore",
			"MMOChat"
		});
	}
}
